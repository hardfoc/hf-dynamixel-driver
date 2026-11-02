/**
 * @file dynamixel_device.hpp
 * @brief One servo on a shared `Bus`, with optional model descriptor.
 *
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 */
#pragma once

#include "dynamixel_bus.hpp"
#include "dynamixel_model.hpp"

#include <cstdint>

namespace dynamixel {

class Device {
public:
    Device(Bus& bus, uint8_t id) noexcept;

    uint8_t id() const noexcept { return id_; }
    const ModelDescriptor* model() const noexcept { return model_; }
    bool identified() const noexcept { return model_ != nullptr; }

    /// Cached Operating Mode: `Unknown` before `Identify()` read it, or when
    /// the servo is in a mode without typed support. Every typed goal call
    /// returns `UnsupportedRequest` while the mode is `Unknown` after a read.
    OperatingMode operating_mode() const noexcept {
        return mode_known_ ? cached_mode_ : OperatingMode::Unknown;
    }
    /// Raw Operating Mode register value from the last read or write.
    uint8_t operating_mode_raw() const noexcept { return mode_raw_; }

    DriverResult<PingInfo> Identify() noexcept;

    /// Bind a hand-authored table. Survives `Identify()` when the ping
    /// model is unknown and `desc.model_number` is 0 or matches the ping.
    void BindModel(const ModelDescriptor& desc) noexcept;
    void BindXSeriesFallback() noexcept;

    /// Reboot this servo (`Bus::Reboot`). Clears a latched Hardware Error
    /// Status; RAM, including Torque Enable, returns to its defaults.
    DriverResult<void> Reboot() noexcept;

    DriverResult<void> SetTorqueEnabled(bool enabled) noexcept;
    DriverResult<void> SetOperatingMode(OperatingMode mode) noexcept;
    DriverResult<void> SetMotionProfile(uint32_t profile_acceleration,
                                        uint32_t profile_velocity) noexcept;
    DriverResult<void> SetGoalPosition(int32_t ticks) noexcept;
    DriverResult<void> SetGoalVelocity(int32_t ticks_per_sec) noexcept;
    DriverResult<void> SetGoalPwm(int16_t pwm) noexcept;

    DriverResult<int32_t> ReadPosition() noexcept;
    /// Present PWM, load/current, velocity, position, input voltage and
    /// temperature in one Read of the contiguous block 124..146, then Hardware
    /// Error Status (70). Falls back to one read per register when the block
    /// does not fit `max_packet_bytes` or the servo refuses the range.
    DriverResult<Telemetry> ReadTelemetry() noexcept;

    DriverResult<uint16_t> ReadRegister(uint16_t address, uint8_t* data,
                                        uint16_t length) noexcept;
    DriverResult<void> WriteRegister(uint16_t address, const uint8_t* data,
                                     uint16_t length,
                                     WritePolicy policy = WritePolicy::TxRx) noexcept;

private:
    DriverResult<void> RequireModel() const noexcept;
    DriverResult<void> RequireMode(OperatingMode mode) const noexcept;
    DriverResult<int32_t> ReadSigned(const RegisterDesc& reg) noexcept;
    DriverResult<Telemetry> ReadTelemetryPerRegister() noexcept;
    DriverResult<void> WriteSigned(const RegisterDesc& reg, int32_t value,
                                   WritePolicy policy) noexcept;

    Bus& bus_;
    uint8_t id_;
    const ModelDescriptor* model_{nullptr};
    OperatingMode cached_mode_{OperatingMode::Position};
    bool mode_known_{false};
    uint8_t mode_raw_{0};
};

}  // namespace dynamixel
