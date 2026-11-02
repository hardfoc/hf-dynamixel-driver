/**
 * @file dynamixel_model.hpp
 * @brief Hand-authored X-series Protocol 2.0 descriptors.
 *
 * @details XC430-W150 is the first validated model. Other common X-series
 *          numbers share the same control-table addresses and can bind
 *          here. An unknown larger servo can use BindXSeriesFallback()
 *          after a successful ping.
 *
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 */
#pragma once

#include "dynamixel_types.hpp"

#include <cstdint>
#include <cstddef>

namespace dynamixel {

enum class RegisterAccess : uint8_t { ReadOnly, WriteOnly, ReadWrite };
enum class RegisterStorage : uint8_t { Eeprom, Ram };

struct RegisterDesc {
    uint16_t        address;
    uint8_t         width;
    bool            is_signed;
    RegisterAccess  access;
    RegisterStorage storage;
};

struct ModelDescriptor {
    uint16_t    model_number;   ///< 0 = generic X-series fallback.
    const char* name;
    uint32_t    position_unit_0p001_deg;  ///< 0.088 deg/tick → 88.
    bool        supports_velocity;
    bool        supports_position;
    bool        supports_extended_position;
    bool        supports_pwm;
    bool        supports_current;
    bool        effort_is_current;  ///< Address 126 is current (XM/XH) vs load (XC/XL).
    RegisterDesc model_number_reg;
    RegisterDesc firmware_reg;
    RegisterDesc id_reg;
    RegisterDesc baud_reg;
    RegisterDesc operating_mode_reg;
    RegisterDesc torque_enable_reg;
    RegisterDesc hardware_error_reg;
    RegisterDesc goal_pwm_reg;
    RegisterDesc goal_current_reg;
    RegisterDesc goal_velocity_reg;
    RegisterDesc profile_acceleration_reg;
    RegisterDesc profile_velocity_reg;
    RegisterDesc goal_position_reg;
    RegisterDesc present_pwm_reg;
    RegisterDesc present_load_reg;
    RegisterDesc present_velocity_reg;
    RegisterDesc present_position_reg;
    RegisterDesc present_voltage_reg;
    RegisterDesc present_temperature_reg;
};

inline constexpr RegisterDesc kXSeriesModelNumberReg{0, 2, false, RegisterAccess::ReadOnly, RegisterStorage::Eeprom};
inline constexpr RegisterDesc kXSeriesFirmwareReg{6, 1, false, RegisterAccess::ReadOnly, RegisterStorage::Eeprom};
inline constexpr RegisterDesc kXSeriesIdReg{7, 1, false, RegisterAccess::ReadWrite, RegisterStorage::Eeprom};
inline constexpr RegisterDesc kXSeriesBaudReg{8, 1, false, RegisterAccess::ReadWrite, RegisterStorage::Eeprom};
inline constexpr RegisterDesc kXSeriesOperatingModeReg{11, 1, false, RegisterAccess::ReadWrite, RegisterStorage::Eeprom};
inline constexpr RegisterDesc kXSeriesTorqueEnableReg{64, 1, false, RegisterAccess::ReadWrite, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesHardwareErrorReg{70, 1, false, RegisterAccess::ReadOnly, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesGoalPwmReg{100, 2, true, RegisterAccess::ReadWrite, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesGoalCurrentReg{102, 2, true, RegisterAccess::ReadWrite, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesGoalVelocityReg{104, 4, true, RegisterAccess::ReadWrite, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesProfileAccelReg{108, 4, false, RegisterAccess::ReadWrite, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesProfileVelReg{112, 4, false, RegisterAccess::ReadWrite, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesGoalPositionReg{116, 4, true, RegisterAccess::ReadWrite, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesPresentPwmReg{124, 2, true, RegisterAccess::ReadOnly, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesPresentEffortReg{126, 2, true, RegisterAccess::ReadOnly, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesPresentVelocityReg{128, 4, true, RegisterAccess::ReadOnly, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesPresentPositionReg{132, 4, true, RegisterAccess::ReadOnly, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesPresentVoltageReg{144, 2, false, RegisterAccess::ReadOnly, RegisterStorage::Ram};
inline constexpr RegisterDesc kXSeriesPresentTemperatureReg{146, 1, false, RegisterAccess::ReadOnly, RegisterStorage::Ram};

constexpr ModelDescriptor MakeXSeries(uint16_t number, const char* name,
                                      bool supports_current, bool effort_is_current) noexcept {
    return ModelDescriptor{
        number,
        name,
        88,
        true,
        true,
        true,
        true,
        supports_current,
        effort_is_current,
        kXSeriesModelNumberReg,
        kXSeriesFirmwareReg,
        kXSeriesIdReg,
        kXSeriesBaudReg,
        kXSeriesOperatingModeReg,
        kXSeriesTorqueEnableReg,
        kXSeriesHardwareErrorReg,
        kXSeriesGoalPwmReg,
        kXSeriesGoalCurrentReg,
        kXSeriesGoalVelocityReg,
        kXSeriesProfileAccelReg,
        kXSeriesProfileVelReg,
        kXSeriesGoalPositionReg,
        kXSeriesPresentPwmReg,
        kXSeriesPresentEffortReg,
        kXSeriesPresentVelocityReg,
        kXSeriesPresentPositionReg,
        kXSeriesPresentVoltageReg,
        kXSeriesPresentTemperatureReg,
    };
}

inline constexpr uint16_t kXc430W150ModelNumber = 1070;
inline constexpr uint16_t kXc430W240ModelNumber = 1080;
inline constexpr uint16_t kXl430W250ModelNumber = 1060;
inline constexpr uint16_t kXm430W210ModelNumber = 1030;
inline constexpr uint16_t kXm430W350ModelNumber = 1020;
inline constexpr uint16_t kXh430W210ModelNumber = 1010;
inline constexpr uint16_t kXh430W350ModelNumber = 1000;

/// First validated bench servo.
inline constexpr ModelDescriptor kXc430W150 =
    MakeXSeries(kXc430W150ModelNumber, "XC430-W150", false, false);

inline constexpr ModelDescriptor kXc430W240 =
    MakeXSeries(kXc430W240ModelNumber, "XC430-W240", false, false);

inline constexpr ModelDescriptor kXl430W250 =
    MakeXSeries(kXl430W250ModelNumber, "XL430-W250", false, false);

/// Larger X-series with current registers (XM430-W350).
inline constexpr ModelDescriptor kXm430W350 =
    MakeXSeries(kXm430W350ModelNumber, "XM430-W350", true, true);

inline constexpr ModelDescriptor kXm430W210 =
    MakeXSeries(kXm430W210ModelNumber, "XM430-W210", true, true);

inline constexpr ModelDescriptor kXh430W210 =
    MakeXSeries(kXh430W210ModelNumber, "XH430-W210", true, true);

inline constexpr ModelDescriptor kXh430W350 =
    MakeXSeries(kXh430W350ModelNumber, "XH430-W350", true, true);

/// Bind this after ping when the model number is not in the table yet.
inline constexpr ModelDescriptor kXSeriesFallback =
    MakeXSeries(0, "X-series (fallback)", false, false);

inline constexpr const ModelDescriptor* kKnownModels[] = {
    &kXc430W150, &kXc430W240, &kXl430W250, &kXm430W350, &kXm430W210,
    &kXh430W210, &kXh430W350,
};

inline bool ModeSupported(const ModelDescriptor& m, OperatingMode mode) noexcept {
    switch (mode) {
        case OperatingMode::Velocity:         return m.supports_velocity;
        case OperatingMode::Position:         return m.supports_position;
        case OperatingMode::ExtendedPosition: return m.supports_extended_position;
        case OperatingMode::Pwm:              return m.supports_pwm;
        case OperatingMode::Unknown:          return false;
    }
    return false;
}

inline const ModelDescriptor* FindModel(uint16_t model_number) noexcept {
    for (const ModelDescriptor* m : kKnownModels) {
        if (m->model_number == model_number) {
            return m;
        }
    }
    return nullptr;
}

struct Telemetry {
    int32_t  position{0};
    int32_t  velocity{0};
    int16_t  load{0};          ///< Load estimate (XC/XL) or current (XM/XH).
    int16_t  pwm{0};
    uint16_t voltage_0p1_v{0};
    uint8_t  temperature_c{0};
    uint8_t  hardware_error{0};
};

}  // namespace dynamixel
