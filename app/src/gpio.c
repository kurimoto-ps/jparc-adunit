#include <stm32_ll_gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "adc_stm32_custom.h"
#include "adc_averaging_ctx.h"

#define LED0_NODE DT_ALIAS(led0)

const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
const struct gpio_dt_spec adtrg = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), adtrig_gpios);
static struct gpio_callback adtrg_cb_data;

void isr_trigger_detection(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	if(avctx.start_flag == false)
		avctx.start_flag = true;
	else 
		avctx.remaining_count = avctx.remaining_count - 1;
	//printk("remaining_cnt = %d, current_idx = %d\n", avctx.remaining_count, avctx.current_idx);
	avctx.current_idx = 0;
}

int gpio_init(){
    int err;
    
    if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (err < 0) {
		return 0;
	}
	
    gpio_pin_set_dt(&led, 0); 
    
    if (!gpio_is_ready_dt(&adtrg)) {
		return 0;
	}

    err = gpio_pin_configure_dt(&adtrg, GPIO_INPUT);
	if (err < 0) {
		return 0;
	}
	
	err = gpio_pin_interrupt_configure_dt(&adtrg,
					      GPIO_INT_DISABLE);
	if (err != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			err, adtrg.port->name, adtrg.pin);
		return 0;
	}
	
    gpio_init_callback(&adtrg_cb_data, isr_trigger_detection, BIT(adtrg.pin));
	gpio_add_callback(adtrg.port, &adtrg_cb_data);
    
	return 0;
}
