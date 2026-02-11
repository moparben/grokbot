#include "display_eyes.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// --- Use same wiring as current test setup ---
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
#ifndef EYE_USE_DUAL
#define EYE_USE_DUAL 1
#endif

static const char *TAG = "colorbars_test";
static uint16_t s_frame[EYE_DISPLAY_WIDTH * EYE_DISPLAY_HEIGHT];

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static void make_vertical_bars(uint16_t *buf, int w, int h)
{
    const uint16_t colors[] = {
        rgb565(255, 255, 255), // white
        rgb565(255, 255, 0),   // yellow
        rgb565(0, 255, 255),   // cyan
        rgb565(0, 255, 0),     // green
        rgb565(255, 0, 255),   // magenta
        rgb565(255, 0, 0),     // red
        rgb565(0, 0, 255),     // blue
        rgb565(0, 0, 0),       // black
    };
    const int n = (int)(sizeof(colors) / sizeof(colors[0]));
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int band = (x * n) / w;
            if (band < 0) band = 0;
            if (band >= n) band = n - 1;
            buf[y * w + x] = colors[band];
        }
    }
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "Starting colorbars_test — both eyes");

    display_eyes_config_t cfg = {
        .gpio_mosi = EYE_PIN_MOSI,
        .gpio_sclk = EYE_PIN_SCLK,
        .gpio_dc = EYE_PIN_DC,
        .gpio_rst = EYE_PIN_RST,
        .gpio_bl = EYE_PIN_BL,
        .gpio_cs_left = EYE_PIN_CS_LEFT,
        .gpio_cs_right = (EYE_USE_DUAL ? EYE_PIN_CS_RIGHT : -1),
        .spi_freq_hz = (10 * 1000 * 1000),
        .backlight_percent = 100,
        .autoblink_enabled = false,
        .tracking_enabled = false,
    };

    esp_err_t err = display_eyes_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "display_eyes_init failed: %s", esp_err_to_name(err));
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    display_eyes_set_brightness(100);

    make_vertical_bars(s_frame, EYE_DISPLAY_WIDTH, EYE_DISPLAY_HEIGHT);
    ESP_ERROR_CHECK(display_eyes_show_image(s_frame, s_frame, EYE_DISPLAY_WIDTH, EYE_DISPLAY_HEIGHT));
    ESP_ERROR_CHECK(display_eyes_set_paused(true));

    ESP_LOGI(TAG, "Color bars displayed on both eyes; entering idle loop");
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
}
