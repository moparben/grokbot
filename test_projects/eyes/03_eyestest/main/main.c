// eyestest: bring up BOTH displays (dual-eye stability test).

// If your backlight turns ON when BL is driven LOW, set this to 1.
#define EYE_BACKLIGHT_ACTIVE_LOW 0

// Optional: PWM brightness using LEDC (0/1)
#define EYE_BACKLIGHT_USE_LEDC 0

#include "display_eyes.h"

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "eyestest";

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);

    // Wiring defaults (edit to match your hardware)
    display_eyes_config_t cfg = {
        .gpio_mosi = 4,
        .gpio_sclk = 5,
        .gpio_dc = 6,
        .gpio_rst = 7,
        .gpio_bl = 15,
        .gpio_cs_left = 16,
        .gpio_cs_right = 16,  // Same as left for single display testing
        .spi_freq_hz = (10 * 1000 * 1000),  // Lower SPI speed to eliminate black lines
        .backlight_percent = 80,
        .autoblink_enabled = true,
        .tracking_enabled = false,  // Disable eyelid tracking - was covering display
    };

    ESP_LOGI(TAG, "Init both eyes...");
    esp_err_t err = display_eyes_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "display_eyes_init failed: %s", esp_err_to_name(err));
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Backlight sanity blink
    display_eyes_set_brightness(0);
    vTaskDelay(pdMS_TO_TICKS(200));
    display_eyes_set_brightness(100);
    vTaskDelay(pdMS_TO_TICKS(200));
    display_eyes_set_brightness(cfg.backlight_percent);

    display_eyes_set_state(EYE_STATE_IDLE);

    while (1) {
        ESP_LOGI(TAG, "FPS: %.1f", display_eyes_get_fps());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
