/* xl9555.c */
#include "xl9555.h"
// #include "esp_check.h"
// #include "esp_log.h"
// #include "esp_bit_defs.h"
// #include "string.h"
// #include "stdlib.h"
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "esp_bit_defs.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_io_expander.h"
#include "xl9555_config.h"
// 设备私有数据结构
typedef struct {
    esp_io_expander_t base;
    i2c_master_dev_handle_t i2c_handle;
    struct {
        uint16_t direction;
        uint16_t output;
    } regs;
} esp_io_expander_xl9555_16bit_t;

// 静态函数声明
 esp_err_t read_input_reg(esp_io_expander_handle_t handle, uint32_t *value);
 esp_err_t write_output_reg(esp_io_expander_handle_t handle, uint32_t value);
 esp_err_t read_output_reg(esp_io_expander_handle_t handle, uint32_t *value);
 esp_err_t write_direction_reg(esp_io_expander_handle_t handle, uint32_t value);
 esp_err_t read_direction_reg(esp_io_expander_handle_t handle, uint32_t *value);
 esp_err_t reset(esp_io_expander_t *handle);
 esp_err_t del(esp_io_expander_t *handle);

/************************* 核心API实现 *************************/
esp_err_t esp_io_expander_new_i2c_xl9555_16bit(i2c_master_bus_handle_t i2c_bus, uint32_t dev_addr,
        esp_io_expander_handle_t *handle_ret)
{
    ESP_RETURN_ON_FALSE(handle_ret != NULL, ESP_ERR_INVALID_ARG, TAG, "Invalid handle_ret");

    // Allocate memory for driver object
    esp_io_expander_xl9555_16bit_t *tca = (esp_io_expander_xl9555_16bit_t *)calloc(1,
                                           sizeof(esp_io_expander_xl9555_16bit_t));
    ESP_RETURN_ON_FALSE(tca, ESP_ERR_NO_MEM, TAG, "Malloc failed");

    // Add new I2C device
    esp_err_t ret = ESP_OK;
    const i2c_device_config_t i2c_dev_cfg = {
        .device_address = dev_addr,
        .scl_speed_hz = I2C_CLK_SPEED,
    };
    ESP_GOTO_ON_ERROR(i2c_master_bus_add_device(i2c_bus, &i2c_dev_cfg, &tca->i2c_handle), err, TAG,
                      "Add new I2C device failed");

    tca->base.config.io_count = IO_COUNT;
    tca->base.config.flags.dir_out_bit_zero = 1;
    tca->base.read_input_reg = read_input_reg;
    tca->base.write_output_reg = write_output_reg;
    tca->base.read_output_reg = read_output_reg;
    tca->base.write_direction_reg = write_direction_reg;
    tca->base.read_direction_reg = read_direction_reg;
    tca->base.del = del;
    tca->base.reset = reset;

    /* Reset configuration and register status */
    ESP_GOTO_ON_ERROR(reset(&tca->base), err, TAG, "Reset failed");

    *handle_ret = &tca->base;
    return ESP_OK;
err:
    free(tca);
    return ret;
}

 esp_err_t read_input_reg(esp_io_expander_handle_t handle, uint32_t *value)
{
    esp_io_expander_xl9555_16bit_t *tca = (esp_io_expander_xl9555_16bit_t *)__containerof(handle,
                                           esp_io_expander_xl9555_16bit_t, base);

    uint8_t temp[2] = {0, 0};
    ESP_RETURN_ON_ERROR(i2c_master_transmit_receive(tca->i2c_handle, (uint8_t[]) {
        INPUT_REG_ADDR
    }, 1, temp, sizeof(temp), I2C_TIMEOUT_MS), TAG, "Read input reg failed");
    *value = (((uint32_t)temp[1]) << 8) | (temp[0]);
    return ESP_OK;
}

 esp_err_t write_output_reg(esp_io_expander_handle_t handle, uint32_t value)
{
    esp_io_expander_xl9555_16bit_t *tca = (esp_io_expander_xl9555_16bit_t *)__containerof(handle,
                                           esp_io_expander_xl9555_16bit_t, base);
    value &= 0xffff;

    uint8_t data[] = {OUTPUT_REG_ADDR, value & 0xff, value >> 8};
    ESP_RETURN_ON_ERROR(i2c_master_transmit(tca->i2c_handle, data, sizeof(data), I2C_TIMEOUT_MS), TAG,
                        "Write output reg failed");
    tca->regs.output = value;
    return ESP_OK;
}

 esp_err_t read_output_reg(esp_io_expander_handle_t handle, uint32_t *value)
{
    esp_io_expander_xl9555_16bit_t *tca = (esp_io_expander_xl9555_16bit_t *)__containerof(handle,
                                           esp_io_expander_xl9555_16bit_t, base);

    *value = tca->regs.output;
    return ESP_OK;
}

 esp_err_t write_direction_reg(esp_io_expander_handle_t handle, uint32_t value)
{
    esp_io_expander_xl9555_16bit_t *tca = (esp_io_expander_xl9555_16bit_t *)__containerof(handle,
                                           esp_io_expander_xl9555_16bit_t, base);
    value &= 0xffff;

    uint8_t data[] = {DIRECTION_REG_ADDR, value & 0xff, value >> 8};
    ESP_RETURN_ON_ERROR(i2c_master_transmit(tca->i2c_handle, data, sizeof(data), I2C_TIMEOUT_MS), TAG,
                        "Write direction reg failed");
    tca->regs.direction = value;
    return ESP_OK;
}

 esp_err_t read_direction_reg(esp_io_expander_handle_t handle, uint32_t *value)
{
    esp_io_expander_xl9555_16bit_t *tca = (esp_io_expander_xl9555_16bit_t *)__containerof(handle,
                                           esp_io_expander_xl9555_16bit_t, base);

    *value = tca->regs.direction;
    return ESP_OK;
}

 esp_err_t reset(esp_io_expander_t *handle)
{
    ESP_RETURN_ON_ERROR(write_direction_reg(handle, DIR_REG_DEFAULT_VAL), TAG, "Write dir reg failed");
    ESP_RETURN_ON_ERROR(write_output_reg(handle, OUT_REG_DEFAULT_VAL), TAG, "Write output reg failed");
    return ESP_OK;
}

 esp_err_t del(esp_io_expander_t *handle)
{
    esp_io_expander_xl9555_16bit_t *tca = (esp_io_expander_xl9555_16bit_t *)__containerof(handle,
                                           esp_io_expander_xl9555_16bit_t, base);

    ESP_RETURN_ON_ERROR(i2c_master_bus_rm_device(tca->i2c_handle), TAG, "Remove I2C device failed");
    free(tca);
    return ESP_OK;
}

/************************* 静态函数实现 *************************/

// {
//     ESP_RETURN_ON_FALSE(handle != NULL, ESP_ERR_INVALID_ARG, XL9555_TAG, "handle is NULL");
//     xl9555_dev_t *dev = __containerof(handle, xl9555_dev_t, base);

//     esp_err_t ret = i2c_master_bus_rm_device(dev->i2c_dev_handle);
//     if (ret != ESP_OK) {
//         ESP_LOGE(XL9555_TAG, "Remove I2C device failed (0x%02X)", ret);
//     }
//     free(dev);
//     ESP_LOGI(XL9555_TAG, "XL9555 destroy success");
//     return ret;
// }