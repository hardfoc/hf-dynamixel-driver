/**
 * @file configuration.cpp
 * @brief The Configuration page's snippets, compiled by the host build (never run).
 */
#include "my_uart.hpp"

// --8<-- [start:echo]
dynamixel::TransportConfig single_wire_config() {
  dynamixel::TransportConfig cfg{};
  cfg.baud_rate = 1000000;
  cfg.echo_policy = dynamixel::EchoPolicy::ExpectedLocalEcho;  // RX hears every TX byte first
  cfg.latency_ms = 1;
  return cfg;
}
// --8<-- [end:echo]

// --8<-- [start:model]
// An X-series servo the catalog does not list yet: same control table, its own model number.
inline constexpr dynamixel::ModelDescriptor kMyXm540 =
    dynamixel::MakeXSeries(1120, "XM540-W270", /*supports_current=*/true,
                           /*effort_is_current=*/true);

void bind_my_servo(dynamixel::Device& servo) {
  servo.BindModel(kMyXm540);   // kept by Identify() when the ping reports model 1120
  (void)servo.Identify();
}
// --8<-- [end:model]
