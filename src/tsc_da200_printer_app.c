/*
 * TSC DA200 Printer Application
 *
 * A PAPPL-backed IPP Everywhere endpoint for the DA200.  PAPPL owns the
 * IPP, DNS-SD, and raster pipeline; this driver owns the final TSPL transport.
 */

#include <pappl/pappl.h>

#include "tspl.h"

#include <stdlib.h>
#include <string.h>

#define DA200_DPI 203U
#define DA200_4X6_WIDTH_DOTS 812U
#define DA200_4X6_LENGTH_DOTS 1218U

typedef struct da200_job_s {
  unsigned width;
  unsigned output_bytes;
  unsigned copies;
  unsigned page_count;
  unsigned char *row;
} da200_job_t;

static bool da200_driver(pappl_system_t *system, const char *driver_name,
                         const char *device_uri, const char *device_id,
                         pappl_pr_driver_data_t *data, ipp_t **attrs,
                         void *cbdata);
static bool da200_end_job(pappl_job_t *job, pappl_pr_options_t *options,
                          pappl_device_t *device);
static bool da200_end_page(pappl_job_t *job, pappl_pr_options_t *options,
                           pappl_device_t *device, unsigned page);
static bool da200_start_job(pappl_job_t *job, pappl_pr_options_t *options,
                            pappl_device_t *device);
static bool da200_start_page(pappl_job_t *job, pappl_pr_options_t *options,
                             pappl_device_t *device, unsigned page);
static bool da200_write_line(pappl_job_t *job, pappl_pr_options_t *options,
                             pappl_device_t *device, unsigned y,
                             const unsigned char *line);
static bool da200_status(pappl_printer_t *printer);
static void da200_identify(pappl_printer_t *printer,
                           pappl_identify_actions_t actions,
                           const char *message);
static pappl_system_t *da200_system(int num_options, cups_option_t *options,
                                    void *cbdata);

static pappl_pr_driver_t da200_drivers[] = {
  { "tsc_da200_4x6", "TSC DA200 Open Driver (AirPrint, fixed 4x6)", NULL, NULL }
};

/*
 * Use a single, explicitly configured device queue.  In PAPPL 1.3 the remote
 * Add-Printer path can retain zero-valued dynamic defaults, which produces an
 * invalid IPP response that iOS quite correctly rejects.  Creating the queue
 * after registering its driver keeps the advertised immutable 4x6 defaults
 * in the printer object from the outset.
 */
static pappl_system_t *da200_system(int num_options, cups_option_t *options,
                                    void *cbdata) {
  const char *device_uri = cupsGetOption("device-uri", num_options, options);
  const char *printer_name = cupsGetOption("printer-name", num_options, options);
  const char *spool_directory = cupsGetOption("spool-directory", num_options, options);
  const char *port_string = cupsGetOption("server-port", num_options, options);
  pappl_system_t *system;
  pappl_printer_t *printer;
  pappl_pr_driver_data_t configured;
  ipp_t *attrs = NULL;
  int port = port_string ? atoi(port_string) : 8000;
  (void)cbdata;

  if (!device_uri || !*device_uri || port < 1 || port > 65535) return NULL;
  if (!printer_name || !*printer_name) printer_name = "TSC-DA200";
  if (!spool_directory || !*spool_directory) spool_directory = "/var/spool/tsc-da200-printer-app";

  system = papplSystemCreate(PAPPL_SOPTIONS_NONE, "TSC-DA200-OpenDriver", port,
                             "_print,_universal", spool_directory, NULL,
                             PAPPL_LOGLEVEL_WARN, NULL, false);
  if (!system) return NULL;
  papplSystemSetPrinterDrivers(system,
                               sizeof(da200_drivers) / sizeof(da200_drivers[0]),
                               da200_drivers, NULL, NULL, da200_driver, NULL);
  printer = papplPrinterCreate(system, 0, printer_name, "tsc_da200_4x6",
                               "MFG:TSC;MDL:DA200;CMD:TSPL;", device_uri);
  if (!printer) {
    papplSystemDelete(system);
    return NULL;
  }
  /* PAPPL 1.3 can initialise a newly created system queue with an empty
   * driver-data copy.  Install a fresh capability record once the printer
   * object exists so Get-Printer-Attributes remains standards-compliant. */
  memset(&configured, 0, sizeof(configured));
  if (!da200_driver(system, "tsc_da200_4x6", device_uri,
                    "MFG:TSC;MDL:DA200;CMD:TSPL;", &configured, &attrs, NULL) ||
      !papplPrinterSetDriverData(printer, &configured, attrs)) {
    ippDelete(attrs);
    papplSystemDelete(system);
    return NULL;
  }
  ippDelete(attrs);
  papplSystemAddListeners(system, NULL);
  return system;
}

