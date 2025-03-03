#pragma once

extern const struct gpio_dt_spec led;
extern const struct gpio_dt_spec adtrg;

void isr_trigger_detection(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
int gpio_init();
