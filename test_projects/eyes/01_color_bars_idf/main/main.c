// Simple LCD bring-up: backlight blink + color bars.
// Uses the vendored display_eyes component only.

// If your backlight turns ON when BL is driven LOW, set this to 1.
#define EYE_BACKLIGHT_ACTIVE_LOW 0

// Optional: PWM brightness using LEDC (0/1)
#define EYE_BACKLIGHT_USE_LEDC 0

#include "display_eyes.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "color_bars";

static int s_bl_percent = 100;
static bool s_bl_on = true;

static display_eyes_config_t s_cfg;
static uint16_t s_frame[EYE_DISPLAY_WIDTH * EYE_DISPLAY_HEIGHT];

// ---- Pin config (edit these to your wiring) ----
// SPI bus
#ifndef EYE_PIN_MOSI
#define EYE_PIN_MOSI 22
#endif
#ifndef EYE_PIN_SCLK
#define EYE_PIN_SCLK 23
#endif

// LCD control
#ifndef EYE_PIN_DC
#define EYE_PIN_DC 4
#endif
#ifndef EYE_PIN_RST
#define EYE_PIN_RST 5
#endif

// Backlight
#ifndef EYE_PIN_BL
#define EYE_PIN_BL 6
#endif

// Chip selects
#ifndef EYE_PIN_CS_LEFT
#define EYE_PIN_CS_LEFT 20
#endif
#ifndef EYE_PIN_CS_RIGHT
#define EYE_PIN_CS_RIGHT 21
#endif

// 0 = left only, 1 = dual displays
#ifndef EYE_USE_DUAL
#define EYE_USE_DUAL 1
#endif

// 1 = run ONLY the backlight probe loop (no LCD init)
#ifndef EYE_BL_DIAGNOSTIC_ONLY
#define EYE_BL_DIAGNOSTIC_ONLY 0
#endif

