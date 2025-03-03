#include <zephyr/kernel.h>
#include "adc_custom.h"
#include "adc_stm32_custom.h"

#define BUFLEN 57600
#define NUM_AVERAGES_DEFAULT 1

/* Global array declaration */
uint16_t buf[BUFLEN];
uint32_t buf_sum[BUFLEN];

struct adc_averaging_ctx avctx = {
	.buf_sum = buf_sum,
	.current_idx = 0,
	.num_averages = NUM_AVERAGES_DEFAULT,
	.remaining_count = NUM_AVERAGES_DEFAULT,
	.buf_size = sizeof(buf_sum),
	.start_flag = false,
};

struct adc_sequence_options options = {
	.extra_samplings = BUFLEN*NUM_AVERAGES_DEFAULT-1,
	.interval_us = 0U,
	.user_data = &avctx,	
};

struct adc_sequence sequence = {
	.options = &options,
	.buffer = buf,
	/* buffer size in bytes, not number of samples */
	.buffer_size = sizeof(buf),
};

void set_num_averages(uint16_t num_averages){
	avctx.num_averages = num_averages;
	avctx.remaining_count = num_averages;
	options.extra_samplings = BUFLEN*num_averages - 1;
}