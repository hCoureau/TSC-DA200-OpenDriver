#ifndef TSC_DA200_TSPL_H
#define TSC_DA200_TSPL_H

#include <stddef.h>
#include <stdio.h>

typedef enum { TSPL_MEDIA_GAP, TSPL_MEDIA_BLACK_MARK,
               TSPL_MEDIA_CONTINUOUS } tspl_media_t;
typedef enum { TSPL_POST_NONE, TSPL_POST_TEAR, TSPL_POST_PEEL,
               TSPL_POST_CUT, TSPL_POST_PARTIAL_CUT } tspl_post_action_t;
typedef enum { TSPL_OCCURRENCE_EVERY, TSPL_OCCURRENCE_COPIES,
               TSPL_OCCURRENCE_JOB, TSPL_OCCURRENCE_INTERVAL } tspl_occurrence_t;
typedef enum { TSPL_DITHER_THRESHOLD, TSPL_DITHER_ORDERED } tspl_dither_t;

typedef struct {
  tspl_media_t media;
  double gap_mm;
  double gap_offset_mm;
  int speed;
  int density;
  int direction;
  int mirror;
  int negative;
  int collate;
  double reference_x_mm;
  double reference_y_mm;
  double shift_x_mm;
  double shift_y_mm;
  double feed_offset_mm;
  tspl_post_action_t post_action;
  tspl_occurrence_t occurrence;
  int interval;
  tspl_dither_t dither;
} tspl_options_t;

void tspl_options_defaults(tspl_options_t *options);
void tspl_options_parse(tspl_options_t *options, const char *cups_options);
int tspl_write_page_begin(FILE *output, const tspl_options_t *options,
                          unsigned width_px, unsigned height_px,
                          unsigned dpi_x, unsigned dpi_y,
                          unsigned bytes_per_line, unsigned copies);
int tspl_write_page_end(FILE *output, unsigned copies);
void tspl_invert_row(unsigned char *row, size_t length);
void tspl_mask_row_padding(unsigned char *row, unsigned width_px);
void tspl_mirror_row(unsigned char *row, unsigned width_px);
void tspl_prepare_bitmap_row(unsigned char *row, unsigned width_px, int mirror);
void tspl_pack_gray_row(unsigned char *output, const unsigned char *input,
                        unsigned width_px, unsigned row_number,
                        tspl_dither_t dither, int luminance_samples,
                        int negative);

#endif
