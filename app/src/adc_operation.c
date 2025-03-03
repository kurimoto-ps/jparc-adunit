#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <stm32_ll_adc.h>
#include <stm32_ll_gpio.h>
#include <stm32_ll_tim.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "adc_custom.h"
#include "adc_stm32_custom.h"
#include "adc_averaging_ctx.h"
#include "gpio.h"
#include "dummy_trigger.h"

#define AUTO_RELOAD_CNT_TIM2 99
#define PRESCALE_TIM2 99

/* define events */
K_EVENT_DEFINE(adc_events);

/* The devicetree node identifier for the "led0" alias. */
#define ADC_NODE DT_NODELABEL(adc1)
#define TIM2_NODE DT_NODELABEL(timers2)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

void initialize_peripherals(void){
	
	int err;
	ADC_TypeDef *adc;
	TIM_TypeDef *tim2;

	adc = (const struct ADC_TypeDef *)DT_REG_ADDR(ADC_NODE);
	tim2 = (const struct TIM_TypeDef *)DT_REG_ADDR(TIM2_NODE);
	
	// ADC Setting. Conversion Trigger from Timer2
	LL_ADC_REG_SetTriggerSource(adc, LL_ADC_REG_TRIG_EXT_TIM2_TRGO);
	LL_ADC_REG_SetTriggerEdge(adc, LL_ADC_REG_TRIG_EXT_RISING);

  	
	// Timer2 Setting. Conversion Frequency 9.6 kHz
	LL_TIM_SetAutoReload(tim2, AUTO_RELOAD_CNT_TIM2);
	LL_TIM_SetPrescaler(tim2, PRESCALE_TIM2);
	LL_TIM_SetTriggerOutput(tim2, LL_TIM_TRGO_UPDATE);
	LL_TIM_EnableCounter(tim2);
	
	enable_dummy_trigger();

	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!device_is_ready(adc_channels[i].dev)) {
			printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
			return 0;
		}
		err = adc_stm32_channel_setup(adc_channels[i].dev, &(adc_channels[i].channel_cfg));
		if (err < 0) {
			printk("Could not setup channel #%d (%d)\n", i, err);
			return 0;
		}
	}
	SCB_InvalidateDCache();
	gpio_init();
}

void adc_operation(void)
{
	
	int err;
	initialize_peripherals();
	
	while(1){
		k_event_wait(&adc_events, BIT(0), true, K_FOREVER);
		gpio_pin_set_dt(&led, 1);
		printk("Start ADC reading\n");
		gpio_pin_interrupt_configure_dt(&adtrg, GPIO_INT_EDGE_TO_ACTIVE);
		for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
			int32_t val_mv;
			(void)adc_sequence_init_dt(&adc_channels[i], &sequence);
			err = adc_stm32_read(adc_channels[i].dev, &sequence);
			if (err < 0) {
				printk("Could not read (%d)\n", err);
				continue;
			}
		}
		gpio_pin_interrupt_configure_dt(&adtrg, GPIO_INT_DISABLE);
		avctx.start_flag = false;
		gpio_pin_set_dt(&led, 0);
	}

}
