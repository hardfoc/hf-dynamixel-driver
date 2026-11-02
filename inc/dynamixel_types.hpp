/**
 * @file dynamixel_types.hpp
 * @brief Public result and status types for the Dynamixel driver.
 *
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 */
#pragma once

#include <cstdint>
#include <cstddef>
#include <string_view>

namespace dynamixel {

enum class DriverError : uint8_t {
    None = 0,
    NotInitialized,
    InvalidParameter,
    InvalidId,
    BufferTooSmall,
    Busy,
    SerialError,
    Timeout,
    MalformedResponse,
    ChecksumError,
    DeviceError,
    UnsupportedRequest,
    UncertainOutcome,
    EchoMismatch,
};

constexpr std::string_view ToString(DriverError e) noexcept {
    switch (e) {
        case DriverError::None:               return "None";
        case DriverError::NotInitialized:     return "NotInitialized";
        case DriverError::InvalidParameter:   return "InvalidParameter";
        case DriverError::InvalidId:          return "InvalidId";
        case DriverError::BufferTooSmall:     return "BufferTooSmall";
        case DriverError::Busy:               return "Busy";
        case DriverError::SerialError:        return "SerialError";
        case DriverError::Timeout:            return "Timeout";
        case DriverError::MalformedResponse:  return "MalformedResponse";
        case DriverError::ChecksumError:      return "ChecksumError";
        case DriverError::DeviceError:        return "DeviceError";
        case DriverError::UnsupportedRequest: return "UnsupportedRequest";
        case DriverError::UncertainOutcome:   return "UncertainOutcome";
        case DriverError::EchoMismatch:       return "EchoMismatch";
    }
    return "?";
}

/// Raw SDK communication code plus device-side status from the last transaction.
struct CommDetail {
    int32_t  comm_code{0};       ///< SDK COMM_* value (0 = success).
    uint8_t  device_error{0};    ///< Protocol 2.0 error byte.
    uint8_t  device_id{0};
    uint16_t requested_bytes{0};
    uint16_t transferred_bytes{0};
    uint64_t timestamp_us{0};
    bool     response_valid{false};
};

template <typename T>
struct DriverResult {
    T            value{};
    DriverError  error{DriverError::None};
    CommDetail   detail{};

    constexpr bool ok() const noexcept { return error == DriverError::None; }
    constexpr explicit operator bool() const noexcept { return ok(); }

    static constexpr DriverResult success(T v, CommDetail d = {}) noexcept {
        return {v, DriverError::None, d};
    }
    static constexpr DriverResult failure(DriverError e, CommDetail d = {}) noexcept {
        return {T{}, e, d};
    }
};

template <>
struct DriverResult<void> {
    DriverError error{DriverError::None};
    CommDetail  detail{};

    constexpr bool ok() const noexcept { return error == DriverError::None; }
    constexpr explicit operator bool() const noexcept { return ok(); }

    static constexpr DriverResult success(CommDetail d = {}) noexcept {
        return {DriverError::None, d};
    }
    static constexpr DriverResult failure(DriverError e, CommDetail d = {}) noexcept {
        return {e, d};
    }
};

enum class EchoPolicy : uint8_t {
    None = 0,
    ExpectedLocalEcho,
};

enum class WritePolicy : uint8_t {
    TxOnly = 0,       ///< Packet left the host; no status expected.
    TxRx,             ///< Wait for the device status packet.
    ReadbackVerify,   ///< TxRx, then read the same register back.
};

/// Operating Mode (address 11) values with typed support. X-series e-Manual,
/// section 2.4 Operating Mode (11): 0 current, 1 velocity, 3 position,
/// 4 extended position, 5 current-based position, 16 PWM.
enum class OperatingMode : uint8_t {
    Velocity = 1,
    Position = 3,
    ExtendedPosition = 4,
    Pwm = 16,
    /// Not read yet, or a register value this driver has no typed mode for
    /// (current 0, current-based position 5, or an undefined value).
    Unknown = 0xFF,
};

/// Map a raw Operating Mode register value; anything without a typed mode
/// becomes `OperatingMode::Unknown` instead of an out-of-range enum value.
constexpr OperatingMode OperatingModeFromRaw(uint8_t raw) noexcept {
    switch (raw) {
        case 1:  return OperatingMode::Velocity;
        case 3:  return OperatingMode::Position;
        case 4:  return OperatingMode::ExtendedPosition;
        case 16: return OperatingMode::Pwm;
        default: return OperatingMode::Unknown;
    }
}

struct PingInfo {
    uint8_t  id{0};
    uint16_t model_number{0};
    uint8_t  firmware_version{0};
    bool     firmware_valid{false};
};

struct TransportIo {
    std::size_t accepted{0};
    DriverError error{DriverError::None};
};

/// Protocol 2.0 section 2.3: unicast device IDs are 0 to 252 (0xFC), 254 (0xFE) is broadcast.
inline constexpr uint8_t kBroadcastId = 0xFE;
inline constexpr uint8_t kMinDeviceId = 0x00;
inline constexpr uint8_t kMaxDeviceId = 0xFC;

/// True for a unicast device ID (0 to 252). Excludes 253 and the broadcast ID 254.
constexpr bool IsDeviceId(uint8_t id) noexcept { return id <= kMaxDeviceId; }
/// Protocol 2.0 section 3.2: bit 7 of the status packet error byte is the Alert
/// flag (read Hardware Error Status); bits 0 to 6 are the error number.
inline constexpr uint8_t kStatusAlertBit = 0x80;
inline constexpr uint8_t kStatusErrorNumberMask = 0x7F;

inline constexpr uint32_t kDefaultBaud = 57600;
/// Default `TransportConfig::max_packet_bytes`: the largest instruction or
/// status packet (before byte stuffing) the bus builds or asks a servo for.
inline constexpr std::size_t kMaxPacketBytes = 256;
/// Smallest accepted `max_packet_bytes`: a Ping status packet (14 bytes).
inline constexpr std::size_t kMinPacketBytes = 14;
/// The bundled ROBOTIS SDK packet buffers (1024 bytes); a larger
/// `max_packet_bytes` is capped to this.
inline constexpr std::size_t kSdkMaxPacketBytes = 1024;

/// Protocol 2.0 section 2.4: Length = parameters + 3. An instruction packet is
/// header (4) + ID + Length (2) + Instruction + parameters + CRC (2) bytes.
constexpr std::size_t InstructionPacketBytes(std::size_t parameters) noexcept {
    return 10 + parameters;
}
/// A status packet adds the Error byte (section 3.2) to the same layout.
constexpr std::size_t StatusPacketBytes(std::size_t parameters) noexcept {
    return 11 + parameters;
}

}  // namespace dynamixel