static void backlight_probe_raw(int gpio_bl)
{
    if (gpio_bl < 0) return;

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << (uint64_t)gpio_bl,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_LOGI(TAG, "Backlight RAW probe on GPIO %d: holding HIGH then LOW (watch brightness)", gpio_bl);

    ESP_LOGI(TAG, "BL RAW = HIGH (3s)");
    gpio_set_level(gpio_bl, 1);
    vTaskDelay(pdMS_TO_TICKS(3000));

    ESP_LOGI(TAG, "BL RAW = LOW (3s)");
    gpio_set_level(gpio_bl, 0);
    vTaskDelay(pdMS_TO_TICKS(3000));

    ESP_LOGI(TAG, "BL RAW: quick blink x4");
    for (int i = 0; i < 4; i++) {
        gpio_set_level(gpio_bl, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(gpio_bl, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static void print_controls(void)
{
    ESP_LOGI(TAG, "Backlight controls via serial:");
    ESP_LOGI(TAG, "  0 = off, 1 = on, b = toggle");
    ESP_LOGI(TAG, "  + / - = brightness +/- 10%%");
    ESP_LOGI(TAG, "  pNN = set percent (e.g. p25, p100)");
    ESP_LOGI(TAG, "  Cnn = set right display CS pin (e.g. C21) and reinit");
    ESP_LOGI(TAG, "  Lnn = set BL pin (e.g. L6) and reinit");
    ESP_LOGI(TAG, "  D = toggle right display on/off (re-init)");
    ESP_LOGI(TAG, "  d = redraw right eye only (requires right display enabled)");
    ESP_LOGI(TAG, "  I = print current pin assignments");
    ESP_LOGI(TAG, "  w = wake display (SLPOUT + DISPON)");
    ESP_LOGI(TAG, "  v = toggle inversion (INVON/INVOFF)");
    ESP_LOGI(TAG, "  s = cycle solid colors on both displays");
    ESP_LOGI(TAG, "  r = re-init display + redraw bars");
    ESP_LOGI(TAG, "  h = help");
}

static void redraw_bars(void)
{
    const int w = EYE_DISPLAY_WIDTH;
    const int h = EYE_DISPLAY_HEIGHT;
    ESP_LOGI(TAG, "Redrawing color bars...");
    ESP_ERROR_CHECK(display_eyes_show_image(s_frame, (s_cfg.gpio_cs_right >= 0) ? s_frame : NULL, w, h));
    ESP_ERROR_CHECK(display_eyes_set_paused(true));
}

static void redraw_right_only(void)
{
    if (s_cfg.gpio_cs_right < 0) {
        ESP_LOGW(TAG, "Right display disabled; cannot redraw right-only");
        return;
    }
    const int w = EYE_DISPLAY_WIDTH;
    const int h = EYE_DISPLAY_HEIGHT;
    ESP_LOGI(TAG, "Redrawing color bars on RIGHT eye only...");
    /* The display_eyes API requires a non-NULL left buffer, so duplicate
       the frame for left to allow updating the right display safely. */
    esp_err_t _err = display_eyes_show_image(s_frame, s_frame, w, h);
    if (_err != ESP_OK) {
        ESP_LOGW(TAG, "Right redraw failed: %s", esp_err_to_name(_err));
    } else {
        ESP_ERROR_CHECK(display_eyes_set_paused(true));
    }
}

static void reinit_display(void)
{
    if (EYE_BL_DIAGNOSTIC_ONLY) {
        ESP_LOGW(TAG, "EYE_BL_DIAGNOSTIC_ONLY=1: cannot re-init display");
        return;
    }

    ESP_LOGW(TAG, "Reinitializing display...");
    display_eyes_deinit();
    vTaskDelay(pdMS_TO_TICKS(200));
    ESP_ERROR_CHECK(display_eyes_init(&s_cfg));
    redraw_bars();
}

static void apply_backlight(void)
{
    int pct = s_bl_on ? s_bl_percent : 0;
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;

    if (EYE_BL_DIAGNOSTIC_ONLY) {
        // Raw GPIO control only (no component init)
        int level = (pct > 0) ? 1 : 0;
#if EYE_BACKLIGHT_ACTIVE_LOW
        level = !level;
#endif
        gpio_set_level(EYE_PIN_BL, level);
        ESP_LOGI(TAG, "BL raw => %s (active_%s)", (pct > 0) ? "ON" : "OFF", EYE_BACKLIGHT_ACTIVE_LOW ? "low" : "high");
    } else {
        display_eyes_set_brightness((uint8_t)pct);
    }
}

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);
static void uart_control_task(void *arg)
{
    (void)arg;

    // UART0 is what idf.py monitor uses.
    // Driver may already be installed; treat that as OK.
    esp_err_t err = uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "uart_driver_install failed: %s", esp_err_to_name(err));
    }

    print_controls();
    apply_backlight();

    // State for runtime tests
    static bool s_inverted = false;
    static int s_solid_idx = -1;
    const uint16_t s_solid_colors[] = {
        rgb565(255,255,255), // white
        rgb565(255,0,0),     // red
        rgb565(0,255,0),     // green
        rgb565(0,0,255),     // blue
        rgb565(0,0,0),       // black
    };

    char cmd_buf[8] = {0};
    int cmd_len = 0;
    TickType_t last_rx_tick = 0;

    // Helper to print current pin mapping
    void print_pin_config(void)
    {
        ESP_LOGI(TAG, "Pins: MOSI=%d SCLK=%d DC=%d RST=%d BL=%d CS_L=%d CS_R=%d (dual=%d)",
                 s_cfg.gpio_mosi, s_cfg.gpio_sclk, s_cfg.gpio_dc, s_cfg.gpio_rst, s_cfg.gpio_bl,
                 s_cfg.gpio_cs_left, s_cfg.gpio_cs_right, (s_cfg.gpio_cs_right >= 0));
    }

    while (1) {
        uint8_t ch = 0;
        int n = uart_read_bytes(UART_NUM_0, &ch, 1, pdMS_TO_TICKS(100));
        if (n <= 0) {
            // If the user types a multi-char command (e.g. p0, C21, L6) without pressing Enter, commit after a short idle timeout.
            if (cmd_len > 1) {
                TickType_t now = xTaskGetTickCount();
                if ((now - last_rx_tick) > pdMS_TO_TICKS(400)) {
                    cmd_buf[cmd_len] = 0;
                    if (cmd_buf[0] == 'p' || cmd_buf[0] == 'P') {
                        int val = atoi(&cmd_buf[1]);
                        s_bl_percent = val;
                        s_bl_on = (val > 0);
                        ESP_LOGI(TAG, "Set BL percent => %d%% (auto-commit)", s_bl_percent);
                        apply_backlight();
                    } else if (cmd_buf[0] == 'C' || cmd_buf[0] == 'c') {
                        int val = atoi(&cmd_buf[1]);
                        if (val >= 0) {
                            s_cfg.gpio_cs_right = val;
                            ESP_LOGI(TAG, "Set right CS => GPIO%d (auto-commit)", val);
                            reinit_display();
                        }
                    } else if (cmd_buf[0] == 'L' || cmd_buf[0] == 'l') {
                        int val = atoi(&cmd_buf[1]);
                        if (val >= 0) {
                            s_cfg.gpio_bl = val;
                            ESP_LOGI(TAG, "Set BL pin => GPIO%d (auto-commit)", val);
                            reinit_display();
                        }
                    }
                    cmd_len = 0;
                }
            }
            continue;
        }

        last_rx_tick = xTaskGetTickCount();

        if (ch == '\r' || ch == '\n') {
            if (cmd_len == 0) continue;

            cmd_buf[cmd_len] = 0;
            if (cmd_buf[0] == 'p' || cmd_buf[0] == 'P') {
                int val = atoi(&cmd_buf[1]);
                s_bl_percent = val;
                s_bl_on = (val > 0);
                ESP_LOGI(TAG, "Set BL percent => %d%%", s_bl_percent);
                apply_backlight();
            } else if (cmd_buf[0] == 'C' || cmd_buf[0] == 'c') {
                int val = atoi(&cmd_buf[1]);
                if (val >= 0) {
                    s_cfg.gpio_cs_right = val;
                    ESP_LOGI(TAG, "Set right CS => GPIO%d (will reinit)", val);
                    reinit_display();
                } else {
                    ESP_LOGW(TAG, "Invalid CS value: %s", &cmd_buf[1]);
                }
            } else if (cmd_buf[0] == 'L' || cmd_buf[0] == 'l') {
                int val = atoi(&cmd_buf[1]);
                if (val >= 0) {
                    s_cfg.gpio_bl = val;
                    ESP_LOGI(TAG, "Set BL pin => GPIO%d (will reinit)", val);
                    reinit_display();
                } else {
                    ESP_LOGW(TAG, "Invalid BL value: %s", &cmd_buf[1]);
                }
            } else {
                ESP_LOGW(TAG, "Unknown command: %s", cmd_buf);
                print_controls();
            }

            cmd_len = 0;
            continue;
        }

        // Single-key commands
        if (ch == 'h' || ch == 'H') {
            print_controls();
            cmd_len = 0;
            continue;
        }
        if (ch == 'I' || ch == 'i') {
            print_pin_config();
            cmd_len = 0;
            continue;
        }
        if (ch == 'r' || ch == 'R') {
            reinit_display();
            cmd_len = 0;
            continue;
        }
        if (ch == 'D') {
            // Toggle right display on/off (re-init required)
            if (s_cfg.gpio_cs_right >= 0) {
                ESP_LOGW(TAG, "Disabling right display, reinitializing...");
                s_cfg.gpio_cs_right = -1;
            } else {
                ESP_LOGW(TAG, "Enabling right display, reinitializing (CS=%d)...", EYE_PIN_CS_RIGHT);
                s_cfg.gpio_cs_right = EYE_PIN_CS_RIGHT;
            }
            reinit_display();
            cmd_len = 0;
            continue;
        }
        if (ch == 'd') {
            // Redraw right-only color bars
            redraw_right_only();
            cmd_len = 0;
            continue;
        }
        if (ch == 'w' || ch == 'W') {
            ESP_LOGI(TAG, "Wake displays (SLPOUT + DISPON)");
            display_eyes_wake();
            cmd_len = 0;
            continue;
        }
        if (ch == 'v' || ch == 'V') {
            s_inverted = !s_inverted;
            ESP_LOGI(TAG, "Invert => %s", s_inverted ? "ON" : "OFF");
            display_eyes_set_inversion(s_inverted);
            cmd_len = 0;
            continue;
        }
        if (ch == 's' || ch == 'S') {
            s_solid_idx = (s_solid_idx + 1) % (int)(sizeof(s_solid_colors)/sizeof(s_solid_colors[0]));
            uint16_t col = s_solid_colors[s_solid_idx];
            ESP_LOGI(TAG, "Solid color test => idx=%d", s_solid_idx);
            display_eyes_fill_solid(col, false);
            display_eyes_set_paused(true);
            cmd_len = 0;
            continue;
        }
        if (ch == '0') {
            s_bl_on = false;
            ESP_LOGI(TAG, "BL => OFF");
            apply_backlight();
            cmd_len = 0;
            continue;
        }
        if (ch == '1') {
            s_bl_on = true;
            ESP_LOGI(TAG, "BL => ON (%d%%)", s_bl_percent);
            apply_backlight();
            cmd_len = 0;
            continue;
        }
        if (ch == 'b' || ch == 'B') {
            s_bl_on = !s_bl_on;
            ESP_LOGI(TAG, "BL => %s", s_bl_on ? "ON" : "OFF");
            apply_backlight();
            cmd_len = 0;
            continue;
        }
        if (ch == '+') {
            s_bl_percent += 10;
            if (s_bl_percent > 100) s_bl_percent = 100;
            s_bl_on = (s_bl_percent > 0);
            ESP_LOGI(TAG, "BL => %d%%", s_bl_percent);
            apply_backlight();
            cmd_len = 0;
            continue;
        }
        if (ch == '-') {
            s_bl_percent -= 10;
            if (s_bl_percent < 0) s_bl_percent = 0;
            s_bl_on = (s_bl_percent > 0);
            ESP_LOGI(TAG, "BL => %d%%", s_bl_percent);
            apply_backlight();
            cmd_len = 0;
            continue;
        }

        // Start of a multi-char command (e.g. p25, C21, L6)
        if ((ch == 'p' || ch == 'P' || ch == 'C' || ch == 'c' || ch == 'L' || ch == 'l') && cmd_len == 0) {
            cmd_buf[0] = (char)ch;
            cmd_len = 1;
            continue;
        }

        // Accumulate digits after multi-char commands
        if (cmd_len > 0) {
            if (cmd_len < (int)(sizeof(cmd_buf) - 1) && ch >= '0' && ch <= '9') {
                cmd_buf[cmd_len++] = (char)ch;
            } else {
                cmd_len = 0;
            }
        }
    }
}

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

    ESP_LOGI(TAG, "Pins: MOSI=%d SCLK=%d DC=%d RST=%d BL=%d CS_L=%d CS_R=%d (dual=%d)",
             EYE_PIN_MOSI, EYE_PIN_SCLK, EYE_PIN_DC, EYE_PIN_RST, EYE_PIN_BL, EYE_PIN_CS_LEFT, EYE_PIN_CS_RIGHT,
             (int)EYE_USE_DUAL);

    // Probe BL polarity BEFORE LCD init (helps find correct BL pin + polarity fast).
    if (EYE_BL_DIAGNOSTIC_ONLY) {
        ESP_LOGW(TAG, "EYE_BL_DIAGNOSTIC_ONLY=1: skipping LCD init; looping raw BL toggle");
        // Start serial-controlled backlight task and keep the app alive.
        xTaskCreate(uart_control_task, "bl_uart", 4096, NULL, 5, NULL);
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    } else {
        backlight_probe_raw(EYE_PIN_BL);
    }

    // These defaults match the earlier MAX98357A test wiring you were using.
    s_cfg = (display_eyes_config_t){
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

    ESP_LOGI(TAG, "Init displays...");
    esp_err_t err = display_eyes_init(&s_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "display_eyes_init failed: %s", esp_err_to_name(err));
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Backlight blink to confirm BL pin/polarity (using component's polarity setting)
    display_eyes_set_brightness(0);
    vTaskDelay(pdMS_TO_TICKS(250));
    display_eyes_set_brightness(100);
    vTaskDelay(pdMS_TO_TICKS(250));
    display_eyes_set_brightness(s_cfg.backlight_percent);

    make_vertical_bars(s_frame, EYE_DISPLAY_WIDTH, EYE_DISPLAY_HEIGHT);
    ESP_LOGI(TAG, "Showing color bars (left%s)...", EYE_USE_DUAL ? "+right" : " only");
    redraw_bars();

    // Allow runtime BL control from idf.py monitor
    s_bl_percent = s_cfg.backlight_percent;
    s_bl_on = (s_cfg.backlight_percent > 0);
    xTaskCreate(uart_control_task, "bl_uart", 4096, NULL, 5, NULL);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
