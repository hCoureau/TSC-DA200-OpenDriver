#include "tspl.h"

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static int clamp_long(long value, int minimum, int maximum) {
  if (value < (long)minimum) return minimum;
  if (value > (long)maximum) return maximum;
  return (int)value;
}

static double clamp_double(double value, double minimum, double maximum) {
  if (value < minimum) return minimum;
  if (value > maximum) return maximum;
  return value;
}

void tspl_options_defaults(tspl_options_t *options) {
  options->media = TSPL_MEDIA_GAP;
  options->gap_mm = 3.0;
  options->gap_offset_mm = 0.0;
  options->speed = 4;
  options->density = 7;
  options->direction = 1;
  options->mirror = 0;
  options->negative = 0;
  options->collate = 1;
  options->reference_x_mm = 0.0;
  options->reference_y_mm = 0.0;
  options->shift_x_mm = 0.0;
  options->shift_y_mm = 0.0;
  options->feed_offset_mm = 0.0;
  options->post_action = TSPL_POST_TEAR;
  options->occurrence = TSPL_OCCURRENCE_EVERY;
  options->interval = 1;
  options->dither = TSPL_DITHER_ORDERED;
}

static void apply_text_option(tspl_options_t *options, const char *key,
                              const char *value) {
  if (!strcasecmp(key, "TscMediaType")) {
    if (!strcasecmp(value, "Gap")) options->media = TSPL_MEDIA_GAP;
    else if (!strcasecmp(value, "BlackMark")) options->media = TSPL_MEDIA_BLACK_MARK;
    else if (!strcasecmp(value, "Continuous")) options->media = TSPL_MEDIA_CONTINUOUS;
  } else if (!strcasecmp(key, "TscPostAction") || !strcasecmp(key, "PostAction")) {
    if (!strcasecmp(value, "None")) options->post_action = TSPL_POST_NONE;
    else if (!strcasecmp(value, "Tear") || !strcasecmp(value, "TearOff")) options->post_action = TSPL_POST_TEAR;
    else if (!strcasecmp(value, "Peel") || !strcasecmp(value, "PeelOff")) options->post_action = TSPL_POST_PEEL;
    else if (!strcasecmp(value, "Cut")) options->post_action = TSPL_POST_CUT;
    else if (!strcasecmp(value, "PartialCut")) options->post_action = TSPL_POST_PARTIAL_CUT;
  } else if (!strcasecmp(key, "TscOccurrence") || !strcasecmp(key, "Occurrence")) {
    if (!strcasecmp(value, "Every")) options->occurrence = TSPL_OCCURRENCE_EVERY;
    else if (!strcasecmp(value, "Copies")) options->occurrence = TSPL_OCCURRENCE_COPIES;
    else if (!strcasecmp(value, "Job")) options->occurrence = TSPL_OCCURRENCE_JOB;
    else if (!strcasecmp(value, "Interval") || !strcasecmp(value, "Specified")) options->occurrence = TSPL_OCCURRENCE_INTERVAL;
  } else if (!strcasecmp(key, "ColorOption") || !strcasecmp(key, "TscHalftone")) {
    if (!strcasecmp(value, "Threshold") || !strcasecmp(value, "GrayScale")) options->dither = TSPL_DITHER_THRESHOLD;
    else options->dither = TSPL_DITHER_ORDERED;
  }
}

static int parse_double_option(tspl_options_t *options, const char *key,
                               const char *value) {
  char *end = NULL;
  double number;
  if (strcasecmp(key, "TscGap") && strcasecmp(key, "TscOffset") &&
      strcasecmp(key, "TscGapOffset") && strcasecmp(key, "TscReferenceX") &&
      strcasecmp(key, "TscReferenceY") && strcasecmp(key, "TscShiftX") &&
      strcasecmp(key, "TscShiftY") && strcasecmp(key, "TscFeedOffset") &&
      strcasecmp(key, "FeedOffset")) return 0;
  errno = 0;
  number = strtod(value, &end);
  if (errno || end == value || *end || !isfinite(number)) return 1;
  if (!strcasecmp(key, "TscGap")) options->gap_mm = clamp_double(number, 0.0, 25.4);
  else if (!strcasecmp(key, "TscOffset") || !strcasecmp(key, "TscGapOffset")) options->gap_offset_mm = clamp_double(number, 0.0, 25.4);
  else if (!strcasecmp(key, "TscReferenceX")) options->reference_x_mm = clamp_double(number, 0.0, 25.4);
  else if (!strcasecmp(key, "TscReferenceY")) options->reference_y_mm = clamp_double(number, 0.0, 25.4);
  else if (!strcasecmp(key, "TscShiftX")) options->shift_x_mm = clamp_double(number, -25.4, 25.4);
  else if (!strcasecmp(key, "TscShiftY")) options->shift_y_mm = clamp_double(number, -25.4, 25.4);
  else options->feed_offset_mm = clamp_double(number, -25.4, 25.4);
  return 1;
}

