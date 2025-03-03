#pragma once

extern struct adc_averaging_ctx avctx;
extern struct adc_sequence_options options;
extern struct adc_sequence sequence;

void set_num_averages(uint16_t num_averages);