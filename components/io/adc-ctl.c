#include <lexp.h>
#include <io.h>
#include <string.h>
#include <module/error/mk_error.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

char* unbox_atom(lexp atom);
lexp atom(const char *s);
lexp car(lexp p);
lexp cdr(lexp p);
extern lexp nil;

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

#define debug(fmt, ...) \
    do { \
        if (DEBUG_LEVEL >= 1) { \
            printf(fmt, ##__VA_ARGS__); \
            fflush(stdout); \
        } \
    } while (0)

/*---------------------------------------------------------------
  ADC Calibration
  ---------------------------------------------------------------*/
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        debug("calibration scheme version is %s\n", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        debug("calibration scheme version is %s\n", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        debug("Calibration Success\n");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        debug("eFuse not burnt, skip software calibration\n");
    } else {
        debug("Invalid arg or no memory\n");
    }

    return calibrated;
}

static void adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    debug("deregister %s calibration scheme\n", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    debug("deregister %s calibration scheme\n", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}

#define ADC_ATTEN           ADC_ATTEN_DB_12

static adc_cali_handle_t adc_cali_chan_handle = NULL;
static adc_oneshot_unit_handle_t adc1_handle;

lexp assoc_pair(lexp v, lexp e);

static lexp adc_ctl_write(struct io_typ *port, lexp exp) {
    int ch = -1;
    lexp result;
    lexp ch_pair = assoc_pair(atom("ch"), exp);
    if (typof(ch_pair) != NIL) {
        ch = car(cdr(ch_pair));
    }
    if (ch == -1)
        return mk_error("Missing channel number in:", exp);

    ch = ADC_CHANNEL_0 + ch;
    //-------------ADC1 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    esp_err_t rc;
    rc = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (rc != ESP_OK) {
        return mk_error("Couldn't allocate ADC unit for channel", exp);
    }

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN,
    };
    rc = adc_oneshot_config_channel(adc1_handle, ch, &config);
    if (rc != ESP_OK) {
        result = mk_error("Couldn't config the channel:", exp);
        goto err_free_unit;
    }
    //-------------ADC1 Calibration Init---------------//
    bool calibrated = adc_calibration_init(ADC_UNIT_1, ch, ADC_ATTEN, &adc_cali_chan_handle);
    if (!calibrated){
        result = mk_error("Couldn't calibrate the channel:", exp);
        goto err_free_unit;
    }

    int adc_raw, voltage;
    rc = adc_oneshot_read(adc1_handle, ch, &adc_raw);
    if (rc != ESP_OK) {
        result = mk_error("Couldn't read the channel:", exp);
        goto err_free_calib;
    }
    debug("ADC%d Channel[%d] Raw Data: %d\n", ADC_UNIT_1 + 1, ch, adc_raw);
    rc = adc_cali_raw_to_voltage(adc_cali_chan_handle, adc_raw, &voltage);
    if (rc != ESP_OK) {
        result = mk_error("Couldn't convert to voltage:", exp);
        goto err_free_calib;
    }
    debug("ADC%d Channel[%d] Cali Voltage: %d mV\n", ADC_UNIT_1 + 1, ch, voltage);
    result = voltage;

err_free_calib:
    adc_calibration_deinit(adc_cali_chan_handle);
err_free_unit:
    rc = adc_oneshot_del_unit(adc1_handle);
    if (rc != ESP_OK) {
        return mk_error("Couldn't release the ADC unit:", exp);
    }
    return result;
}




PORTS_SECTION struct io_typ adc_ctl_port = {
    .private = NULL,
    .read = NULL,
    .write = adc_ctl_write,
    .proto = "adc-ctl",
};


