# Software for Magnet Current Measurement Board for J-PARC MR Magnet Power Supplies

## Overview

The project is a Zephyr-based application designed to interface with ADC (Analog-to-Digital Converter) hardware on STM32 microcontrollers 
mounted on [nucleo-h753zi](https://www.st.com/ja/evaluation-tools/nucleo-h753zi.html) board.
The project includes custom drivers and configurations to support ADC operations, GPIO interactions, and network server functionalities.

## Requirements
- Zephyr 3.7.0
- Zephyr SDK 0.17.0

You can also use a [docker image](kurimoto422/jparc-adunit:rtos-dev-v0.0) to obtain the environment. 

## How to Setup Build Environment 

```bash
mkdir workspace
cd workspace
git clone https://github.com/kurimoto-ps/jparc-adunit
pip install west
west init -l jparc-adunit
west update
pip install -r zephyr/scripts/requirements-base.txt
```
## How to Build the Application

```bash
cd workspace/jparc-adunit/app
rm -rf build && west build -b nucleo-h753zi
```

## How to Modify Network Configuration
Please modify [workspace/jparc-adunit/app/prj.conf](app/prj.conf) to configure IP address and so on. 

```
workspace/jparc-adunit/app/prj.conf:
CONFIG_NET_CONFIG_MY_IPV4_ADDR="192.168.1.159"
CONFIG_NET_CONFIG_MY_IPV4_NETMASK="255.255.255.000"
CONFIG_NET_CONFIG_MY_IPV4_GW="192.168.001.001"
```
## Directory Structure

- **app/**: Contains the main application source code and headers.
  - **src/**: Source files for ADC operations, GPIO handling, and main application logic.
  - **CMakeLists.txt**: CMake configuration for building the application.
- **zephyr/**: Contains Zephyr-specific configurations and custom drivers.
  - **drivers/adc/**: Custom ADC driver implementations and configurations.
  - **CMakeLists.txt**: CMake configuration for Zephyr components.
  - **Kconfig**: Configuration options for the Zephyr build system.
- **LICENSE**: Apache License 2.0 under which the project is distributed.

## Key Components

### ADC Operations

The project includes custom ADC operations tailored for STM32 microcontrollers. The ADC operations are defined in the following files:

- `adc_operation.c`: Implements the main ADC operation logic, including initialization and reading sequences.
- `adc_averaging_ctx.c`: Manages ADC averaging context and sequence options.

### GPIO Handling

GPIO operations are crucial for triggering ADC conversions and indicating status through LEDs. The GPIO handling is implemented in:

- `gpio.c`: Configures GPIO pins and handles interrupts for ADC triggers.

### Network Server

The project includes a basic network server setup to handle Ethernet communications, defined in:

- `enet_server.c`: Implements the network server logic.

