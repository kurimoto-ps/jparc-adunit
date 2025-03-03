# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(adunit)

target_include_directories(app PRIVATE src)
target_sources(app PRIVATE 
        src/adc_averaging_ctx.c
        src/adc_operation.c
        src/dummy_trigger.c
        src/enet_server.c
        src/gpio.c
        src/main.c)
