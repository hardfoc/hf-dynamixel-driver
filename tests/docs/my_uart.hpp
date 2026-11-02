/**
 * @file my_uart.hpp
 * @brief The UART transport shown on the documentation's Porting page, compiled by the host build.
 *
 * Every platform call goes through the `platform_*` functions declared below; a real transport
 * replaces them with its SDK (ESP-IDF, STM32 HAL, Zephyr, Linux termios). The class is what the
 * driver needs, nothing more.
 */
#pragma once

#include "dynamixel.hpp"

#include <cstddef>
#include <cstdint>

// Platform hooks: your SDK's calls. Declared only, so the docs build checks the transport compiles.
bool platform_uart_set_baud(uint32_t baud) noexcept;
int platform_uart_write(const uint8_t* data, std::size_t len) noexcept;  // bytes queued, < 0 on error
bool platform_uart_wait_tx_done() noexcept;                              // last stop bit sent
int platform_uart_read(uint8_t* out, std::size_t max) noexcept;          // bytes available now, never blocks
void platform_uart_flush_rx() noexcept;
void platform_dir_set(bool transmit) noexcept;                           // half-duplex buffer direction
uint64_t platform_micros() noexcept;
void platform_delay_ms(uint32_t ms) noexcept;

// --8<-- [start:port]
class MyUart : public dynamixel::Transport<MyUart> {
public:
  // Configure the UART: 8 data bits, no parity, 1 stop bit at cfg.baud_rate. No servo traffic.
  dynamixel::DriverError open(const dynamixel::TransportConfig& cfg) noexcept {
    platform_dir_set(false);  // listen
    return platform_uart_set_baud(cfg.baud_rate) ? dynamixel::DriverError::None
                                                 : dynamixel::DriverError::SerialError;
  }
  void close() noexcept {}

  dynamixel::DriverError set_baud_rate(uint32_t baud) noexcept {
    return platform_uart_set_baud(baud) ? dynamixel::DriverError::None
                                        : dynamixel::DriverError::SerialError;
  }

  // Queue bytes for transmission; report how many were accepted (a partial write is fine).
  dynamixel::TransportIo write_some(const uint8_t* data, std::size_t length) noexcept {
    platform_dir_set(true);  // drive the bus
    const int n = platform_uart_write(data, length);
    if (n < 0) {
      return {0, dynamixel::DriverError::SerialError};
    }
    return {static_cast<std::size_t>(n), dynamixel::DriverError::None};
  }

  // Block until the last stop bit has left the pin, then release the bus for the reply.
  dynamixel::DriverError finish_transmit() noexcept {
    const bool done = platform_uart_wait_tx_done();
    platform_dir_set(false);
    return done ? dynamixel::DriverError::None : dynamixel::DriverError::SerialError;
  }

  // Return what has arrived so far; 0 bytes is not an error.
  dynamixel::TransportIo read_some(uint8_t* out, std::size_t max) noexcept {
    const int n = platform_uart_read(out, max);
    if (n < 0) {
      return {0, dynamixel::DriverError::SerialError};
    }
    return {static_cast<std::size_t>(n), dynamixel::DriverError::None};
  }

  void discard_stale_input() noexcept { platform_uart_flush_rx(); }
  uint64_t now_us() noexcept { return platform_micros(); }

  // Optional: lets the driver sleep instead of spinning while it waits.
  void delay_ms_impl(uint32_t ms) noexcept { platform_delay_ms(ms); }
};
// --8<-- [end:port]
