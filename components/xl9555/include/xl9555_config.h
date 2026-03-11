/* xl9555_config.h */
/**
 * 
 * 主要内容包括：XL9555 IO扩展器组件的基础配置和寄存器定义，通过修改IIC时钟频率、设备地址等宏定义来适配不同的硬件环境和需求。
 * 该文件还定义了设备的寄存器地址和默认值，供组件实现文件使用。同时也包含了一些测试相关的配置宏，方便在测试代码中使用
 * 
 * 
 */
#pragma once

#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************* 基础配置 *************************/

/* I2C communication related */
#define I2C_TIMEOUT_MS          (1000)
#define I2C_CLK_SPEED           (400000)

#define IO_COUNT                (16)
/************************* 硬件io *************************/
#define I2C_MASTER_SCL_IO   42          /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO   41          /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM      I2C_NUM_0   /*!< I2C port number for master dev */
#define I2C_ADDRESS         ESP_IO_EXPANDER_I2C_XL9555_ADDRESS_00
/*!< I2C address of slave dev */



static const char *TAG = "xl9555_16bit test";
static esp_io_expander_handle_t io_expander = NULL;
static i2c_master_bus_handle_t i2c_handle = NULL;
//为啥只能使用static？extern会报错undefined reference，感觉是编译链接的问题

/************************* 设备地址配置 *************************/
// XL9555 7位I2C地址（根据硬件A0/A1/A2配置选择）
enum esp_io_expander_xl9555_16bit_address {
    ESP_IO_EXPANDER_I2C_TCA9539_ADDRESS_00 = 0b1110100,
    ESP_IO_EXPANDER_I2C_TCA9539_ADDRESS_01 = 0b1110101,
    ESP_IO_EXPANDER_I2C_TCA9539_ADDRESS_10 = 0b1110110,
    ESP_IO_EXPANDER_I2C_TCA9539_ADDRESS_11 = 0b1110111,
    ESP_IO_EXPANDER_I2C_TCA9555_ADDRESS_000 = 0b0100000,
    ESP_IO_EXPANDER_I2C_TCA9555_ADDRESS_001 = 0b0100001,
    ESP_IO_EXPANDER_I2C_TCA9555_ADDRESS_010 = 0b0100010,
    ESP_IO_EXPANDER_I2C_TCA9555_ADDRESS_011 = 0b0100011,
    ESP_IO_EXPANDER_I2C_TCA9555_ADDRESS_100 = 0b0100000,
    ESP_IO_EXPANDER_I2C_TCA9555_ADDRESS_101 = 0b0100101,
    ESP_IO_EXPANDER_I2C_TCA9555_ADDRESS_110 = 0b0100110,
    ESP_IO_EXPANDER_I2C_TCA9555_ADDRESS_111 = 0b0100111,


    ESP_IO_EXPANDER_I2C_XL9555_ADDRESS_00 = 0x20, // 0b0100000
};

// 默认设备地址（调试时可快速修改）
#define XL9555_DEFAULT_I2C_ADDR    XL9555_I2C_ADDR_000

/************************* 寄存器配置 *************************/
// 寄存器地址


/* Register address */
#define INPUT_REG_ADDR          (0x00)
#define OUTPUT_REG_ADDR         (0x02)
#define DIRECTION_REG_ADDR      (0x06)

/* Default register value on power-up */
#define DIR_REG_DEFAULT_VAL     (0xffff)
#define OUT_REG_DEFAULT_VAL     (0xffff)

/************************* 测试配置（可选） *************************/
// 测试循环次数/延迟
#define TEST_LOOP_CNT       10
#define TEST_LOOP_DELAY_MS  500
#define TEST_OUTPUT_PINS    (IO_EXPANDER_PIN_NUM_3 | IO_EXPANDER_PIN_NUM_1)
#define TEST_INPUT_PINS     (IO_EXPANDER_PIN_NUM_2 | IO_EXPANDER_PIN_NUM_3)

#ifdef __cplusplus
}
#endif