static void apply_option(tspl_options_t *options, const char *key,
                         const char *value) {
  char *end = NULL;
  long integer;
  apply_text_option(options, key, value);
  if (parse_double_option(options, key, value)) return;
  errno = 0;
  integer = strtol(value, &end, 10);
  if (errno || end == value || *end) {
    if (!strcasecmp(value, "True")) integer = 1;
    else if (!strcasecmp(value, "False")) integer = 0;
    else return;
  }
  if (!strcasecmp(key, "TscSpeed")) options->speed = clamp_long(integer, 2, 6);
  else if (!strcasecmp(key, "TscDensity")) options->density = clamp_long(integer, 0, 15);
  else if (!strcasecmp(key, "TscDirection")) options->direction = integer ? 1 : 0;
  else if (!strcasecmp(key, "TscMirror") || !strcasecmp(key, "MirrorImage")) options->mirror = integer ? 1 : 0;
  else if (!strcasecmp(key, "TscNegative") || !strcasecmp(key, "NegativeImage")) options->negative = integer ? 1 : 0;
  else if (!strcasecmp(key, "Collate")) options->collate = integer ? 1 : 0;
  else if (!strcasecmp(key, "TscInterval") || !strcasecmp(key, "Interval")) options->interval = clamp_long(integer, 1, 65535);
}

void tspl_options_parse(tspl_options_t *options, const char *cups_options) {
  char *copy;
  char *cursor;
  char *token;
  if (!cups_options || !*cups_options) return;
  copy = strdup(cups_options);
  if (!copy) return;
  cursor = copy;
  while ((token = strsep(&cursor, " \t\r\n")) != NULL) {
    char *equals;
    if (!*token) continue;
    equals = strchr(token, '=');
    if (!equals) {
      if (!strncasecmp(token, "no", 2) && token[2])
        apply_option(options, token + 2, "False");
      else
        apply_option(options, token, "True");
      continue;
    }
    *equals = '\0';
    apply_option(options, token, equals + 1);
  }
  free(copy);
}

static int dots_from_mm(double millimetres, unsigned dpi) {
  double dots = millimetres * (double)dpi / 25.4;
  return (int)(dots + (dots < 0.0 ? -0.5 : 0.5));
}

static int write_post_action(FILE *output, const tspl_options_t *options,
                             unsigned copies) {
  const char *cut_value = "1";
  char cut_number[32];
  if (fputs("SET CUTTER OFF\r\nSET PARTIAL_CUTTER OFF\r\nSET PEEL OFF\r\nSET TEAR OFF\r\n", output) == EOF) return -1;
  if (!copies) copies = 1;
  if (options->occurrence == TSPL_OCCURRENCE_COPIES) {
    snprintf(cut_number, sizeof(cut_number), "%u", copies);
    cut_value = cut_number;
  } else if (options->occurrence == TSPL_OCCURRENCE_JOB) {
    cut_value = "BATCH";
  } else if (options->occurrence == TSPL_OCCURRENCE_INTERVAL) {
    snprintf(cut_number, sizeof(cut_number), "%d", options->interval);
    cut_value = cut_number;
  }
  if (options->post_action == TSPL_POST_TEAR) return fputs("SET TEAR ON\r\n", output) == EOF ? -1 : 0;
  if (options->post_action == TSPL_POST_PEEL) return fputs("SET PEEL ON\r\n", output) == EOF ? -1 : 0;
  if (options->post_action == TSPL_POST_CUT) return fprintf(output, "SET CUTTER %s\r\n", cut_value) < 0 ? -1 : 0;
  if (options->post_action == TSPL_POST_PARTIAL_CUT) return fprintf(output, "SET PARTIAL_CUTTER %s\r\n", cut_value) < 0 ? -1 : 0;
  return 0;
}

