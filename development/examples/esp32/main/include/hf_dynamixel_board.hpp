/**
 * @file hf_dynamixel_board.hpp
 * @brief Target-aware pin / baud aliases for the ESP-IDF examples.
 *
 * @details Kconfig (`menuconfig` → HF Dynamixel Example) is the source of
 *          truth. These fallbacks keep host-side editors happy and match
 *          the documented C6 / S3 bring-up headers.
 */
#pragma once

#include "sdkconfig.h"

#ifndef CONFIG_HF_DYNAMIXEL_UART_PORT
#define CONFIG_HF_DYNAMIXEL_UART_PORT 1
#endif

#ifndef CONFIG_HF_DYNAMIXEL_UART_TX_GPIO
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define CONFIG_HF_DYNAMIXEL_UART_TX_GPIO 17
#else
#define CONFIG_HF_DYNAMIXEL_UART_TX_GPIO 4
#endif
#endif

#ifndef CONFIG_HF_DYNAMIXEL_UART_RX_GPIO
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define CONFIG_HF_DYNAMIXEL_UART_RX_GPIO 18
#else
#define CONFIG_HF_DYNAMIXEL_UART_RX_GPIO 5
#endif
#endif

#ifndef CONFIG_HF_DYNAMIXEL_UART_DIR_GPIO
#define CONFIG_HF_DYNAMIXEL_UART_DIR_GPIO (-1)
#endif

#ifndef CONFIG_HF_DYNAMIXEL_BAUD
#define CONFIG_HF_DYNAMIXEL_BAUD 57600
#endif

#ifndef CONFIG_HF_DYNAMIXEL_SERVO_ID
#define CONFIG_HF_DYNAMIXEL_SERVO_ID 1
#endif

#ifndef CONFIG_HF_DYNAMIXEL_SCAN_LAST_ID
#define CONFIG_HF_DYNAMIXEL_SCAN_LAST_ID 20
#endif
