#include "tspl.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned count_bits(unsigned char value) {
  unsigned count = 0;
  while (value) {
    count += value & 1U;
    value >>= 1U;
  }
  return count;
}

int main(void) {
  tspl_options_t options;
  unsigned char row[] = {0x00, 0x55, 0xaa, 0xff};
  unsigned char padded[] = {0xff, 0xff};
  unsigned char gray_k[] = {0, 127, 128, 255};
  unsigned char gray_w[] = {255, 128, 127, 0};
  unsigned char packed[1];
  unsigned char halftone_white[] = {0, 0, 0, 0};
  unsigned char barcode_edge[] = {0, 0, 127, 128, 255, 255, 0, 0};
  unsigned char ordered_input[8];
  unsigned char ordered_output[1];
  unsigned previous_dots = 0;
  unsigned char mirrored[] = {0x90, 0x40};
  unsigned char bitmap_black[] = {0xff, 0xff};
  unsigned char bitmap_white[] = {0x00, 0x00};
  unsigned char bitmap_padded[] = {0xff, 0xff};
  char *buffer = NULL;
  size_t length = 0;
  FILE *stream;

  tspl_options_defaults(&options);
  assert(options.media == TSPL_MEDIA_GAP);
  assert(options.gap_mm == 3.0);
  assert(options.speed == 4);
  tspl_options_parse(&options,
                     "TscMediaType=BlackMark TscGap=4 TscGapOffset=2 "
                     "TscSpeed=99 TscDensity=12 TscDirection=0 "
                     "TscReferenceX=1 TscReferenceY=2 TscShiftX=-2 "
                     "TscShiftY=3 TscFeedOffset=-1 TscMirror=True "
                     "TscNegative=True TscPostAction=Cut "
                     "TscOccurrence=Interval TscInterval=3");
  assert(options.media == TSPL_MEDIA_BLACK_MARK);
  assert(options.gap_mm == 4.0);
  assert(options.gap_offset_mm == 2.0);
  assert(options.speed == 6);
  assert(options.density == 12);
  assert(options.direction == 0);
  assert(options.reference_x_mm == 1.0 && options.reference_y_mm == 2.0);
  assert(options.shift_x_mm == -2.0 && options.shift_y_mm == 3.0);
  assert(options.feed_offset_mm == -1.0);
  assert(options.mirror == 1 && options.negative == 1);
  assert(options.collate == 1);
  assert(options.post_action == TSPL_POST_CUT);
  assert(options.occurrence == TSPL_OCCURRENCE_INTERVAL);
  assert(options.interval == 3);
  options.mirror = 0;
  options.negative = 0;
  options.collate = 0;
  tspl_options_parse(&options, "TscMirror TscNegative Collate");
  assert(options.mirror == 1 && options.negative == 1 && options.collate == 1);
  tspl_options_parse(&options, "noTscMirror noTscNegative noCollate");
  assert(options.mirror == 0 && options.negative == 0 && options.collate == 0);
  tspl_options_defaults(&options);
  tspl_options_parse(&options,
                     "TscGap=nan TscOffset=inf TscReferenceX=-inf "
                     "TscShiftX=NaN TscFeedOffset=Infinity");
  assert(options.gap_mm == 3.0 && options.gap_offset_mm == 0.0);
  assert(options.reference_x_mm == 0.0 && options.shift_x_mm == 0.0);
  assert(options.feed_offset_mm == 0.0);
  tspl_options_parse(&options,
                     "TscSpeed=2147483647 TscDensity=-2147483647 "
                     "TscInterval=2147483647");
  assert(options.speed == 6 && options.density == 0 && options.interval == 65535);

  /* Restore the option set used by the command-generation assertions. */
  tspl_options_parse(&options,
                     "TscMediaType=BlackMark TscGap=4 TscGapOffset=2 "
                     "TscSpeed=6 TscDensity=12 TscDirection=0 "
                     "TscReferenceX=1 TscReferenceY=2 TscShiftX=-2 "
                     "TscShiftY=3 TscFeedOffset=-1 TscPostAction=Cut "
                     "TscOccurrence=Interval TscInterval=3 noCollate");

  tspl_invert_row(row, sizeof(row));
  assert(row[0] == 0xff && row[1] == 0xaa && row[2] == 0x55 && row[3] == 0x00);
  tspl_mask_row_padding(padded, 10);
  assert(padded[0] == 0xff && padded[1] == 0xc0);
  tspl_mirror_row(mirrored, 10);
  assert(mirrored[0] == 0x82 && mirrored[1] == 0x40);
  tspl_prepare_bitmap_row(bitmap_black, 16, 0);
  assert(bitmap_black[0] == 0x00 && bitmap_black[1] == 0x00);
  tspl_prepare_bitmap_row(bitmap_white, 16, 0);
  assert(bitmap_white[0] == 0xff && bitmap_white[1] == 0xff);
  tspl_prepare_bitmap_row(bitmap_padded, 10, 0);
  assert(bitmap_padded[0] == 0x00 && bitmap_padded[1] == 0x3f);
  tspl_pack_gray_row(packed, gray_k, 4, 0, TSPL_DITHER_THRESHOLD, 0, 0);
  assert(packed[0] == 0x30);
  tspl_pack_gray_row(packed, gray_w, 4, 0, TSPL_DITHER_THRESHOLD, 1, 0);
  assert(packed[0] == 0x30);
  /* Barcode-safe thresholding must make the same binary choice on every
     row; ordered dithering would instead vary these anti-aliased edges. */
  tspl_pack_gray_row(packed, barcode_edge, 8, 0, TSPL_DITHER_THRESHOLD, 1, 0);
  assert(packed[0] == 0xe3);
  tspl_pack_gray_row(ordered_output, barcode_edge, 8, 7, TSPL_DITHER_THRESHOLD, 1, 0);
  assert(ordered_output[0] == packed[0]);
  tspl_pack_gray_row(packed, halftone_white, 4, 0, TSPL_DITHER_ORDERED, 0, 0);
  assert(packed[0] == 0x00);
  {
    unsigned level;
    for (level = 0; level <= 255; ++level) {
      unsigned y;
      unsigned dots = 0;
      memset(ordered_input, (int)level, sizeof(ordered_input));
      for (y = 0; y < 8; ++y) {
        tspl_pack_gray_row(ordered_output, ordered_input, 8, y,
                           TSPL_DITHER_ORDERED, 0, 0);
        dots += count_bits(ordered_output[0]);
      }
      assert(dots >= previous_dots);
      if (level == 0) assert(dots == 0);
      if (level == 128) assert(dots == 32);
      if (level == 255) assert(dots == 64);
      previous_dots = dots;
    }
  }

  stream = open_memstream(&buffer, &length);
  assert(stream != NULL);
  assert(tspl_write_page_begin(stream, &options, 812, 1218, 203, 203, 102, 2) == 0);
  assert(tspl_write_page_end(stream, 2) == 0);
  assert(fclose(stream) == 0);
  assert(strstr(buffer, "SIZE 101.60 mm,152.40 mm\r\n") != NULL);
  assert(strstr(buffer, "BLINE 4.00 mm,2.00 mm\r\n") != NULL);
  assert(strstr(buffer, "DIRECTION 0,0\r\n") != NULL);
  assert(strstr(buffer, "REFERENCE 8,16\r\n") != NULL);
  assert(strstr(buffer, "SHIFT -16,24\r\n") != NULL);
  assert(strstr(buffer, "OFFSET -1.00 mm\r\n") != NULL);
  assert(strstr(buffer, "SET CUTTER 3\r\n") != NULL);
  assert(strstr(buffer, "BITMAP 0,0,102,1218,1,") != NULL);
  assert(strstr(buffer, "PRINT 1,2\r\n") != NULL);
  free(buffer);
  puts("TSPL encoder tests passed");
  return 0;
}
