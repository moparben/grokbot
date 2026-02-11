#include "display_eyes.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef EYE_PIN_MOSI
#define EYE_PIN_MOSI 22
#endif
#ifndef EYE_PIN_SCLK
#define EYE_PIN_SCLK 23
#endif
#ifndef EYE_PIN_DC
#define EYE_PIN_DC 4
#endif
#ifndef EYE_PIN_RST
#define EYE_PIN_RST 5
#endif
#ifndef EYE_PIN_BL
#define EYE_PIN_BL 6
#endif
#ifndef EYE_PIN_CS_LEFT
#define EYE_PIN_CS_LEFT 20
#endif
#ifndef EYE_PIN_CS_RIGHT
#define EYE_PIN_CS_RIGHT 21
#endif

static const char *TAG = "eyes_animation";

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "Starting eyes_animation — both eyes with procedural animation");

    display_eyes_config_t cfg = {
        .gpio_mosi = EYE_PIN_MOSI,
        .gpio_sclk = EYE_PIN_SCLK,
        .gpio_dc = EYE_PIN_DC,
        .gpio_rst = EYE_PIN_RST,
        .gpio_bl = EYE_PIN_BL,
        .gpio_cs_left = EYE_PIN_CS_LEFT,
        .gpio_cs_right = EYE_PIN_CS_RIGHT,
        .spi_freq_hz = (10 * 1000 * 1000),
        .backlight_percent = 100,
        .autoblink_enabled = true,
        .tracking_enabled = true,
    };

    esp_err_t err = display_eyes_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "display_eyes_init failed: %s", esp_err_to_name(err));
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Let the component run its internal animation task and report FPS periodically
    while (1) {
        float fps = display_eyes_get_fps();
        ESP_LOGI(TAG, "Eye animation FPS: %.1f", fps);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
