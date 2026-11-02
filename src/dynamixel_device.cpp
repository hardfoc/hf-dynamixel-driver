#include "dynamixel_device.hpp"

namespace dynamixel {

Device::Device(Bus& bus, uint8_t id) noexcept : bus_(bus), id_(id) {}

void Device::BindModel(const ModelDescriptor& desc) noexcept {
    model_ = &desc;
}

void Device::BindXSeriesFallback() noexcept {
    model_ = &kXSeriesFallback;
}

DriverResult<void> Device::RequireModel() const noexcept {
    if (model_ == nullptr) {
        return DriverResult<void>::failure(DriverError::UnsupportedRequest);
    }
    return DriverResult<void>::success();
}

DriverResult<void> Device::RequireMode(OperatingMode mode) const noexcept {
    const auto m = RequireModel();
    if (!m.ok()) {
        return m;
    }
    if (!ModeSupported(*model_, mode)) {
        return DriverResult<void>::failure(DriverError::UnsupportedRequest);
    }
    if (mode_known_ && cached_mode_ != mode) {
        return DriverResult<void>::failure(DriverError::UnsupportedRequest);
    }
    return DriverResult<void>::success();
}

DriverResult<PingInfo> Device::Identify() noexcept {
    auto ping = bus_.Ping(id_);
    if (!ping.ok()) {
        model_ = nullptr;
        return ping;
    }
    const ModelDescriptor* found = FindModel(ping.value.model_number);
    if (found != nullptr) {
        model_ = found;
    } else if (model_ != nullptr &&
               (model_->model_number == 0 ||
                model_->model_number == ping.value.model_number)) {
        // Keep BindModel / BindXSeriesFallback for an unlisted servo.
    } else {
        model_ = nullptr;
        ping.error = DriverError::UnsupportedRequest;
        return ping;
    }
    uint8_t mode = 0;
    const auto rd = bus_.ReadRegister(id_, model_->operating_mode_reg.address, &mode, 1);
    if (rd.ok()) {
        mode_raw_ = mode;
        cached_mode_ = OperatingModeFromRaw(mode);
        mode_known_ = true;
    }
    return ping;
}

DriverResult<int32_t> Device::ReadSigned(const RegisterDesc& reg) noexcept {
    uint8_t raw[4] = {0, 0, 0, 0};
    const auto rd = bus_.ReadRegister(id_, reg.address, raw, reg.width);
    if (!rd.ok()) {
        return DriverResult<int32_t>::failure(rd.error, rd.detail);
    }
    uint32_t u = 0;
    for (uint8_t i = 0; i < reg.width; ++i) {
        u |= static_cast<uint32_t>(raw[i]) << (8 * i);
    }
    int32_t value = static_cast<int32_t>(u);
    if (reg.is_signed && reg.width < 4) {
        const uint32_t sign = 1u << (8 * reg.width - 1);
        if (u & sign) {
            value = static_cast<int32_t>(u | ~((1u << (8 * reg.width)) - 1u));
        }
    }
    return DriverResult<int32_t>::success(value, rd.detail);
}

DriverResult<void> Device::WriteSigned(const RegisterDesc& reg, int32_t value,
                                       WritePolicy policy) noexcept {
    uint8_t raw[4] = {0, 0, 0, 0};
    const uint32_t u = static_cast<uint32_t>(value);
    for (uint8_t i = 0; i < reg.width; ++i) {
        raw[i] = static_cast<uint8_t>((u >> (8 * i)) & 0xFF);
    }
    return bus_.WriteRegister(id_, reg.address, raw, reg.width, policy);
}

DriverResult<void> Device::Reboot() noexcept {
    return bus_.Reboot(id_);
}

DriverResult<void> Device::SetTorqueEnabled(bool enabled) noexcept {
    const auto m = RequireModel();
    if (!m.ok()) {
        return m;
    }
    const uint8_t v = enabled ? 1 : 0;
    return bus_.WriteRegister(id_, model_->torque_enable_reg.address, &v, 1);
}

DriverResult<void> Device::SetOperatingMode(OperatingMode mode) noexcept {
    const auto m = RequireModel();
    if (!m.ok()) {
        return m;
    }
    if (mode == OperatingMode::Unknown) {
        return DriverResult<void>::failure(DriverError::InvalidParameter);
    }
    if (!ModeSupported(*model_, mode)) {
        return DriverResult<void>::failure(DriverError::UnsupportedRequest);
    }
    uint8_t torque = 0;
    const auto tr = bus_.ReadRegister(id_, model_->torque_enable_reg.address, &torque, 1);
    if (!tr.ok()) {
        return DriverResult<void>::failure(tr.error, tr.detail);
    }
    if (torque != 0) {
        return DriverResult<void>::failure(DriverError::Busy);
    }
    const uint8_t v = static_cast<uint8_t>(mode);
    auto wr = bus_.WriteRegister(id_, model_->operating_mode_reg.address, &v, 1);
    if (wr.ok()) {
        mode_raw_ = v;
        cached_mode_ = mode;
        mode_known_ = true;
    }
    return wr;
}

DriverResult<void> Device::SetMotionProfile(uint32_t profile_acceleration,
                                            uint32_t profile_velocity) noexcept {
    const auto m = RequireModel();
    if (!m.ok()) {
        return m;
    }
    auto acc = WriteSigned(model_->profile_acceleration_reg,
                           static_cast<int32_t>(profile_acceleration),
                           WritePolicy::TxRx);
    if (!acc.ok()) {
        return acc;
    }
    return WriteSigned(model_->profile_velocity_reg,
                       static_cast<int32_t>(profile_velocity),
                       WritePolicy::TxRx);
}

DriverResult<void> Device::SetGoalPosition(int32_t ticks) noexcept {
    const auto m = RequireMode(mode_known_ ? cached_mode_ : OperatingMode::Position);
    if (!m.ok()) {
        return m;
    }
    if (mode_known_ && cached_mode_ != OperatingMode::Position &&
        cached_mode_ != OperatingMode::ExtendedPosition) {
        return DriverResult<void>::failure(DriverError::UnsupportedRequest);
    }
    return WriteSigned(model_->goal_position_reg, ticks, WritePolicy::TxRx);
}

DriverResult<void> Device::SetGoalVelocity(int32_t ticks_per_sec) noexcept {
    auto m = RequireMode(OperatingMode::Velocity);
    if (!mode_known_) {
        m = RequireModel();
    }
    if (!m.ok()) {
        return m;
    }
    if (mode_known_ && cached_mode_ != OperatingMode::Velocity) {
        return DriverResult<void>::failure(DriverError::UnsupportedRequest);
    }
    return WriteSigned(model_->goal_velocity_reg, ticks_per_sec, WritePolicy::TxRx);
}

DriverResult<void> Device::SetGoalPwm(int16_t pwm) noexcept {
    if (mode_known_ && cached_mode_ != OperatingMode::Pwm) {
        return DriverResult<void>::failure(DriverError::UnsupportedRequest);
    }
    const auto m = RequireModel();
    if (!m.ok()) {
        return m;
    }
    return WriteSigned(model_->goal_pwm_reg, pwm, WritePolicy::TxRx);
}

DriverResult<int32_t> Device::ReadPosition() noexcept {
    const auto m = RequireModel();
    if (!m.ok()) {
        return DriverResult<int32_t>::failure(m.error, m.detail);
    }
    return ReadSigned(model_->present_position_reg);
}

DriverResult<Telemetry> Device::ReadTelemetry() noexcept {
    Telemetry tel{};
    const auto m = RequireModel();
    if (!m.ok()) {
        return DriverResult<Telemetry>::failure(m.error, m.detail);
    }
    const auto pos = ReadSigned(model_->present_position_reg);
    if (!pos.ok()) {
        return DriverResult<Telemetry>::failure(pos.error, pos.detail);
    }
    tel.position = pos.value;
    const auto vel = ReadSigned(model_->present_velocity_reg);
    if (vel.ok()) {
        tel.velocity = vel.value;
    }
    const auto load = ReadSigned(model_->present_load_reg);
    if (load.ok()) {
        tel.load = static_cast<int16_t>(load.value);
    }
    const auto pwm = ReadSigned(model_->present_pwm_reg);
    if (pwm.ok()) {
        tel.pwm = static_cast<int16_t>(pwm.value);
    }
    uint8_t volt[2] = {0, 0};
    const auto v = bus_.ReadRegister(id_, model_->present_voltage_reg.address, volt, 2);
    if (v.ok()) {
        tel.voltage_0p1_v = static_cast<uint16_t>(volt[0] | (volt[1] << 8));
    }
    uint8_t temp = 0;
    const auto t = bus_.ReadRegister(id_, model_->present_temperature_reg.address, &temp, 1);
    if (t.ok()) {
        tel.temperature_c = temp;
    }
    uint8_t hw = 0;
    const auto h = bus_.ReadRegister(id_, model_->hardware_error_reg.address, &hw, 1);
    if (h.ok()) {
        tel.hardware_error = hw;
    }
    return DriverResult<Telemetry>::success(tel, pos.detail);
}

DriverResult<uint16_t> Device::ReadRegister(uint16_t address, uint8_t* data,
                                            uint16_t length) noexcept {
    return bus_.ReadRegister(id_, address, data, length);
}

DriverResult<void> Device::WriteRegister(uint16_t address, const uint8_t* data,
                                         uint16_t length, WritePolicy policy) noexcept {
    return bus_.WriteRegister(id_, address, data, length, policy);
}

}  // namespace dynamixel