int tspl_write_page_begin(FILE *output, const tspl_options_t *options,
                          unsigned width_px, unsigned height_px,
                          unsigned dpi_x, unsigned dpi_y,
                          unsigned bytes_per_line, unsigned copies) {
  double width_mm;
  double height_mm;
  int speed;
  if (!output || !options || !width_px || !height_px || !dpi_x || !dpi_y || !bytes_per_line) return -1;
  width_mm = (double)width_px * 25.4 / (double)dpi_x;
  height_mm = (double)height_px * 25.4 / (double)dpi_y;
  speed = options->speed;
  if (options->post_action == TSPL_POST_PEEL && speed > 3) speed = 3;
  if (fprintf(output, "SIZE %.2f mm,%.2f mm\r\n", width_mm, height_mm) < 0) return -1;
  if (options->media == TSPL_MEDIA_GAP) {
    if (fprintf(output, "GAP %.2f mm,%.2f mm\r\n", options->gap_mm, options->gap_offset_mm) < 0) return -1;
  } else if (options->media == TSPL_MEDIA_BLACK_MARK) {
    if (fprintf(output, "BLINE %.2f mm,%.2f mm\r\n", options->gap_mm, options->gap_offset_mm) < 0) return -1;
  } else if (fputs("GAP 0 mm,0 mm\r\n", output) == EOF) return -1;
  if (fprintf(output,
              "SPEED %d\r\nDENSITY %d\r\nSET RIBBON OFF\r\nDIRECTION %d,%d\r\n"
              "REFERENCE %d,%d\r\nSHIFT %d,%d\r\nOFFSET %.2f mm\r\n",
              speed, options->density, options->direction, 0,
              dots_from_mm(options->reference_x_mm, dpi_x), dots_from_mm(options->reference_y_mm, dpi_y),
              dots_from_mm(options->shift_x_mm, dpi_x), dots_from_mm(options->shift_y_mm, dpi_y),
              options->feed_offset_mm) < 0) return -1;
  if (write_post_action(output, options, copies) < 0) return -1;
  return fprintf(output, "CLS\r\nBITMAP 0,0,%u,%u,1,", bytes_per_line, height_px) < 0 ? -1 : 0;
}

int tspl_write_page_end(FILE *output, unsigned copies) {
  if (!copies) copies = 1;
  return fprintf(output, "\r\nPRINT 1,%u\r\n", copies) < 0 ? -1 : 0;
}

void tspl_invert_row(unsigned char *row, size_t length) {
  size_t index;
  for (index = 0; index < length; ++index) row[index] = (unsigned char)~row[index];
}

void tspl_mask_row_padding(unsigned char *row, unsigned width_px) {
  unsigned remainder = width_px & 7U;
  if (remainder) row[width_px / 8U] &= (unsigned char)(0xffU << (8U - remainder));
}

void tspl_mirror_row(unsigned char *row, unsigned width_px) {
  unsigned left;
  if (!row || width_px < 2) return;
  for (left = 0; left < width_px / 2U; ++left) {
    unsigned right = width_px - 1U - left;
    unsigned char left_mask = (unsigned char)(0x80U >> (left & 7U));
    unsigned char right_mask = (unsigned char)(0x80U >> (right & 7U));
    int left_set = (row[left / 8U] & left_mask) != 0;
    int right_set = (row[right / 8U] & right_mask) != 0;
    if (left_set != right_set) {
      row[left / 8U] ^= left_mask;
      row[right / 8U] ^= right_mask;
    }
  }
}

void tspl_prepare_bitmap_row(unsigned char *row, unsigned width_px, int mirror) {
  size_t length;
  if (!row || !width_px) return;
  length = (width_px + 7U) / 8U;
  tspl_mask_row_padding(row, width_px);
  if (mirror) tspl_mirror_row(row, width_px);
  /* TSPL BITMAP uses zero bits for printed dots on the tested DA200 firmware. */
  tspl_invert_row(row, length);
}

/* Return the rank of a pixel in a mathematically generated 8x8 Bayer screen.
   The construction starts with the canonical 2x2 order and recursively
   expands it three times. Keeping the construction in code makes the screen
   independently reproducible without carrying a vendor-derived lookup table. */
static unsigned ordered_rank_8x8(unsigned x, unsigned y) {
  static const unsigned quadrant[2][2] = {{0U, 2U}, {3U, 1U}};
  unsigned rank = 0;
  int bit;
  for (bit = 2; bit >= 0; --bit) {
    unsigned column = (x >> (unsigned)bit) & 1U;
    unsigned row = (y >> (unsigned)bit) & 1U;
    rank = rank * 4U + quadrant[row][column];
  }
  return rank;
}

void tspl_pack_gray_row(unsigned char *output, const unsigned char *input,
                        unsigned width_px, unsigned row_number,
                        tspl_dither_t dither, int luminance_samples,
                        int negative) {
  unsigned x;
  memset(output, 0, (width_px + 7U) / 8U);
  for (x = 0; x < width_px; ++x) {
    unsigned blackness = luminance_samples ? 255U - input[x] : input[x];
    unsigned threshold = dither == TSPL_DITHER_ORDERED ?
        ordered_rank_8x8(x & 7U, row_number & 7U) * 4U + 2U : 128U;
    if (negative) blackness = 255U - blackness;
    /* The ordered thresholds are centered within their four-level buckets,
       giving exact white no dots and exact black complete coverage. */
    if ((dither == TSPL_DITHER_ORDERED && blackness > threshold) ||
        (dither == TSPL_DITHER_THRESHOLD && blackness >= threshold))
      output[x / 8U] |= (unsigned char)(0x80U >> (x & 7U));
  }
}
