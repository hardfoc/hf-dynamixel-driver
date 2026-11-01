/**
 * @file hf_dynamixel_esp_uart.hpp
 * @brief Native ESP-IDF UART adapter for dynamixel::Transport.
 *
 * @details Half-duplex turnaround is expressed as finish_transmit()
 *          (uart_wait_tx_done) plus an optional direction GPIO.
 *          Electrical mode stays here; Device never sees a DIR pin.
 */
#pragma once

#include "dynamixel.hpp"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstddef>
#include <cstdint>

namespace hf_dynamixel_examples {

/**
 * @tparam kPort UART port.
 * @tparam kTxGpio MCU TX (to the servo data line or buffer DI).
 * @tparam kRxGpio MCU RX.
 * @tparam kDirGpio Optional TX-enable GPIO; GPIO_NUM_NC to skip.
 * @tparam kDirActiveHigh True if the buffer transmits when DIR is high.
 */
template <uart_port_t kPort, gpio_num_t kTxGpio, gpio_num_t kRxGpio,
          gpio_num_t kDirGpio = GPIO_NUM_NC, bool kDirActiveHigh = true,
          std::uint32_t kBaud = dynamixel::kDefaultBaud>
class DynamixelEspIdfUart
    : public dynamixel::Transport<
          DynamixelEspIdfUart<kPort, kTxGpio, kRxGpio, kDirGpio, kDirActiveHigh, kBaud>> {
public:
    static esp_err_t Install() noexcept {
        uart_config_t uart_cfg = {};
        uart_cfg.baud_rate = static_cast<int>(kBaud);
        uart_cfg.data_bits = UART_DATA_8_BITS;
        uart_cfg.parity = UART_PARITY_DISABLE;
        uart_cfg.stop_bits = UART_STOP_BITS_1;
        uart_cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        uart_cfg.source_clk = UART_SCLK_DEFAULT;
        esp_err_t err = uart_driver_install(kPort, 2048, 0, 0, nullptr, 0);
        if (err != ESP_OK) {
            return err;
        }
        err = uart_param_config(kPort, &uart_cfg);
        if (err != ESP_OK) {
            return err;
        }
        err = uart_set_pin(kPort, kTxGpio, kRxGpio, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        if (err != ESP_OK) {
            return err;
        }
        if (kDirGpio != GPIO_NUM_NC) {
            gpio_config_t io = {};
            io.pin_bit_mask = 1ULL << static_cast<int>(kDirGpio);
            io.mode = GPIO_MODE_OUTPUT;
            io.pull_up_en = GPIO_PULLUP_DISABLE;
            io.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io.intr_type = GPIO_INTR_DISABLE;
            err = gpio_config(&io);
            if (err != ESP_OK) {
                return err;
            }
            SetDir(false);
        }
        return ESP_OK;
    }

    dynamixel::DriverError open(const dynamixel::TransportConfig& cfg) noexcept {
        if (uart_set_baudrate(kPort, cfg.baud_rate) != ESP_OK) {
            return dynamixel::DriverError::SerialError;
        }
        return dynamixel::DriverError::None;
    }

    void close() noexcept {}

    dynamixel::DriverError set_baud_rate(std::uint32_t baud) noexcept {
        return (uart_set_baudrate(kPort, baud) == ESP_OK)
                   ? dynamixel::DriverError::None
                   : dynamixel::DriverError::SerialError;
    }

    dynamixel::TransportIo write_some(const std::uint8_t* data, std::size_t length) noexcept {
        SetDir(true);
        const int n = uart_write_bytes(kPort, data, length);
        if (n < 0) {
            return {0, dynamixel::DriverError::SerialError};
        }
        return {static_cast<std::size_t>(n), dynamixel::DriverError::None};
    }

    dynamixel::DriverError finish_transmit() noexcept {
        const esp_err_t err = uart_wait_tx_done(kPort, pdMS_TO_TICKS(20));
        SetDir(false);
        return (err == ESP_OK) ? dynamixel::DriverError::None
                               : dynamixel::DriverError::SerialError;
    }

    dynamixel::TransportIo read_some(std::uint8_t* out, std::size_t max) noexcept {
        const int n = uart_read_bytes(kPort, out, static_cast<int>(max), 0);
        if (n < 0) {
            return {0, dynamixel::DriverError::SerialError};
        }
        return {static_cast<std::size_t>(n), dynamixel::DriverError::None};
    }

    void discard_stale_input() noexcept { uart_flush_input(kPort); }

    std::uint64_t now_us() noexcept {
        return static_cast<std::uint64_t>(esp_timer_get_time());
    }

    void delay_ms_impl(std::uint32_t ms) noexcept { vTaskDelay(pdMS_TO_TICKS(ms)); }
    void yield_impl() noexcept { taskYIELD(); }

private:
    static void SetDir(bool transmit) noexcept {
        if (kDirGpio == GPIO_NUM_NC) {
            return;
        }
        const int level = (transmit == kDirActiveHigh) ? 1 : 0;
        gpio_set_level(kDirGpio, level);
    }
};

}  // namespace hf_dynamixel_examples
