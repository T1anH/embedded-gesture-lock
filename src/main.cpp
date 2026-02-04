
// Group members:
// 1. Vaibhav Rouduri - vr2470
// 2. Tian Huang - th2334
// 3. Puneeth Kotha - pk3058

#include <mbed.h>
#include <vector>
#include <cmath>
#include "LCD_DISCO_F429ZI.h"

// --- Register Addresses and Configuration Values ---
#define CTRL_REG1 0x20               // Control register 1 address
#define CTRL_REG1_CONFIG 0b01'10'1'1'1'1  // Configuration: ODR=100Hz, Enable X/Y/Z axes, power on
#define CTRL_REG4 0x23               // Control register 4 address
#define CTRL_REG4_CONFIG 0b0'0'01'0'00'0  // Configuration: High-resolution, 2000dps sensitivity

#define SPI_FLAG 1
#define OUT_X_L 0x28
#define DEG_TO_RAD (17.5f * 0.0174532925199432957692236907684886f / 1000.0f)

EventFlags flags;

void spi_cb(int event) {
    flags.set(SPI_FLAG);  // Set the SPI_FLAG to signal that transfer is complete
}

// Helper functions for statistical calculations
float calculate_mean(const std::vector<float>& data) {
    float sum = 0;
    for (float value : data) {
        sum += value;
    }
    return sum / data.size();
}

float calculate_variance(const std::vector<float>& data, float mean) {
    float variance = 0;
    for (float value : data) {
        variance += (value - mean) * (value - mean);
    }
    return variance / data.size();
}

float calculate_std_dev(float variance) {
    return sqrt(variance);
}

float calculate_covariance(const std::vector<float>& data1, const std::vector<float>& data2, float mean1, float mean2) {
    float covariance = 0;
    for (size_t i = 0; i < data1.size(); ++i) {
        covariance += (data1[i] - mean1) * (data2[i] - mean2);
    }
    return covariance / data1.size();
}

float calculate_normalized_covariance(const std::vector<float>& data1, const std::vector<float>& data2, float mean1, float mean2, float std_dev1, float std_dev2) {
    float covariance = calculate_covariance(data1, data2, mean1, mean2);
    return covariance / (std_dev1 * std_dev2);
}

