/******************************************************************************
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "logging.h"

//------------------------------------------------------------------------------

#define DEFAULT_WIFI_STATION_SSID       "SherlockForest"
#define DEFAULT_WIFI_STATION_PASSWORD   "tranyarita"

#define RED_LED_GPIO                    GPIO_NUM_3
#define GREEN_LED_GPIO                  GPIO_NUM_4
#define BLUE_LED_GPIO                   GPIO_NUM_5
#define COOL_WHITE_LED_GPIO             GPIO_NUM_18
#define WARM_WHITE_LED_GPIO             GPIO_NUM_19

#define RGB_LEDC_RESOLUTION_BITS        8
#define RGB_LEDC_RESOLUTION_COUNTS      (1 << RGB_LEDC_RESOLUTION_BITS)
#define RGB_LEDC_FREQUENCY              10000

#define RGB_LEDC_TIMER                  LEDC_TIMER_0
#define RGB_LEDC_MODE                   LEDC_LOW_SPEED_MODE     // High speed mode not available on C3
#define RED_LEDC_CHANNEL                LEDC_CHANNEL_0
#define GREEN_LEDC_CHANNEL              LEDC_CHANNEL_1
#define BLUE_LEDC_CHANNEL               LEDC_CHANNEL_2

// Handles (instances) for esp_event API
// Configured up in wifi_station_init()

static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;
static esp_event_handler_instance_t instance_lost_ip;

// ESP-NETIF handle
// Needed if network needs to be de-initialized via esp_netif_destroy() or
// esp_netif_destroy_default_wifi()

static esp_netif_t *wifi_netif_handle;

// WiFi (station or AP) configuration - SSID and PW

wifi_config_t wifi_station_config;

/******************************************************************************
 *
 ******************************************************************************/

void wifi_event_handler(void* arg, esp_event_base_t event_base,
                        int32_t event_id, void* event_data)
{

}

/******************************************************************************
 *
 ******************************************************************************/

void error_check(esp_err_t err)
{
    ESP_ERROR_CHECK(err);
}

/******************************************************************************
 *
 ******************************************************************************/

void wifi_init(void)
{
    esp_err_t ret;
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_station_config =
    {
        .sta =
        {
            .ssid = DEFAULT_WIFI_STATION_SSID,
            .password = DEFAULT_WIFI_STATION_PASSWORD,
            .bssid_set = false
        }
    };

    ret = esp_netif_init();
    error_check(ret);
    wifi_netif_handle = esp_netif_create_default_wifi_sta();

    ret = esp_wifi_init(&wifi_config);
    error_check(ret);

    //----------------------------------------
    //  Register wifi events of interest
    //----------------------------------------

    //  All WiFi-related events
    ret = esp_event_handler_instance_register(
          WIFI_EVENT, ESP_EVENT_ANY_ID,
          &wifi_event_handler, NULL, &instance_any_id);

    // - Getting an IP
    ret = esp_event_handler_instance_register(
          IP_EVENT, IP_EVENT_STA_GOT_IP,
          &wifi_event_handler, NULL, &instance_got_ip);

    // - Loosing an IP
    ret = esp_event_handler_instance_register(
          IP_EVENT, IP_EVENT_STA_LOST_IP,
          &wifi_event_handler, NULL, &instance_lost_ip);

    ret = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    error_check(ret);
    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    error_check(ret);
    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_station_config);
    error_check(ret);
    ret = esp_wifi_start();
    error_check(ret);
    ret = esp_wifi_connect();
    error_check(ret);
}

/******************************************************************************
 *
 ******************************************************************************/

