//THIS IS NOT MY CODE
//This is an AI generated response to convert the code from arduinogetstarted.com/faq/how-toknow-i2c-address-of-sensor-device
//from arduino framework to esp-idf.
//This was used as a tool to find the i2c address of my OLED module because I couldn't find the datasheet.
//OLED module has address 0x3c.
#include <esp_log.h>
#include <driver/i2c.h>

// Define the I2C port and pins
#define I2C_PORT I2C_NUM_0
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// Define the I2C frequency
#define I2C_FREQ 400000

// Define the I2C address range to scan
#define I2C_ADDR_MIN 0x01
#define I2C_ADDR_MAX 0x7F

void i2c_scanner(void)
{
    // Initialize the I2C driver
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ,
    };

    // Create the I2C driver
    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);

    ESP_LOGI("I2C", "Scanning I2C bus...");

    for (uint8_t addr = I2C_ADDR_MIN; addr <= I2C_ADDR_MAX; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            ESP_LOGI("I2C", "I2C device found at address 0x%02x", addr);
        } else if (ret == ESP_ERR_TIMEOUT) {
            // Ignore timeout errors
        } else {
            ESP_LOGW("I2C", "Error scanning address 0x%02x: %s", addr, esp_err_to_name(ret));
        }
    }

    // Uninstall the I2C driver
    i2c_driver_delete(I2C_PORT);
}

void app_main(void)
{
    i2c_scanner();
}
