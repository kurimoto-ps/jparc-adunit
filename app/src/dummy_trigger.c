#include <stm32_ll_gpio.h>
#include <stm32_ll_tim.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#define AUTO_RELOAD_CNT_TIM5 999999
#define PRESCALE_TIM5 95

#define COMPARE_VALUE_TIM5 999999

#define GPIOA_NODE DT_NODELABEL(gpioa)
#define TIM5_NODE DT_NODELABEL(timers5)

void enable_dummy_trigger(){

    TIM_TypeDef *tim5;
	GPIO_TypeDef *gpioa;

	tim5 = (const struct TIM_TypeDef *)DT_REG_ADDR(TIM5_NODE);
	gpioa = (const struct GPIO_TypeDef *)DT_REG_ADDR(GPIOA_NODE);

    // 1 Hz DUMMY TRIGGER (TEST USAGE) 
	LL_GPIO_SetPinMode(gpioa, LL_GPIO_PIN_0, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinOutputType(gpioa, LL_GPIO_PIN_0, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetAFPin_0_7(gpioa, LL_GPIO_PIN_0, LL_GPIO_AF_2);
	LL_TIM_SetAutoReload(tim5, AUTO_RELOAD_CNT_TIM5);
	LL_TIM_SetPrescaler(tim5, PRESCALE_TIM5);
	LL_TIM_CC_EnableChannel(tim5, LL_TIM_CHANNEL_CH1);
	LL_TIM_OC_SetCompareCH1(tim5, COMPARE_VALUE_TIM5);
	LL_TIM_OC_SetMode(tim5, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
	LL_TIM_OC_SetPolarity(tim5, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_LOW);
	LL_TIM_EnableCounter(tim5);

}