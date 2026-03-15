/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"
#include "unity_test_runner.h"
#include "unity_test_utils_memory.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "xl9555.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"  // ST7789 的具体驱动头文件
#include "xl9555.h"

#include "ui.h"
#include "esp_lvgl_port.h"//连接硬件时
#include "lvgl.h"//创建UI时config中使用了swap来校准自序即色彩

//static const char *TAG = "LCD";

// ===== 根据你的硬件修改这里 =====
#define LCD_HOST    SPI2_HOST
#define PIN_NUM_MOSI 11      // 数据线
#define PIN_NUM_SCLK 12      // 时钟线
#define PIN_NUM_CS   21       // 片选（没有就设-1）
#define PIN_NUM_DC   40       // 数据/命令
#define PIN_NUM_RST  4       // 复位  扩展io
#define PIN_NUM_BCKL 15      // 背光 扩展io

#define LCD_H_RES    240     // 宽度
#define LCD_V_RES    320     // 高度
#define LCD_BITS_PER_PIXEL 16 // RGB565
// ===============================

 void i2c_bus_init(void)
{
    const i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_config, &i2c_handle);
    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "I2C install returned error");
}

 void i2c_bus_deinit(void)
{
    esp_err_t ret = i2c_del_master_bus(i2c_handle);
    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "I2C uninstall returned error");
}

 void i2c_dev_xl9555_16bit_init(void)
{
    esp_err_t ret = esp_io_expander_new_i2c_xl9555_16bit(i2c_handle, I2C_ADDRESS, &io_expander);
    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "TCA9554 create returned error");
}

 void i2c_dev_xl9555_16bit_deinit(void)
{
    esp_err_t ret = esp_io_expander_del(io_expander);
    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "TCA9554 delete returned error");
}


static lv_disp_t *disp_handle = NULL;
static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, 
                                    esp_lcd_panel_io_event_data_t *edata, 
                                    void *user_ctx) {
    lvgl_port_flush_ready(disp_handle);  // 直接使用全局 disp_handle
    return false;
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting ST7789 example...");
    // 初始化 I2C 总线和 IO 扩展器
    i2c_bus_init();
    i2c_dev_xl9555_16bit_init();
    esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_11|IO_EXPANDER_PIN_NUM_10, IO_EXPANDER_OUTPUT);

    // 1. 初始化 SPI 总线
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_SCLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));
    
    // 2. 创建 SPI 接口 IO 设备
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = 40 * 1000 * 1000, // 40MHz
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,                // ST7789 通常用 Mode 0 或 3
        .trans_queue_depth = 10,
            // 👇 关键：添加回调函数
        .on_color_trans_done = notify_lvgl_flush_ready,
        .user_ctx = &disp_handle,  // 传递 disp_handle 的指针
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));
    
    // 3. 创建 ST7789 面板
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1, // 复位由扩展IO控制
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, // 颜色顺序
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    //将复位引脚连接到io扩展器的P11，并在这里通过io扩展器控制复位信号，确保屏幕正确复位
    ESP_LOGI("LCD", "Resetting ST7789 via XL9555 P11...");
    esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_11, 0);
    vTaskDelay(pdMS_TO_TICKS(10));                 // 保持至少 10ms
    esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_11, 1);
    vTaskDelay(pdMS_TO_TICKS(120));                // 等待屏幕稳定

    // 4. 初始化面板
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true)); // 很多ST7789需要颜色反转
    
    // 6. 打开显示和背光
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    
    // 7. 点亮背光（简单粗暴：直接拉高）（现在用的是io扩展器控制的背光）
    ESP_LOGI(TAG, "open the backlight via XL9555 P10...");
    esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_11, 1);

// === 2. LVGL 初始化（v1.x API） ===
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = tskIDLE_PRIORITY + 2,
        .task_stack = 8192,
        .task_affinity = -1,
        .timer_period_ms = 5,
    };
    lvgl_port_init(&lvgl_cfg);
    
    // 添加显示设备
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = LCD_H_RES * 40,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,  // 使用 DMA
        }
    };
    
    disp_handle = lvgl_port_add_disp(&disp_cfg);
    
    
    // === 4. 加载UI ===
    ui_init();
    
    ESP_LOGI(TAG, "UI loaded successfully!");
    
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}