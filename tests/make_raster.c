#include <cups/raster.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  cups_page_header2_t header;
  cups_raster_t *raster;
  unsigned char rows[8][2] = {
      {0xff, 0xff}, {0x80, 0x01}, {0xaa, 0x55}, {0x80, 0x01},
      {0x80, 0x01}, {0x55, 0xaa}, {0x80, 0x01}, {0xff, 0xff}};
  unsigned y;
  int gray = argc == 2 && !strcmp(argv[1], "--gray");
  int two_pages = argc == 2 && !strcmp(argv[1], "--two-pages");
  int bad_dpi = argc == 2 && !strcmp(argv[1], "--bad-dpi");
  int bad_width = argc == 2 && !strcmp(argv[1], "--bad-width");
  int short_row = argc == 2 && !strcmp(argv[1], "--short-row");
  int truncated = argc == 2 && !strcmp(argv[1], "--truncated");
  FILE *temporary = NULL;
  int raster_fd = STDOUT_FILENO;
  unsigned page;

  if (argc > 2 || (argc == 2 && !gray && !two_pages && !bad_dpi &&
                   !bad_width && !short_row && !truncated)) {
    fprintf(stderr, "Unknown test raster option\n");
    return 2;
  }

  memset(&header, 0, sizeof(header));
  header.HWResolution[0] = 203;
  header.HWResolution[1] = 203;
  header.PageSize[0] = 6;
  header.PageSize[1] = 3;
  header.cupsWidth = 16;
  header.cupsHeight = 8;
  header.cupsBitsPerColor = gray ? 8 : 1;
  header.cupsBitsPerPixel = gray ? 8 : 1;
  header.cupsBytesPerLine = gray ? 16 : 2;
  header.cupsColorOrder = CUPS_ORDER_CHUNKED;
  header.cupsColorSpace = CUPS_CSPACE_K;
  header.cupsNumColors = 1;
  if (bad_dpi) header.HWResolution[0] = 300;
  if (bad_width) {
    header.cupsWidth = 865;
    header.cupsBytesPerLine = 109;
  }
  /*
   * Newer CUPS writers correctly refuse an internally inconsistent header.
   * For the reader-hardening test, write a valid native raster first and then
   * mutate only cupsBytesPerLine in a seekable temporary file. CUPS native
   * raster fields use host byte order, so the uint32_t write works on either
   * endian architecture.
   */
  if (short_row) {
    temporary = tmpfile();
    if (!temporary) return 1;
    raster_fd = dup(fileno(temporary));
    if (raster_fd < 0) return 1;
  }

  raster = cupsRasterOpen(raster_fd, CUPS_RASTER_WRITE);
  if (!raster) return 1;
  for (page = 0; page < (two_pages ? 2U : 1U); ++page) {
    if (!cupsRasterWriteHeader2(raster, &header)) return 1;
    if (truncated) break;
    for (y = 0; y < 8; ++y) {
      if (gray) {
        unsigned char gradient[16];
        unsigned x;
        for (x = 0; x < 16; ++x) gradient[x] = (unsigned char)(x * 17U);
        if (cupsRasterWritePixels(raster, gradient, 16) != 16) return 1;
      } else {
        unsigned char row[2] = {rows[y][0], rows[y][1]};
        if (page) { row[0] ^= 0xff; row[1] ^= 0xff; }
        unsigned bytes = header.cupsBytesPerLine;
        unsigned char wide_row[109] = {0};
        if (bytes == 2) memcpy(wide_row, row, 2);
        if (cupsRasterWritePixels(raster, wide_row, bytes) != bytes) return 1;
      }
    }
  }
  cupsRasterClose(raster);
  if (short_row) {
    uint32_t one = 1;
    unsigned char buffer[4096];
    size_t count;
    long field_offset = 4L + (long)offsetof(cups_page_header2_t,
                                            cupsBytesPerLine);

    close(raster_fd);
    if (fseek(temporary, field_offset, SEEK_SET) != 0 ||
        fwrite(&one, sizeof(one), 1, temporary) != 1 ||
        fflush(temporary) != 0 || fseek(temporary, 0, SEEK_SET) != 0) {
      return 1;
    }
    while ((count = fread(buffer, 1, sizeof(buffer), temporary)) > 0) {
      if (fwrite(buffer, 1, count, stdout) != count) return 1;
    }
    if (ferror(temporary) || fclose(temporary) != 0) return 1;
  }
  return 0;
}
