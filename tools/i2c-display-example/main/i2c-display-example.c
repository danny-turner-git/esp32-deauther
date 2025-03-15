#include <stdio.h>
#include "esp_err.h"
#include "esp_check.h"
#include "driver/i2c_master.h"

#define I2C_MASTER_SCL_IO 36
#define I2C_MASTER_SDA_IO 33
#define OLED_ADDRESS 0x3c

i2c_master_bus_config_t i2c_bus_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = -1,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
};

void app_main(void)
{

    i2c_master_bus_handle_t bus_handle;
    i2c_new_master_bus(&i2c_bus_config, &bus_handle);

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OLED_ADDRESS,
        .scl_speed_hz = 100000,
    };

    i2c_master_dev_handle_t dev_handle;
    i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);

    uint8_t message= 15;
    uint8_t * ptr = &message;

    i2c_master_transmit(dev_handle, ptr, 1, -1 );


}
