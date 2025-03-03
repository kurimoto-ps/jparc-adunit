# 03.02.2025 Yoshinori Kurimoto
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_ADC_STM32_CUSTOM		adc_stm32_custom.c)

zephyr_include_directories(.)

