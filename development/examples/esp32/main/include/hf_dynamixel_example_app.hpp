/**
 * @file hf_dynamixel_example_app.hpp
 * @brief Shared ESP32 example wiring: UART type, bus open, identify/bind.
 */
#pragma once

#include "dynamixel.hpp"
#include "hf_dynamixel_board.hpp"
#include "hf_dynamixel_esp_uart.hpp"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

namespace hf_dynamixel_examples {

inline constexpr gpio_num_t kExampleDirGpio =
    (CONFIG_HF_DYNAMIXEL_UART_DIR_GPIO < 0)
        ? GPIO_NUM_NC
        : static_cast<gpio_num_t>(CONFIG_HF_DYNAMIXEL_UART_DIR_GPIO);

#if defined(CONFIG_HF_DYNAMIXEL_DIR_ACTIVE_HIGH)
inline constexpr bool kExampleDirActiveHigh = CONFIG_HF_DYNAMIXEL_DIR_ACTIVE_HIGH;
#else
inline constexpr bool kExampleDirActiveHigh = true;
#endif

using ExampleUart = DynamixelEspIdfUart<
    static_cast<uart_port_t>(CONFIG_HF_DYNAMIXEL_UART_PORT),
    static_cast<gpio_num_t>(CONFIG_HF_DYNAMIXEL_UART_TX_GPIO),
    static_cast<gpio_num_t>(CONFIG_HF_DYNAMIXEL_UART_RX_GPIO),
    kExampleDirGpio, kExampleDirActiveHigh,
    static_cast<std::uint32_t>(CONFIG_HF_DYNAMIXEL_BAUD)>;

inline dynamixel::TransportConfig ExampleTransportConfig() noexcept {
    dynamixel::TransportConfig cfg;
    cfg.baud_rate = static_cast<std::uint32_t>(CONFIG_HF_DYNAMIXEL_BAUD);
    cfg.latency_ms = 2;
#if defined(CONFIG_HF_DYNAMIXEL_EXPECT_LOCAL_ECHO) && CONFIG_HF_DYNAMIXEL_EXPECT_LOCAL_ECHO
    cfg.echo_policy = dynamixel::EchoPolicy::ExpectedLocalEcho;
#else
    cfg.echo_policy = dynamixel::EchoPolicy::None;
#endif
    return cfg;
}

inline bool OpenExampleBus(dynamixel::Bus& bus, const char* tag) noexcept {
    const auto opened = bus.Open(ExampleTransportConfig());
    if (!opened.ok()) {
        ESP_LOGE(tag, "bus open failed: %s comm=%d",
                 dynamixel::ToString(opened.error).data(), opened.detail.comm_code);
        return false;
    }
    ESP_LOGI(tag, "uart%u tx=%d rx=%d dir=%d baud=%u echo=%s",
             static_cast<unsigned>(CONFIG_HF_DYNAMIXEL_UART_PORT),
             CONFIG_HF_DYNAMIXEL_UART_TX_GPIO, CONFIG_HF_DYNAMIXEL_UART_RX_GPIO,
             CONFIG_HF_DYNAMIXEL_UART_DIR_GPIO, CONFIG_HF_DYNAMIXEL_BAUD,
             (ExampleTransportConfig().echo_policy == dynamixel::EchoPolicy::ExpectedLocalEcho)
                 ? "local"
                 : "none");
    return true;
}

/// Ping + Identify. Unknown models keep ping data; optionally bind the
/// shared X-series table so XC430 and a later larger servo share the
/// same typed position path.
inline dynamixel::DriverResult<dynamixel::PingInfo> IdentifyOrBind(
    dynamixel::Device& dev, const char* tag) noexcept {
    auto ping = dev.Identify();
    if (ping.ok()) {
        ESP_LOGI(tag, "id=%u model=%u (%s) fw=%u", ping.value.id,
                 ping.value.model_number,
                 (dev.model() != nullptr) ? dev.model()->name : "?",
                 ping.value.firmware_version);
        return ping;
    }
    if (ping.error == dynamixel::DriverError::UnsupportedRequest &&
        ping.value.model_number != 0) {
        ESP_LOGW(tag, "id=%u model=%u is not in the catalog", ping.value.id,
                 ping.value.model_number);
#if defined(CONFIG_HF_DYNAMIXEL_BIND_XSERIES_FALLBACK) && \
    CONFIG_HF_DYNAMIXEL_BIND_XSERIES_FALLBACK
        dev.BindXSeriesFallback();
        ESP_LOGW(tag, "bound X-series fallback (shared control table)");
        ping.error = dynamixel::DriverError::None;
        return ping;
#endif
    }
    ESP_LOGE(tag, "identify failed: %s comm=%d",
             dynamixel::ToString(ping.error).data(), ping.detail.comm_code);
    return ping;
}

}  // namespace hf_dynamixel_examples