static bool da200_write(pappl_device_t *device, const void *buffer,
                        size_t length) {
  return papplDeviceWrite(device, buffer, length) == (ssize_t)length;
}

static bool da200_driver(pappl_system_t *system, const char *driver_name,
                         const char *device_uri, const char *device_id,
                         pappl_pr_driver_data_t *data, ipp_t **attrs,
                         void *cbdata) {
  (void)system;
  (void)device_uri;
  (void)device_id;
  (void)attrs;
  (void)cbdata;
  if (strcmp(driver_name, "tsc_da200_4x6")) return false;

  data->identify_cb = da200_identify;
  data->rendjob_cb = da200_end_job;
  data->rendpage_cb = da200_end_page;
  data->rstartjob_cb = da200_start_job;
  data->rstartpage_cb = da200_start_page;
  data->rwriteline_cb = da200_write_line;
  data->status_cb = da200_status;
  /*
   * PAPPL accepts a PWG raster stream and invokes the raster callbacks below;
   * TSPL is the device output language, not the format advertised to IPP
   * clients.  Advertising TSPL here makes the queue non-driverless.
   */
  data->format = "image/pwg-raster";
  papplCopyString(data->make_and_model, "TSC DA200 Open Driver", sizeof(data->make_and_model));
  data->kind = PAPPL_KIND_LABEL | PAPPL_KIND_ROLL;
  data->ppm = 20;
  data->color_supported = PAPPL_COLOR_MODE_BI_LEVEL | PAPPL_COLOR_MODE_MONOCHROME;
  data->color_default = PAPPL_COLOR_MODE_MONOCHROME;
  data->content_default = PAPPL_CONTENT_AUTO;
  data->orient_default = IPP_ORIENT_PORTRAIT;
  data->quality_default = IPP_QUALITY_NORMAL;
  data->scaling_default = PAPPL_SCALING_FIT;
  data->sides_supported = PAPPL_SIDES_ONE_SIDED;
  data->sides_default = PAPPL_SIDES_ONE_SIDED;
  data->num_resolution = 1;
  data->x_resolution[0] = DA200_DPI;
  data->y_resolution[0] = DA200_DPI;
  data->x_default = DA200_DPI;
  data->y_default = DA200_DPI;
  data->raster_types = PAPPL_PWG_RASTER_TYPE_SGRAY_8;
  data->force_raster_type = PAPPL_PWG_RASTER_TYPE_SGRAY_8;
  data->borderless = true;
  /* PAPPL requires non-zero imageable-area margins.  One PWG unit is
   * effectively borderless while retaining a standards-compliant media DB. */
  data->left_right = 1;
  data->bottom_top = 1;
  data->num_media = 1;
  data->media[0] = "na_index-4x6_4x6in";
  data->num_source = 1;
  data->source[0] = "main-roll";
  data->num_type = 1;
  data->type[0] = "labels";
  papplCopyString(data->media_default.size_name, "na_index-4x6_4x6in", sizeof(data->media_default.size_name));
  papplCopyString(data->media_default.source, "main-roll", sizeof(data->media_default.source));
  papplCopyString(data->media_default.type, "labels", sizeof(data->media_default.type));
  data->media_default.size_width = 10160;
  data->media_default.size_length = 15240;
  data->media_default.tracking = PAPPL_MEDIA_TRACKING_GAP;
  data->media_ready[0] = data->media_default;
  data->tracking_supported = PAPPL_MEDIA_TRACKING_GAP;
  data->mode_supported = PAPPL_LABEL_MODE_TEAR_OFF;
  data->mode_configured = PAPPL_LABEL_MODE_TEAR_OFF;
  data->darkness_supported = 16;
  data->darkness_configured = 53;
  data->identify_supported = PAPPL_IDENTIFY_ACTIONS_SOUND;
  data->identify_default = PAPPL_IDENTIFY_ACTIONS_SOUND;
  return true;
}

static bool da200_start_job(pappl_job_t *job, pappl_pr_options_t *options,
                            pappl_device_t *device) {
  da200_job_t *state = calloc(1, sizeof(*state));
  (void)options;
  (void)device;
  if (!state) {
    papplJobSetMessage(job, "Unable to allocate rendering state.");
    return false;
  }
  papplJobSetData(job, state);
  return true;
}

