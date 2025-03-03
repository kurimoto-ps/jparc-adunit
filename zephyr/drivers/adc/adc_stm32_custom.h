#pragma once

#include <zephyr/kernel.h>
#include "adc_custom.h"

struct adc_averaging_ctx {
	uint32_t* buf_sum;
	uint16_t current_idx;
	uint16_t num_averages;
	uint16_t remaining_count;
	uint32_t buf_size;
	bool start_flag;
};

int adc_stm32_read(const struct device *dev, const struct adc_sequence *sequence);
int adc_stm32_channel_setup(const struct device *dev, const struct adc_channel_cfg *channel_cfg);
#ifdef CONFIG_ADC_ASYNC
int adc_stm32_read_async(const struct device *dev, const struct adc_sequence *sequence, struct k_poll_signal *async);
#endif