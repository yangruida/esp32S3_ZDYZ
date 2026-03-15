/* xl9555.h */
#pragma once

#include "stdint.h"
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "esp_io_expander.h"
#include "xl9555_config.h"

#ifdef __cplusplus
extern "C" {
#endif
esp_err_t esp_io_expander_new_i2c_xl9555_16bit(i2c_master_bus_handle_t i2c_bus, uint32_t dev_addr,
    esp_io_expander_handle_t *handle_ret);

// esp_err_t read_input_reg(esp_io_expander_handle_t handle, uint32_t *value);


// esp_err_t write_output_reg(esp_io_expander_handle_t handle, uint32_t value);


// esp_err_t read_output_reg(esp_io_expander_handle_t handle, uint32_t *value);


// esp_err_t write_direction_reg(esp_io_expander_handle_t handle, uint32_t value);


// esp_err_t read_direction_reg(esp_io_expander_handle_t handle, uint32_t *value);


// esp_err_t reset(esp_io_expander_t *handle);

// esp_err_t del(esp_io_expander_t *handle);

#ifdef __cplusplus
}
#endif