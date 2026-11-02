/**
 * @file dynamixel_bus.hpp
 * @brief Shared Protocol 2.0 bus over a caller-supplied transport.
 *
 * @details The transport and clock outlive the bus. The bus is not copyable
 *          and is not thread-safe. Constructors do not open the UART or
 *          move servos.
 *
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 */
#pragma once

#include "dynamixel_transport.hpp"

#include <cstdint>
#include <cstddef>
#include <memory>

namespace dynamixel {

struct SyncReadItem {
    uint8_t id{0};
    bool    valid{false};
    uint8_t device_error{0};
};

class Bus {
public:
    Bus() = delete;
    explicit Bus(TransportView transport) noexcept;
    Bus(const Bus&) = delete;
    Bus& operator=(const Bus&) = delete;
    Bus(Bus&&) noexcept;
    Bus& operator=(Bus&&) noexcept;
    ~Bus();

    DriverResult<void> Open(const TransportConfig& config) noexcept;
    void Close() noexcept;
    bool IsOpen() const noexcept;

    DriverResult<void> SetHostBaudRate(uint32_t baud) noexcept;

    DriverResult<PingInfo> Ping(uint8_t id) noexcept;
    /// Ping first_id..last_id (inclusive). The default covers every device ID, 0 to 252.
    DriverResult<uint8_t> Scan(uint8_t* out_ids, uint8_t max_ids,
                               uint8_t first_id = kMinDeviceId,
                               uint8_t last_id = kMaxDeviceId) noexcept;

    DriverResult<uint16_t> ReadRegister(uint8_t id, uint16_t address,
                                        uint8_t* data, uint16_t length) noexcept;
    DriverResult<void> WriteRegister(uint8_t id, uint16_t address,
                                     const uint8_t* data, uint16_t length,
                                     WritePolicy policy = WritePolicy::TxRx) noexcept;

    DriverResult<uint8_t> SyncWrite(uint16_t address, uint16_t length,
                                    const uint8_t* ids, const uint8_t* const* payloads,
                                    uint8_t count) noexcept;
    DriverResult<uint8_t> SyncRead(uint16_t address, uint16_t length,
                                   const uint8_t* ids, uint8_t count,
                                   uint8_t* const* payloads,
                                   SyncReadItem* items) noexcept;

    TransportView transport() const noexcept;
    const TransportConfig& config() const noexcept;
    CommDetail last_detail() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

DriverError MapCommCode(int comm_code) noexcept;

}  // namespace dynamixel
