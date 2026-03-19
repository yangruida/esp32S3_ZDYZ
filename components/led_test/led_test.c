#include "led_indicator_gpio.h"
/*
*led_init
*/

#define Led_indicator_GPIO 1 // led的gpio口
led_indicator_handle_t led_handle = NULL;
    const blink_step_t  pattern_1[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 1000},               // step1: turn on LED 50 ms
    {LED_BLINK_HOLD, LED_STATE_OFF, 1000},             // step2: turn off LED 100 ms
    {LED_BLINK_LOOP, 0, 0},                           // step3: loop from step1
    };
    static const blink_step_t *blink_lists[] = {
    pattern_1,      // 模式0
    // pattern_2,   // 模式1 (如果需要)
    };
    led_indicator_config_t led_cfg = {
    .blink_lists = blink_lists,   // 传入模式列表
    .blink_list_num = 1,           // 有1个模式

    };
void led_init(void) {
    led_indicator_gpio_config_t gpio_cfg = {
        .is_active_level_high = true,
        .gpio_num = Led_indicator_GPIO,
    };



    esp_err_t ret = led_indicator_new_gpio_device(&led_cfg, &gpio_cfg,
                                                    &led_handle);
    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "Failed to initialize LED indicator");
}
    led_init();
    led_indicator_start(led_handle, 0);
while (1)
{
    // ret = esp_io_expander_get_level(io_expander, IO_EXPANDER_PIN_NUM_15|IO_EXPANDER_PIN_NUM_14, &input_level_mask);
    // TEST_ASSERT_EQUAL(ESP_OK, ret);
    // ESP_LOGI(TAG, "Input level mask: 0x%02" PRIX32, input_level_mask);

    // led_indicator_start(led_handle, 0);
    vTaskDelay(10000);
    // led_indicator_stop(led_handle, 0);

}