/**
 * @file api_tasks.cpp
 * @brief The API guide's task snippets, compiled by the host build (never run).
 */
#include "my_uart.hpp"

#include <cstdio>

using dynamixel::DriverError;

bool open_bus(dynamixel::Bus& bus) {
  // --8<-- [start:open]
  dynamixel::TransportConfig cfg{};
  cfg.baud_rate = 57600;                               // must match the servo's Baud Rate (address 8)
  cfg.echo_policy = dynamixel::EchoPolicy::None;       // ExpectedLocalEcho on a single-wire link
  cfg.latency_ms = 2;                                  // slack added to every reply timeout
  const auto opened = bus.Open(cfg);
  if (!opened) {
    std::printf("open: %s\n", dynamixel::ToString(opened.error).data());
    return false;
  }
  // --8<-- [end:open]
  return true;
}

void scan(dynamixel::Bus& bus) {
  // --8<-- [start:scan]
  uint8_t ids[16];
  const auto found = bus.Scan(ids, 16, 1, 20);          // ping IDs 1..20, keep up to 16 answers
  for (uint8_t i = 0; found && i < found.value; ++i) {
    const auto ping = bus.Ping(ids[i]);
    if (ping) {
      std::printf("id %u model %u fw %u\n", ids[i], ping.value.model_number,
                  ping.value.firmware_version);
    }
  }
  // --8<-- [end:scan]
}

bool identify(dynamixel::Device& servo) {
  // --8<-- [start:identify]
  const auto ping = servo.Identify();                   // ping, catalog lookup, read Operating Mode
  if (ping.error == DriverError::UnsupportedRequest) {
    // The servo answered; ping.value.model_number is not in kKnownModels.
    servo.BindXSeriesFallback();                        // shared X-series addresses
  } else if (!ping) {
    return false;
  }
  std::printf("%s\n", servo.model()->name);
  // --8<-- [end:identify]
  return true;
}

bool move(dynamixel::Device& servo) {
  // --8<-- [start:move]
  // Operating Mode is in EEPROM: torque must be off to change it.
  if (!servo.SetTorqueEnabled(false) ||
      !servo.SetOperatingMode(dynamixel::OperatingMode::Position) ||
      !servo.SetMotionProfile(20, 50)) {                 // Profile Acceleration, Profile Velocity
    return false;
  }

  const auto here = servo.ReadPosition();               // never command a goal you did not read near
  if (!here || !servo.SetTorqueEnabled(true)) {
    return false;
  }
  return servo.SetGoalPosition(here.value + 64).ok();   // 64 ticks = 5.6 degrees
  // --8<-- [end:move]
}

bool spin(dynamixel::Device& servo) {
  // --8<-- [start:velocity]
  if (!servo.SetTorqueEnabled(false) ||
      !servo.SetOperatingMode(dynamixel::OperatingMode::Velocity) ||
      !servo.SetTorqueEnabled(true)) {
    return false;
  }
  return servo.SetGoalVelocity(44).ok();                // 44 x 0.229 rev/min = about 10 rev/min
  // --8<-- [end:velocity]
}

void read_state(dynamixel::Device& servo) {
  // --8<-- [start:read]
  const auto tel = servo.ReadTelemetry();
  if (!tel) {
    return;                                             // the position read failed
  }
  const auto& t = tel.value;
  std::printf("pos %ld vel %ld pwm %d %u.%u V %u C hw 0x%02x\n", static_cast<long>(t.position),
              static_cast<long>(t.velocity), t.pwm, t.voltage_0p1_v / 10U, t.voltage_0p1_v % 10U,
              t.temperature_c, t.hardware_error);
  if (servo.model()->effort_is_current) {
    std::printf("current %d x 2.69 mA\n", t.load);      // XM / XH: Present Current
  } else {
    std::printf("load %d x 0.1 %%\n", t.load);           // XC / XL: Present Load
  }
  // --8<-- [end:read]
}

bool raw_access(dynamixel::Device& servo) {
  // --8<-- [start:raw]
  uint8_t delay = 0;                                    // Return Delay Time, address 9, 2 us per unit
  if (!servo.ReadRegister(9, &delay, 1)) {
    return false;
  }
  const uint8_t shorter = 0;                            // EEPROM: torque must be off
  return servo.WriteRegister(9, &shorter, 1, dynamixel::WritePolicy::ReadbackVerify).ok();
  // --8<-- [end:raw]
}

bool sync(dynamixel::Bus& bus) {
  // --8<-- [start:sync]
  const uint8_t ids[2] = {1, 2};
  uint8_t pos1[4] = {}, pos2[4] = {};
  uint8_t* const out[2] = {pos1, pos2};
  dynamixel::SyncReadItem items[2];
  const auto got = bus.SyncRead(132, 4, ids, 2, out, items);  // Present Position, both servos
  if (!got || got.value != 2) {
    return false;                                       // items[i].valid says which one is missing
  }

  const uint8_t goal1[4] = {0x00, 0x08, 0, 0};          // 2048, little-endian
  const uint8_t goal2[4] = {0x00, 0x04, 0, 0};          // 1024
  const uint8_t* const in[2] = {goal1, goal2};
  return bus.SyncWrite(116, 4, ids, in, 2).ok();        // Goal Position, one packet, no replies
  // --8<-- [end:sync]
}

void errors(dynamixel::Device& servo) {
  // --8<-- [start:errors]
  const auto r = servo.SetGoalPosition(2048);
  switch (r.error) {
    case DriverError::None:
      break;
    case DriverError::Timeout:                          // no status packet: wiring, baud, ID, power
    case DriverError::ChecksumError:                    // corrupted reply: cable, termination, baud
      std::printf("link: %s\n", dynamixel::ToString(r.error).data());
      break;
    case DriverError::UncertainOutcome:                 // write sent, no reply: it may have applied
      std::printf("re-read before retrying\n");
      break;
    case DriverError::DeviceError:                      // the servo answered with an error byte
      std::printf("device error 0x%02x\n", r.detail.device_error);
      break;
    default:                                            // UnsupportedRequest: wrong mode or no model
      std::printf("%s (comm %ld)\n", dynamixel::ToString(r.error).data(),
                  static_cast<long>(r.detail.comm_code));
      break;
  }
  // --8<-- [end:errors]
}

bool change_baud(dynamixel::Bus& bus, dynamixel::Device& servo) {
  // --8<-- [start:baud]
  const uint8_t index_1m = 3;                           // Baud Rate (address 8): 3 = 1 Mbit/s
  const auto wr = servo.WriteRegister(8, &index_1m, 1); // EEPROM: torque must be off
  if (!wr) {
    return false;
  }
  return bus.SetHostBaudRate(1000000).ok();             // follow it on the host side
  // --8<-- [end:baud]
}