static bool da200_start_page(pappl_job_t *job, pappl_pr_options_t *options,
                             pappl_device_t *device, unsigned page) {
  da200_job_t *state = papplJobGetData(job);
  int density;
  (void)page;
  if (!state || options->header.HWResolution[0] != DA200_DPI ||
      options->header.HWResolution[1] != DA200_DPI ||
      options->header.cupsWidth != DA200_4X6_WIDTH_DOTS ||
      options->header.cupsHeight != DA200_4X6_LENGTH_DOTS) {
    papplJobSetMessage(job, "Only 4x6 inch labels at 203 dpi are supported.");
    return false;
  }
  state->width = options->header.cupsWidth;
  state->output_bytes = (state->width + 7U) / 8U;
  state->copies = options->copies > 0 ? (unsigned)options->copies : 1U;
  state->row = malloc(state->output_bytes);
  if (!state->row) {
    papplJobSetMessage(job, "Unable to allocate a raster row.");
    return false;
  }
  density = (options->darkness_configured + options->print_darkness) * 15 / 100;
  if (density < 0) density = 0;
  if (density > 15) density = 15;
  /* Direction 1 makes the logical top of a portrait AirPrint document exit
   * the DA200 first.  This is the device's normal physical feed direction. */
  if (papplDevicePrintf(device, "SIZE 101.6 mm,152.4 mm\nGAP 3 mm,0 mm\nDIRECTION 1,0\nDENSITY %d\nCLS\nBITMAP 0,0,%u,%u,1,",
                        density, state->output_bytes, options->header.cupsHeight) < 0) {
    papplJobSetMessage(job, "Unable to initialize the DA200 print job.");
    return false;
  }
  return true;
}

static bool da200_write_line(pappl_job_t *job, pappl_pr_options_t *options,
                             pappl_device_t *device, unsigned y,
                             const unsigned char *line) {
  da200_job_t *state = papplJobGetData(job);
  (void)options;
  if (!state || !state->row || !line) return false;
  /* IPP clients commonly supply anti-aliased grayscale pixels even for
   * barcodes.  A fixed threshold preserves their rectangular 1-bit geometry;
   * ordered dithering turns those edge pixels into a visible dot pattern. */
  tspl_pack_gray_row(state->row, line, state->width, y, TSPL_DITHER_THRESHOLD, 1, 0);
  tspl_prepare_bitmap_row(state->row, state->width, 0);
  if (!da200_write(device, state->row, state->output_bytes)) {
    papplJobSetMessage(job, "Unable to send raster data to the DA200.");
    return false;
  }
  return true;
}

static bool da200_end_page(pappl_job_t *job, pappl_pr_options_t *options,
                           pappl_device_t *device, unsigned page) {
  da200_job_t *state = papplJobGetData(job);
  (void)options;
  (void)page;
  if (!state || papplDevicePrintf(device, "\nPRINT %u,1\n", state->copies) < 0) return false;
  papplDeviceFlush(device);
  free(state->row);
  state->row = NULL;
  state->page_count++;
  return true;
}

static bool da200_end_job(pappl_job_t *job, pappl_pr_options_t *options,
                          pappl_device_t *device) {
  da200_job_t *state = papplJobGetData(job);
  (void)options;
  (void)device;
  if (!state) return false;
  papplJobSetImpressions(job, (int)state->page_count);
  papplJobSetImpressionsCompleted(job, (int)state->page_count);
  free(state->row);
  free(state);
  papplJobSetData(job, NULL);
  return true;
}

static bool da200_status(pappl_printer_t *printer) {
  pappl_device_t *device = papplPrinterOpenDevice(printer);
  pappl_preason_t reasons;
  if (!device) {
    papplPrinterSetReasons(printer, PAPPL_PREASON_OFFLINE, PAPPL_PREASON_DEVICE_STATUS);
    return true;
  }
  reasons = papplDeviceGetStatus(device) & PAPPL_PREASON_DEVICE_STATUS;
  papplPrinterSetReasons(printer, reasons, PAPPL_PREASON_DEVICE_STATUS & ~reasons);
  papplPrinterCloseDevice(printer);
  return true;
}

static void da200_identify(pappl_printer_t *printer,
                           pappl_identify_actions_t actions,
                           const char *message) {
  pappl_device_t *device;
  (void)message;
  if (!(actions & PAPPL_IDENTIFY_ACTIONS_SOUND) || !(device = papplPrinterOpenDevice(printer))) return;
  papplDevicePuts(device, "SOUND 1,100\n");
  papplDeviceFlush(device);
  papplPrinterCloseDevice(printer);
}

int main(int argc, char *argv[]) {
  return papplMainloop(argc, argv, "1.1.1", NULL,
                        (int)(sizeof(da200_drivers) / sizeof(da200_drivers[0])),
                        da200_drivers, NULL, da200_driver,
                        NULL, NULL, da200_system, NULL, NULL);
}