void gpio_init(void)
{
    gpio_reset_pin(RED_LED_GPIO);
    gpio_set_direction(RED_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(GREEN_LED_GPIO);
    gpio_set_direction(GREEN_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(BLUE_LED_GPIO);
    gpio_set_direction(BLUE_LED_GPIO, GPIO_MODE_OUTPUT);
}

/******************************************************************************
 *
 ******************************************************************************/

const ledc_timer_config_t ledc_timer =
{
    .speed_mode       = RGB_LEDC_MODE,
    .timer_num        = RGB_LEDC_TIMER,
    .duty_resolution  = RGB_LEDC_RESOLUTION_BITS,
    .freq_hz          = RGB_LEDC_FREQUENCY,
    .clk_cfg          = LEDC_AUTO_CLK
};

const ledc_channel_config_t ledc_channel[] =
{
    {
        .speed_mode     = RGB_LEDC_MODE,
        .channel        = RED_LEDC_CHANNEL,
        .timer_sel      = RGB_LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = RED_LED_GPIO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    },
    {
        .speed_mode     = RGB_LEDC_MODE,
        .channel        = GREEN_LEDC_CHANNEL,
        .timer_sel      = RGB_LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GREEN_LED_GPIO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    },
    {
        .speed_mode     = RGB_LEDC_MODE,
        .channel        = BLUE_LEDC_CHANNEL,
        .timer_sel      = RGB_LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BLUE_LED_GPIO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    }
};

void led_controller_init(void)
{
    esp_err_t ret;
    uint8_t index;

    ret = ledc_timer_config(&ledc_timer);
    ESP_ERROR_CHECK(ret);
    for (index = 0; index < 3; index++)
    {
        ret = ledc_channel_config(&ledc_channel[index]);
        ESP_ERROR_CHECK(ret);
    }
}

/******************************************************************************
 *
 ******************************************************************************/

void app_main(void)
{
    esp_err_t ret;
    uint16_t red_level, green_level, blue_level;
    uint16_t increment = RGB_LEDC_RESOLUTION_COUNTS / 16;

    DPRINTF_C(LOGC_HIGHLIGHT, "--- LED Floodlight Start ---\n");

    gpio_init();
    nvs_flash_init();

    ret = esp_event_loop_create_default();
    error_check(ret);

    led_controller_init();
    wifi_init();

    while (true)
    {
        for (blue_level = 0; blue_level <= RGB_LEDC_RESOLUTION_COUNTS; blue_level += increment)
        {
            ledc_set_duty(RGB_LEDC_MODE, BLUE_LEDC_CHANNEL, blue_level);
            ledc_update_duty(RGB_LEDC_MODE, BLUE_LEDC_CHANNEL);

            for (green_level = 0; green_level <= RGB_LEDC_RESOLUTION_COUNTS; green_level += increment)
            {
                ledc_set_duty(RGB_LEDC_MODE, GREEN_LEDC_CHANNEL, green_level);
                ledc_update_duty(RGB_LEDC_MODE, GREEN_LEDC_CHANNEL);

                for (red_level = 0; red_level <= RGB_LEDC_RESOLUTION_COUNTS; red_level += increment)
                {
//                    ledc_set_duty_and_update(RGB_LEDC_MODE, RED_LEDC_CHANNEL, red_level, 0);
                    ledc_set_duty(RGB_LEDC_MODE, RED_LEDC_CHANNEL, red_level);
                    ledc_update_duty(RGB_LEDC_MODE, RED_LEDC_CHANNEL);

                    vTaskDelay(3);  // 30 mS
                }
                ledc_set_duty(RGB_LEDC_MODE, RED_LEDC_CHANNEL, 0);
                ledc_update_duty(RGB_LEDC_MODE, RED_LEDC_CHANNEL);
            }
            ledc_set_duty(RGB_LEDC_MODE, GREEN_LEDC_CHANNEL, 0);
            ledc_update_duty(RGB_LEDC_MODE, GREEN_LEDC_CHANNEL);
        }
        ledc_set_duty(RGB_LEDC_MODE, BLUE_LEDC_CHANNEL, 0);
        ledc_update_duty(RGB_LEDC_MODE, BLUE_LEDC_CHANNEL);
    }
}
