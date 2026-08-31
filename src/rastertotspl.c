#include "tspl.h"

#include <cups/raster.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_JOB_COPIES 9999U
#define MAX_RASTER_ROW_BYTES 4096U

static void fail(const char *message) { fprintf(stderr, "ERROR: %s\n", message); }

static int parse_copies(const char *text, unsigned *copies) {
  char *end = NULL;
  unsigned long value;
  errno = 0;
  value = strtoul(text, &end, 10);
  if (errno || end == text || *end != '\0' || value < 1 ||
      value > MAX_JOB_COPIES) {
    return -1;
  }
  *copies = (unsigned)value;
  return 0;
}

static int replay_stream(FILE *input, FILE *output) {
  unsigned char buffer[65536];
  size_t count;
  if (fflush(input) == EOF || fseek(input, 0, SEEK_SET) != 0) return -1;
  while ((count = fread(buffer, 1, sizeof(buffer), input)) > 0) {
    if (fwrite(buffer, 1, count, output) != count) return -1;
  }
  return ferror(input) ? -1 : 0;
}

int main(int argc, char **argv) {
  int input_fd = STDIN_FILENO;
  cups_raster_t *raster;
  cups_page_header2_t header;
  tspl_options_t options;
  unsigned char *input_row = NULL;
  unsigned char *output_row = NULL;
  unsigned page = 0;
  unsigned requested_copies = 1;
  unsigned job_copies;
  FILE *render_output = stdout;
  FILE *collation_buffer = NULL;
  int status = EXIT_FAILURE;
  if (argc != 6 && argc != 7) {
    fail("Usage: rastertotspl job-id user title copies options [file]");
    return EXIT_FAILURE;
  }
  if (argc == 7) {
    input_fd = open(argv[6], O_RDONLY);
    if (input_fd < 0) {
      fprintf(stderr, "ERROR: Unable to open input: %s\n", strerror(errno));
      return EXIT_FAILURE;
    }
  }
  if (parse_copies(argv[4], &requested_copies) < 0) {
    fail("Copy count must be an integer from 1 through 9999");
    goto cleanup_fd;
  }
  tspl_options_defaults(&options);
  tspl_options_parse(&options, argv[5]);
  fprintf(stderr, "INFO: Effects mirror=%d negative=%d collate=%d\n",
          options.mirror, options.negative, options.collate);
  job_copies = requested_copies;
  if (options.collate) {
    collation_buffer = tmpfile();
    if (!collation_buffer) {
      fprintf(stderr, "ERROR: Unable to create secure collation buffer: %s\n",
              strerror(errno));
      goto cleanup_fd;
    }
    render_output = collation_buffer;
  }
  raster = cupsRasterOpen(input_fd, CUPS_RASTER_READ);
  if (!raster) { fail("Unable to open CUPS raster stream"); goto cleanup_fd; }
  while (cupsRasterReadHeader2(raster, &header)) {
    unsigned y;
    unsigned copies = requested_copies;
    unsigned page_copies;
    unsigned output_bytes;
    int luminance_samples;
    ++page;
    fprintf(stderr, "PAGE: %u %u\n", page, copies);
    fprintf(stderr, "INFO: Rendering %ux%u at %ux%u dpi\n", header.cupsWidth,
            header.cupsHeight, header.HWResolution[0], header.HWResolution[1]);
    if (!((header.cupsBitsPerPixel == 1 && header.cupsBitsPerColor == 1) ||
          (header.cupsBitsPerPixel == 8 && header.cupsBitsPerColor == 8))) {
      fail("PPD pipeline did not produce a supported 1-bit or 8-bit raster");
      goto cleanup_raster;
    }
    if (!header.cupsWidth || header.cupsWidth > 864 || !header.cupsHeight ||
        header.cupsHeight > 18270 || !header.cupsBytesPerLine ||
        header.cupsBytesPerLine > MAX_RASTER_ROW_BYTES) {
      fail("Raster dimensions are invalid or wider than the DA200 printhead");
      goto cleanup_raster;
    }
    if (header.HWResolution[0] != 203 || header.HWResolution[1] != 203) {
      fail("Raster resolution is not the DA200's supported 203 dpi");
      goto cleanup_raster;
    }
    if (header.cupsBitsPerPixel == 8 && header.cupsBytesPerLine < header.cupsWidth) {
      fail("Eight-bit raster row is shorter than its pixel width");
      goto cleanup_raster;
    }
    if (header.cupsBitsPerPixel == 1 &&
        header.cupsBytesPerLine < (header.cupsWidth + 7U) / 8U) {
      fail("One-bit raster row is shorter than its pixel width");
      goto cleanup_raster;
    }
    if (header.NumCopies > MAX_JOB_COPIES) {
      fail("Raster copy count exceeds the supported maximum of 9999");
      goto cleanup_raster;
    }
    if (header.NumCopies > 1) copies = header.NumCopies;
    if (copies > job_copies) job_copies = copies;
    page_copies = options.collate ? 1U : copies;
    luminance_samples = header.cupsColorSpace == CUPS_CSPACE_W || header.cupsColorSpace == CUPS_CSPACE_SW;
    output_bytes = (header.cupsWidth + 7U) / 8U;
    input_row = malloc(header.cupsBytesPerLine);
    output_row = malloc(output_bytes);
    if (!input_row || !output_row) { fail("Out of memory allocating raster row"); goto cleanup_raster; }
    if (tspl_write_page_begin(render_output, &options, header.cupsWidth, header.cupsHeight,
                              header.HWResolution[0], header.HWResolution[1],
                              output_bytes, page_copies) < 0) {
      fail("Unable to write TSPL header"); goto cleanup_raster;
    }
    for (y = 0; y < header.cupsHeight; ++y) {
      if (cupsRasterReadPixels(raster, input_row, header.cupsBytesPerLine) != header.cupsBytesPerLine) {
        fail("Unexpected end of raster page"); goto cleanup_raster;
      }
      if (header.cupsBitsPerPixel == 8) {
        tspl_pack_gray_row(output_row, input_row, header.cupsWidth, y,
                           options.dither, luminance_samples, options.negative);
      } else {
        memcpy(output_row, input_row, output_bytes);
        if (luminance_samples != options.negative) tspl_invert_row(output_row, output_bytes);
      }
      /* CUPS rows use 1 bits for black coverage internally; this firmware's
         TSPL BITMAP command prints zero bits. Convert only at the transport
         boundary so grayscale, negative, and mirror logic share one model. */
      tspl_prepare_bitmap_row(output_row, header.cupsWidth, options.mirror);
      if (fwrite(output_row, 1, output_bytes, render_output) != output_bytes) {
        fail("Unable to write TSPL bitmap"); goto cleanup_raster;
      }
    }
    free(input_row); input_row = NULL;
    free(output_row); output_row = NULL;
    if (tspl_write_page_end(render_output, page_copies) < 0 || fflush(render_output) == EOF) {
      fail("Unable to finish TSPL page"); goto cleanup_raster;
    }
  }
  if (!page) {
    fail("Input contained no raster pages");
  } else if (collation_buffer) {
    unsigned copy;
    for (copy = 0; copy < job_copies; ++copy) {
      if (replay_stream(collation_buffer, stdout) < 0) {
        fail("Unable to replay collated job");
        goto cleanup_raster;
      }
    }
    if (fflush(stdout) == EOF) fail("Unable to finish collated job");
    else status = EXIT_SUCCESS;
  } else {
    status = EXIT_SUCCESS;
  }
cleanup_raster:
  free(input_row);
  free(output_row);
  cupsRasterClose(raster);
cleanup_fd:
  if (collation_buffer) fclose(collation_buffer);
  if (argc == 7 && input_fd >= 0) close(input_fd);
  return status;
}
