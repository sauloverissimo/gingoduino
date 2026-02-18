// I2S DAC Pin Scanner — Minimal test for PCM5102 on T-Display-S3 MIDI Shield
//
// Tests ALL available GPIO combinations to find which pins connect to
// the PCM5102 DAC on the MIDI Shield V1.1.
//
// HOW TO USE:
//   1. Connect headphones/speaker to the 3.5mm jack
//   2. Open Serial Monitor at 115200 baud
//   3. Upload this sketch
//   4. Listen — each test plays 440Hz for 1.5 seconds
//   5. Report which test number produces sound
//
// After the scan, it plays a continuous tone for multimeter probing.

#include <driver/i2s.h>
#include <math.h>

#define SAMPLE_RATE     44100
#define BUFFER_FRAMES   256

static int16_t* buf = NULL;

void playTone(int bck, int ws, int dout, int testNum) {
    i2s_config_t cfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = BUFFER_FRAMES,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0,
    };

    i2s_pin_config_t pins = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = bck,
        .ws_io_num = ws,
        .data_out_num = dout,
        .data_in_num = I2S_PIN_NO_CHANGE,
    };

    Serial.printf("\n[Test %02d] BCK=GPIO%d  WS=GPIO%d  DOUT=GPIO%d\n",
                  testNum, bck, ws, dout);

    esp_err_t err = i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("  SKIP (driver install err=%d)\n", err);
        return;
    }

    err = i2s_set_pin(I2S_NUM_0, &pins);
    if (err != ESP_OK) {
        Serial.printf("  SKIP (set_pin err=%d)\n", err);
        i2s_driver_uninstall(I2S_NUM_0);
        return;
    }

    i2s_zero_dma_buffer(I2S_NUM_0);
    Serial.println("  Playing 440Hz...");

    // Play 440Hz for 1.5 seconds
    float phase = 0.0f;
    const float freq = 440.0f;
    const int blocks = (int)((SAMPLE_RATE * 1.5f) / BUFFER_FRAMES);

    for (int b = 0; b < blocks; b++) {
        for (int i = 0; i < BUFFER_FRAMES; i++) {
            phase += (2.0f * M_PI * freq) / SAMPLE_RATE;
            if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
            int16_t s = (int16_t)(sinf(phase) * 30000.0f);
            buf[i * 2]     = s;
            buf[i * 2 + 1] = s;
        }
        size_t bw = 0;
        i2s_write(I2S_NUM_0, buf, BUFFER_FRAMES * 2 * sizeof(int16_t),
                  &bw, portMAX_DELAY);
    }

    // Brief silence
    memset(buf, 0, BUFFER_FRAMES * 2 * sizeof(int16_t));
    size_t bw = 0;
    i2s_write(I2S_NUM_0, buf, BUFFER_FRAMES * 2 * sizeof(int16_t),
              &bw, portMAX_DELAY);

    i2s_driver_uninstall(I2S_NUM_0);
    delay(300);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // T-Display-S3 power enable
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);

    buf = (int16_t*)malloc(BUFFER_FRAMES * 2 * sizeof(int16_t));

    Serial.println("\n========================================");
    Serial.println("  PCM5102 DAC Pin Scanner");
    Serial.println("  T-Display-S3 MIDI Shield V1.1");
    Serial.println("========================================");
    Serial.println("Headphones on 3.5mm jack.");
    Serial.println("Listen for 440Hz tone on each test.\n");

    int t = 1;

    // --- Phase 1: All 6 permutations of GPIO 10/11/12 ---
    Serial.println("--- Phase 1: GPIO 10/11/12 ---");
    playTone(10, 11, 12, t++);  //  1
    playTone(10, 12, 11, t++);  //  2
    playTone(11, 10, 12, t++);  //  3
    playTone(11, 12, 10, t++);  //  4
    playTone(12, 10, 11, t++);  //  5
    playTone(12, 11, 10, t++);  //  6

    // --- Phase 2: Include GPIO 13 ---
    Serial.println("\n--- Phase 2: +GPIO 13 ---");
    playTone(11, 13, 12, t++);  //  7
    playTone(12, 13, 11, t++);  //  8
    playTone(13, 10, 12, t++);  //  9
    playTone(13, 11, 12, t++);  // 10
    playTone(10, 13, 12, t++);  // 11
    playTone(10, 13, 11, t++);  // 12
    playTone(11, 10, 13, t++);  // 13
    playTone(12, 10, 13, t++);  // 14
    playTone(12, 13, 10, t++);  // 15
    playTone(13, 12, 11, t++);  // 16
    playTone(13, 10, 11, t++);  // 17
    playTone(10, 11, 13, t++);  // 18

    // --- Phase 3: GPIO 1/2/3 (left header, top) ---
    Serial.println("\n--- Phase 3: GPIO 1/2/3 ---");
    playTone(1, 2, 3,   t++);  // 19
    playTone(2, 1, 3,   t++);  // 20
    playTone(3, 1, 2,   t++);  // 21

    // --- Phase 4: Right-side GPIOs (17/18/21/16/43/44) ---
    Serial.println("\n--- Phase 4: Right-side GPIOs ---");
    playTone(17, 18, 21, t++); // 22
    playTone(18, 17, 21, t++); // 23
    playTone(21, 17, 18, t++); // 24
    playTone(43, 44, 21, t++); // 25
    playTone(16, 17, 18, t++); // 26

    Serial.println("\n========================================");
    Serial.printf("  SCAN COMPLETE — %d tests done\n", t - 1);
    Serial.println("  Which test produced sound?");
    Serial.println("========================================");
    Serial.println("\nNow: continuous 440Hz on BCK=11 WS=10 DOUT=12");
    Serial.println("Use multimeter on 3.5mm jack:");
    Serial.println("  Tip=Left  Ring=Right  Sleeve=GND");
    Serial.println("  Expected: ~0.5V AC if DAC works\n");

    // Continuous tone for probing
    i2s_config_t cfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = BUFFER_FRAMES,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0,
    };
    i2s_pin_config_t pins = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = 11,
        .ws_io_num = 10,
        .data_out_num = 12,
        .data_in_num = I2S_PIN_NO_CHANGE,
    };
    i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pins);
    i2s_zero_dma_buffer(I2S_NUM_0);
}

static float loopPhase = 0.0f;

void loop() {
    const float freq = 440.0f;
    for (int i = 0; i < BUFFER_FRAMES; i++) {
        loopPhase += (2.0f * M_PI * freq) / SAMPLE_RATE;
        if (loopPhase > 2.0f * M_PI) loopPhase -= 2.0f * M_PI;
        int16_t s = (int16_t)(sinf(loopPhase) * 30000.0f);
        buf[i * 2]     = s;
        buf[i * 2 + 1] = s;
    }
    size_t bw = 0;
    i2s_write(I2S_NUM_0, buf, BUFFER_FRAMES * 2 * sizeof(int16_t),
              &bw, portMAX_DELAY);
}
