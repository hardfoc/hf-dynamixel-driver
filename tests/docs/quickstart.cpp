/**
 * @file quickstart.cpp
 * @brief The Quickstart page's example, compiled by the host build (never run).
 */
#include "my_uart.hpp"

#include <cstdio>

// --8<-- [start:quickstart]
#include "dynamixel.hpp"

int read_servo_1(MyUart& uart) {
  dynamixel::Bus bus{dynamixel::TransportView{uart}};      // braces: see "most vexing parse"

  dynamixel::TransportConfig cfg{};                         // 1. 57600 bit/s, 8N1: factory default
  if (!bus.Open(cfg)) {
    return -1;                                              // your transport's open() failed
  }

  dynamixel::Device servo(bus, 1);                          // 2. factory ID is 1
  const auto ping = servo.Identify();                       //    ping + catalog lookup
  if (ping.error == dynamixel::DriverError::UnsupportedRequest) {
    servo.BindXSeriesFallback();                            //    answered, model not in the catalog
  } else if (!ping) {
    return -2;                                              // Timeout: wiring, power, baud, ID
  }

  const auto tel = servo.ReadTelemetry();                   // 3. torque stays off
  if (!tel) {
    return -3;
  }
  std::printf("position %ld ticks, %u.%u V\n", static_cast<long>(tel.value.position),
              tel.value.voltage_0p1_v / 10U, tel.value.voltage_0p1_v % 10U);
  return 0;
}
// --8<-- [end:quickstart]