int main() {
    SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel);
    DigitalOut led1(LED1);
    DigitalOut led2(LED2);
    //DigitalOut led3(PB_13);

    LCD_DISCO_F429ZI lcd;
    lcd.Clear(LCD_COLOR_BLACK);

    uint8_t write_buf[32], read_buf[32];

    spi.format(8, 3);
    spi.frequency(1'000'000);

    write_buf[0] = CTRL_REG1;
    write_buf[1] = CTRL_REG1_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

    write_buf[0] = CTRL_REG4;
    write_buf[1] = CTRL_REG4_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

    printf("Learning mode initiated. Perform the gesture as instructed.\n");

    std::vector<std::vector<float>> gx_records, gy_records, gz_records;

    bool gestures_recorded = false;

    while (true) {
        if (!gestures_recorded) {
            for (int i = 0; i < 5; ++i) {
                char buffer[500];
                snprintf(buffer, sizeof(buffer), "Prepare to gesture %d", i + 1);
                printf("Prepare to gesture %d\n", i + 1);
                lcd.Clear(LCD_COLOR_WHITE);
                // lcd.SetFont(&Font16);
                lcd.DisplayStringAt(0, LINE(5), (uint8_t *)buffer, CENTER_MODE);
                lcd.DisplayStringAt(0, LINE(7), (uint8_t *)"For 4s", CENTER_MODE);
                thread_sleep_for(3000); // 3-second delay to ensure readability

                snprintf(buffer, sizeof(buffer), "Perform gesture %d", i + 1);
                printf("Peform gesture %d\n", i + 1);
                lcd.Clear(LCD_COLOR_WHITE);
                lcd.DisplayStringAt(0, LINE(5), (uint8_t *)buffer, CENTER_MODE);
                thread_sleep_for(3000); // Extended timing for clear instruction

                std::vector<float> gx_samples, gy_samples, gz_samples;

                for (int t = 0; t < 40; ++t) { // 40 readings over 4 seconds (100ms per reading)
                    uint16_t raw_gx, raw_gy, raw_gz;
                    float gx, gy, gz;

                    write_buf[0] = OUT_X_L | 0x80 | 0x40;
                    spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
                    flags.wait_all(SPI_FLAG);

                    raw_gx = (((uint16_t)read_buf[2]) << 8) | read_buf[1];
                    raw_gy = (((uint16_t)read_buf[4]) << 8) | read_buf[3];
                    raw_gz = (((uint16_t)read_buf[6]) << 8) | read_buf[5];

                    gx = raw_gx * DEG_TO_RAD;
                    gy = raw_gy * DEG_TO_RAD;
                    gz = raw_gz * DEG_TO_RAD;

                    gx_samples.push_back(gx);
                    gy_samples.push_back(gy);
                    gz_samples.push_back(gz);

                    thread_sleep_for(100); // 100ms delay between readings
                }

                gx_records.push_back(gx_samples);
                gy_records.push_back(gy_samples);
                gz_records.push_back(gz_samples);

                snprintf(buffer, sizeof(buffer), "Gesture %d recorded", i + 1);
                printf("Gesture %d recorded\n", i + 1);
                lcd.Clear(LCD_COLOR_WHITE);
                lcd.DisplayStringAt(0, LINE(5), (uint8_t *)buffer, CENTER_MODE);
                thread_sleep_for(3000); // Ensure user sees confirmation
            }

            printf("Learning mode completed.\n");
            lcd.Clear(LCD_COLOR_WHITE);
            lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Learning Complete", CENTER_MODE);
            thread_sleep_for(3000);
            gestures_recorded = true;
        } else {
            lcd.Clear(LCD_COLOR_WHITE);
            lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Recognition Mode", CENTER_MODE);
            printf("Recognition mode: Get ready to perform your gesture.\n");
            thread_sleep_for(3000); // 3-second delay

            lcd.Clear(LCD_COLOR_WHITE);
            lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Perform Gesture", CENTER_MODE);
            printf("Perform your gesture now for 4 seconds.\n");

            std::vector<float> gx_test, gy_test, gz_test;

            for (int t = 0; t < 40; ++t) {
                uint16_t raw_gx, raw_gy, raw_gz;
                float gx, gy, gz;

                write_buf[0] = OUT_X_L | 0x80 | 0x40;
                spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
                flags.wait_all(SPI_FLAG);

                raw_gx = (((uint16_t)read_buf[2]) << 8) | read_buf[1];
                raw_gy = (((uint16_t)read_buf[4]) << 8) | read_buf[3];
                raw_gz = (((uint16_t)read_buf[6]) << 8) | read_buf[5];

                gx = raw_gx * DEG_TO_RAD;
                gy = raw_gy * DEG_TO_RAD;
                gz = raw_gz * DEG_TO_RAD;

                gx_test.push_back(gx);
                gy_test.push_back(gy);
                gz_test.push_back(gz);

                thread_sleep_for(100);
            }

            printf("Gesture recording completed. Processing...\n");
            lcd.Clear(LCD_COLOR_WHITE);
            lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Processing", CENTER_MODE);

            float mean_gx_test = calculate_mean(gx_test);
            float mean_gy_test = calculate_mean(gy_test);
            float mean_gz_test = calculate_mean(gz_test);

            float std_dev_gx_test = calculate_std_dev(calculate_variance(gx_test, mean_gx_test));
            float std_dev_gy_test = calculate_std_dev(calculate_variance(gy_test, mean_gy_test));
            float std_dev_gz_test = calculate_std_dev(calculate_variance(gz_test, mean_gz_test));

            bool unlock_success = false;

            for (int i = 0; i < 5; ++i) {
                float mean_gx = calculate_mean(gx_records[i]);
                float mean_gy = calculate_mean(gy_records[i]);
                float mean_gz = calculate_mean(gz_records[i]);

                float std_dev_gx = calculate_std_dev(calculate_variance(gx_records[i], mean_gx));
                float std_dev_gy = calculate_std_dev(calculate_variance(gy_records[i], mean_gy));
                float std_dev_gz = calculate_std_dev(calculate_variance(gx_records[i], mean_gz));

                float norm_cov_gx_gy = calculate_normalized_covariance(gx_records[i], gy_records[i], mean_gx, mean_gy, std_dev_gx, std_dev_gx);
                float norm_cov_gy_gz = calculate_normalized_covariance(gy_records[i], gz_records[i], mean_gy, mean_gz, std_dev_gy, std_dev_gz);
                float norm_cov_gz_gx = calculate_normalized_covariance(gz_records[i], gx_records[i], mean_gz, mean_gx, std_dev_gz, std_dev_gx);

                float norm_cov_gx_gy_test = calculate_normalized_covariance(gx_test, gy_test, mean_gx_test, mean_gy_test, std_dev_gx_test, std_dev_gx_test);
                float norm_cov_gy_gz_test = calculate_normalized_covariance(gy_test, gz_test, mean_gy_test, mean_gz_test, std_dev_gy_test, std_dev_gz_test);
                float norm_cov_gz_gx_test = calculate_normalized_covariance(gz_test, gx_test, mean_gz_test, mean_gx_test, std_dev_gz_test, std_dev_gx_test);

                // Increased threshold for better matching tolerance
                if (fabs(norm_cov_gx_gy - norm_cov_gx_gy_test) < 0.15 &&
                    fabs(norm_cov_gy_gz - norm_cov_gy_gz_test) < 0.15 &&
                    fabs(norm_cov_gz_gx - norm_cov_gz_gx_test) < 0.15) {
                    unlock_success = true;
                    break;
                }
            }

            if (unlock_success) {
                printf("Gesture recognized! Unlock successful.\n");
                lcd.Clear(LCD_COLOR_WHITE);
                lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Unlock Successful", CENTER_MODE);
                led1 = 1; // Green
                led2 = 0; // Red
                //led3 = 1;

                for (int i = 0; i < 20; ++i) { // Blink green for 10 seconds
                    lcd.Clear(LCD_COLOR_GREEN);
                    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Unlocked!!!", CENTER_MODE);
                    thread_sleep_for(500);
                    lcd.Clear(LCD_COLOR_WHITE);
                    thread_sleep_for(500);
                }

                return 0; // End the program
            } else {
                printf("Gesture not recognized. Try again.\n");
                lcd.Clear(LCD_COLOR_WHITE);
                lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Try Again!!!", CENTER_MODE);
                led1 = 0;
                led2 = 1;

                // Blink red 3 times for unsuccessful unlock
                for (int i = 0; i < 3; ++i) {
                    lcd.Clear(LCD_COLOR_RED);
                    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Try Again!!!", CENTER_MODE);
                    thread_sleep_for(500);
                    lcd.Clear(LCD_COLOR_WHITE);
                    thread_sleep_for(500);
                }
            }
        }
    }
    lcd.SetFont(&Font24);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Hello!", CENTER_MODE);
    return 0;
}
