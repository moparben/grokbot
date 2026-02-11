// Focused eye animation bring-up.
// This is the ESP-IDF equivalent of the Arduino eye demo behavior (blink + look directions).

// If your backlight turns ON when BL is driven LOW, set this to 1.
#define EYE_BACKLIGHT_ACTIVE_LOW 0

// Optional: PWM brightness using LEDC (0/1)
#define EYE_BACKLIGHT_USE_LEDC 0

#include "display_eyes.h"

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char *TAG = "eyes_anim";

// Console task state (needs to be global to avoid nested function issues)
static bool bl_on = true;

void console_task(void *arg) {
    display_eyes_config_t *cfg = (display_eyes_config_t *)arg;
    if (!cfg) {
        ESP_LOGE(TAG, "Console task: NULL config");
        vTaskDelete(NULL);
        return;
    }
    for (;;) {
        int c = getchar();
        if (c == EOF) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        if (c == 's') {
            static bool q = EYE_HIGH_QUALITY;
            q = !q;
            display_eyes_set_high_quality(q);
            printf("Smoothing: %s\n", q ? "ON" : "OFF");
        } else if (c == 'b') {
            bl_on = !bl_on;
            display_eyes_set_brightness(bl_on ? cfg->backlight_percent : 0);
            printf("Backlight: %s\n", bl_on ? "ON" : "OFF");
        } else if (c == 't') {
            display_eyes_perf_t p;
            display_eyes_get_perf(&p);
            printf("PERF: FPS=%.1f render=%luus xfer=%luus free=%lu\n", p.fps, (unsigned long)p.render_time_us, (unsigned long)p.transfer_time_us, (unsigned long)p.free_heap);
        }
    }
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);

    static display_eyes_config_t cfg = {
        .gpio_mosi = 4,
        .gpio_sclk = 5,
        .gpio_dc = 6,
        .gpio_rst = 7,
        .gpio_bl = 15,
        .gpio_cs_left = 16,
        .gpio_cs_right = 17,
        .spi_freq_hz = (40 * 1000 * 1000),  // 40MHz - ESP32-S3 can handle higher speeds
        .backlight_percent = 80,
        .autoblink_enabled = true,
        .tracking_enabled = false,  // Disabled - tracking calculation needs debugging
    };

    ESP_LOGI(TAG, "Init eyes...");
    esp_err_t err = display_eyes_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "display_eyes_init failed: %s", esp_err_to_name(err));
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Backlight sanity blink
    display_eyes_set_brightness(0);
    vTaskDelay(pdMS_TO_TICKS(150));
    display_eyes_set_brightness(100);
    vTaskDelay(pdMS_TO_TICKS(150));
    display_eyes_set_brightness(cfg.backlight_percent);

    const eye_direction_t dirs[] = {
        EYE_LOOK_CENTER,
        EYE_LOOK_LEFT,
        EYE_LOOK_RIGHT,
        EYE_LOOK_UP,
        EYE_LOOK_DOWN,
        EYE_LOOK_UP_LEFT,
        EYE_LOOK_UP_RIGHT,
        EYE_LOOK_DOWN_LEFT,
        EYE_LOOK_DOWN_RIGHT,
    };

    size_t i = 0;
    // Spawn a simple console task to accept a few single-character commands over UART0:
    //  s - toggle smoothing (high-quality rendering)
    //  b - toggle backlight on/off
    //  t - print telemetry immediately
    display_eyes_set_high_quality(EYE_HIGH_QUALITY);

    // Console task will be created below after defining it outside app_main
    xTaskCreatePinnedToCore(console_task, "console", 2048, &cfg, 1, NULL, 1);

    while (1) {
        display_eyes_set_direction(dirs[i % (sizeof(dirs) / sizeof(dirs[0]))]);
        if ((i % 3) == 0) display_eyes_blink();
        ESP_LOGI(TAG, "FPS: %.1f", display_eyes_get_fps());
        i++;
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}
