/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>

#include "adc_operation.h"
#include "enet_server.h"

#define ADC_PRIORITY -7
#define ENET_PRIORITY 7
#define STACKSIZE 1024

K_THREAD_DEFINE(adc_id, STACKSIZE, adc_operation, NULL, NULL, NULL,
		ADC_PRIORITY, 0, 0);
K_THREAD_DEFINE(enet_id, STACKSIZE, enet_server, NULL, NULL, NULL,
		ENET_PRIORITY, 0, 0);
