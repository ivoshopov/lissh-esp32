#include <lexp.h>
#include <io.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"


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
        printf("calibration scheme version is %s\n", "Curve Fitting");
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
        printf("calibration scheme version is %s\n", "Line Fitting");
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
        printf("Calibration Success\n");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        printf("eFuse not burnt, skip software calibration\n");
    } else {
        printf("Invalid arg or no memory\n");
    }

    return calibrated;
}

static void example_adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    printf("deregister %s calibration scheme\n", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    printf("deregister %s calibration scheme\n", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}

#define ADC_ATTEN           ADC_ATTEN_DB_12

adc_cali_handle_t adc_cali_chan_handle = NULL;
adc_oneshot_unit_handle_t adc1_handle;
static int esp_adc_init(int channel)
{
    if (channel <= 10) {
        int ch = ADC_CHANNEL_0 + channel;
        //-------------ADC1 Init---------------//
        adc_oneshot_unit_init_cfg_t init_config1 = {
            .unit_id = ADC_UNIT_1,
        };
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

        //-------------ADC1 Config---------------//
        adc_oneshot_chan_cfg_t config = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = ADC_ATTEN,
        };
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ch, &config));
        //-------------ADC1 Calibration Init---------------//
        bool calibrated = adc_calibration_init(ADC_UNIT_1, ch, ADC_ATTEN, &adc_cali_chan_handle);
        if (calibrated)
            return 0;
        return -1;
    } else {
        return -1;
    }
}

static int esp_adc_read(int channel) {
    if (channel <= 10) {
        int ch = ADC_CHANNEL_0 + channel;
        int adc_raw, voltage;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ch, &adc_raw));
        printf("ADC%d Channel[%d] Raw Data: %d\n", ADC_UNIT_1 + 1, ch, adc_raw);
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_chan_handle, adc_raw, &voltage));
        printf("ADC%d Channel[%d] Cali Voltage: %d mV\n", ADC_UNIT_1 + 1, ch, voltage);
        return voltage;
    } else {
        return -1;
    }
}

static lexp adc_read(struct io_typ *port) {
    int *ch = port->private;
    esp_adc_init(*ch);
    lexp result = esp_adc_read(*ch);
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
    example_adc_calibration_deinit(adc_cali_chan_handle);
    return result;
}



static int adc0 = 0;

PORTS_SECTION struct io_typ adc_port0 = {
    .private = &adc0,
    .read = adc_read,
    .write = NULL,
    .proto = "adc0",
};


static int adc1 = 1;

PORTS_SECTION struct io_typ adc_port1 = {
    .private = &adc1,
    .read = adc_read,
    .write = NULL,
    .proto = "adc1",
};
