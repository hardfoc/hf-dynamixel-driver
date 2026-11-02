#include "dynamixel_bus.hpp"
#include "backend/port_handler_adapter.hpp"

#ifndef WINDECLSPEC
#define WINDECLSPEC
#endif

#include "protocol2_packet_handler.h"
#include "group_sync_write.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace dynamixel {

class Bus::Impl {
public:
    explicit Impl(TransportView transport)
        : adapter_(transport),
          ph_(::dynamixel::Protocol2PacketHandler::getInstance()) {}

    backend::PortHandlerAdapter adapter_;
    ::dynamixel::Protocol2PacketHandler* ph_;
    TransportConfig config_{};
    bool open_{false};
    CommDetail last_{};
};

Bus::Bus(TransportView transport) noexcept
    : impl_(std::make_unique<Impl>(transport)) {}

Bus::Bus(Bus&&) noexcept = default;
Bus& Bus::operator=(Bus&&) noexcept = default;
Bus::~Bus() = default;

TransportView Bus::transport() const noexcept { return impl_->adapter_.transport(); }
const TransportConfig& Bus::config() const noexcept { return impl_->config_; }
CommDetail Bus::last_detail() const noexcept { return impl_->last_; }
bool Bus::IsOpen() const noexcept { return impl_->open_; }

namespace {

CommDetail MakeDetail(uint8_t id, int comm, uint8_t device_error,
                      uint16_t requested, uint16_t transferred, uint64_t now,
                      bool valid) noexcept {
    CommDetail d;
    d.comm_code = comm;
    d.device_error = device_error;
    d.device_id = id;
    d.requested_bytes = requested;
    d.transferred_bytes = transferred;
    d.timestamp_us = now;
    d.response_valid = valid;
    return d;
}

}  // namespace

DriverResult<void> Bus::Open(const TransportConfig& config) noexcept {
    if (!impl_->adapter_.transport().valid()) {
        return DriverResult<void>::failure(DriverError::InvalidParameter);
    }
    impl_->config_ = config;
    impl_->adapter_.set_config(config);
    if (!impl_->adapter_.openPort()) {
        impl_->open_ = false;
        impl_->last_ = MakeDetail(0, COMM_TX_FAIL, 0, 0, 0,
                                  impl_->adapter_.transport().now_us(), false);
        return DriverResult<void>::failure(DriverError::SerialError, impl_->last_);
    }
    impl_->open_ = true;
    impl_->last_ = MakeDetail(0, COMM_SUCCESS, 0, 0, 0,
                              impl_->adapter_.transport().now_us(), true);
    return DriverResult<void>::success(impl_->last_);
}

void Bus::Close() noexcept {
    impl_->adapter_.closePort();
    impl_->open_ = false;
}

DriverResult<void> Bus::SetHostBaudRate(uint32_t baud) noexcept {
    if (!impl_->open_) {
        return DriverResult<void>::failure(DriverError::NotInitialized);
    }
    if (impl_->adapter_.is_using_) {
        return DriverResult<void>::failure(DriverError::Busy);
    }
    if (!impl_->adapter_.setBaudRate(static_cast<int>(baud))) {
        return DriverResult<void>::failure(DriverError::SerialError);
    }
    impl_->config_.baud_rate = baud;
    return DriverResult<void>::success();
}

DriverResult<PingInfo> Bus::Ping(uint8_t id) noexcept {
    PingInfo info{};
    info.id = id;
    if (!impl_->open_) {
        return DriverResult<PingInfo>::failure(DriverError::NotInitialized);
    }
    if (!IsDeviceId(id)) {
        return DriverResult<PingInfo>::failure(DriverError::InvalidId);
    }
    uint16_t model = 0;
    uint8_t error = 0;
    const int comm = impl_->ph_->ping(&impl_->adapter_, id, &model, &error);
    if (!impl_->adapter_.last_echo_ok()) {
        impl_->last_ = MakeDetail(id, COMM_TX_FAIL, 0, 0, 0,
                                  impl_->adapter_.transport().now_us(), false);
        return DriverResult<PingInfo>::failure(DriverError::EchoMismatch, impl_->last_);
    }
    impl_->last_ = MakeDetail(id, comm, error, 2, (comm == COMM_SUCCESS) ? 2 : 0,
                              impl_->adapter_.transport().now_us(),
                              comm == COMM_SUCCESS);
    if (comm != COMM_SUCCESS) {
        return DriverResult<PingInfo>::failure(MapCommCode(comm), impl_->last_);
    }
    if (error != 0) {
        info.model_number = model;
        impl_->last_.response_valid = false;
        return DriverResult<PingInfo>::failure(DriverError::DeviceError, impl_->last_);
    }
    info.model_number = model;
    uint8_t fw = 0;
    uint8_t fw_err = 0;
    const int fw_comm = impl_->ph_->read1ByteTxRx(&impl_->adapter_, id, 6, &fw, &fw_err);
    if (fw_comm == COMM_SUCCESS && fw_err == 0) {
        info.firmware_version = fw;
        info.firmware_valid = true;
    }
    return DriverResult<PingInfo>::success(info, impl_->last_);
}

DriverResult<uint8_t> Bus::Scan(uint8_t* out_ids, uint8_t max_ids,
                                uint8_t first_id, uint8_t last_id) noexcept {
    if (out_ids == nullptr || max_ids == 0) {
        return DriverResult<uint8_t>::failure(DriverError::InvalidParameter);
    }
    // Protocol 2.0 section 2.3: device IDs are 0 to 252 (0xFC); 254 is broadcast.
    if (last_id > kMaxDeviceId || first_id > last_id) {
        return DriverResult<uint8_t>::failure(DriverError::InvalidParameter);
    }
    uint8_t found = 0;
    for (uint16_t id = first_id; id <= last_id && found < max_ids; ++id) {
        const auto ping = Ping(static_cast<uint8_t>(id));
        if (ping.ok()) {
            out_ids[found++] = static_cast<uint8_t>(id);
        }
    }
    return DriverResult<uint8_t>::success(found, impl_->last_);
}

DriverResult<uint16_t> Bus::ReadRegister(uint8_t id, uint16_t address,
                                         uint8_t* data, uint16_t length) noexcept {
    if (!impl_->open_) {
        return DriverResult<uint16_t>::failure(DriverError::NotInitialized);
    }
    if (!IsDeviceId(id)) {
        return DriverResult<uint16_t>::failure(DriverError::InvalidId);
    }
    if (data == nullptr || length == 0) {
        return DriverResult<uint16_t>::failure(DriverError::InvalidParameter);
    }
    uint8_t error = 0;
    const int comm = impl_->ph_->readTxRx(&impl_->adapter_, id, address, length, data, &error);
    impl_->last_ = MakeDetail(id, comm, error, length,
                              (comm == COMM_SUCCESS) ? length : 0,
                              impl_->adapter_.transport().now_us(),
                              comm == COMM_SUCCESS && error == 0);
    if (comm != COMM_SUCCESS) {
        return DriverResult<uint16_t>::failure(MapCommCode(comm), impl_->last_);
    }
    if (error != 0) {
        return DriverResult<uint16_t>::failure(DriverError::DeviceError, impl_->last_);
    }
    return DriverResult<uint16_t>::success(length, impl_->last_);
}

DriverResult<void> Bus::WriteRegister(uint8_t id, uint16_t address,
                                      const uint8_t* data, uint16_t length,
                                      WritePolicy policy) noexcept {
    if (!impl_->open_) {
        return DriverResult<void>::failure(DriverError::NotInitialized);
    }
    if (data == nullptr || length == 0) {
        return DriverResult<void>::failure(DriverError::InvalidParameter);
    }
    if (id != kBroadcastId && !IsDeviceId(id)) {
        return DriverResult<void>::failure(DriverError::InvalidId);
    }
    if (id == kBroadcastId && policy != WritePolicy::TxOnly) {
        return DriverResult<void>::failure(DriverError::InvalidParameter);
    }

    uint8_t* writable = const_cast<uint8_t*>(data);
    uint8_t error = 0;
    int comm = COMM_TX_FAIL;
    if (policy == WritePolicy::TxOnly) {
        comm = impl_->ph_->writeTxOnly(&impl_->adapter_, id, address, length, writable);
        impl_->last_ = MakeDetail(id, comm, 0, length,
                                  (comm == COMM_SUCCESS) ? length : 0,
                                  impl_->adapter_.transport().now_us(),
                                  comm == COMM_SUCCESS);
        if (comm != COMM_SUCCESS) {
            return DriverResult<void>::failure(MapCommCode(comm), impl_->last_);
        }
        return DriverResult<void>::success(impl_->last_);
    }

    comm = impl_->ph_->writeTxRx(&impl_->adapter_, id, address, length, writable, &error);
    impl_->last_ = MakeDetail(id, comm, error, length,
                              (comm == COMM_SUCCESS) ? length : 0,
                              impl_->adapter_.transport().now_us(),
                              comm == COMM_SUCCESS && error == 0);
    if (comm != COMM_SUCCESS) {
        const DriverError mapped = MapCommCode(comm);
        if (mapped == DriverError::Timeout) {
            return DriverResult<void>::failure(DriverError::UncertainOutcome, impl_->last_);
        }
        return DriverResult<void>::failure(mapped, impl_->last_);
    }
    if (error != 0) {
        return DriverResult<void>::failure(DriverError::DeviceError, impl_->last_);
    }
    if (policy == WritePolicy::ReadbackVerify) {
        uint8_t back[64];
        if (length > sizeof(back)) {
            return DriverResult<void>::failure(DriverError::BufferTooSmall, impl_->last_);
        }
        const auto rb = ReadRegister(id, address, back, length);
        if (!rb.ok()) {
            return DriverResult<void>::failure(rb.error, rb.detail);
        }
        if (std::memcmp(back, data, length) != 0) {
            impl_->last_.response_valid = false;
            return DriverResult<void>::failure(DriverError::MalformedResponse, impl_->last_);
        }
    }
    return DriverResult<void>::success(impl_->last_);
}

DriverResult<uint8_t> Bus::SyncWrite(uint16_t address, uint16_t length,
                                     const uint8_t* ids, const uint8_t* const* payloads,
                                     uint8_t count) noexcept {
    if (!impl_->open_) {
        return DriverResult<uint8_t>::failure(DriverError::NotInitialized);
    }
    if (ids == nullptr || payloads == nullptr || count == 0 || length == 0) {
        return DriverResult<uint8_t>::failure(DriverError::InvalidParameter);
    }
    for (uint8_t i = 0; i < count; ++i) {
        if (!IsDeviceId(ids[i])) {
            return DriverResult<uint8_t>::failure(DriverError::InvalidId);
        }
    }
    ::dynamixel::GroupSyncWrite group(&impl_->adapter_, impl_->ph_, address, length);
    uint8_t added = 0;
    for (uint8_t i = 0; i < count; ++i) {
        if (payloads[i] == nullptr) {
            continue;
        }
        if (group.addParam(ids[i], const_cast<uint8_t*>(payloads[i]))) {
            ++added;
        }
    }
    const int comm = group.txPacket();
    impl_->last_ = MakeDetail(kBroadcastId, comm, 0, static_cast<uint16_t>(length * added),
                              (comm == COMM_SUCCESS) ? static_cast<uint16_t>(length * added) : 0,
                              impl_->adapter_.transport().now_us(),
                              comm == COMM_SUCCESS);
    if (comm != COMM_SUCCESS) {
        return DriverResult<uint8_t>::failure(MapCommCode(comm), impl_->last_);
    }
    return DriverResult<uint8_t>::success(added, impl_->last_);
}

DriverResult<uint8_t> Bus::SyncRead(uint16_t address, uint16_t length,
                                    const uint8_t* ids, uint8_t count,
                                    uint8_t* const* payloads,
                                    SyncReadItem* items) noexcept {
    if (!impl_->open_) {
        return DriverResult<uint8_t>::failure(DriverError::NotInitialized);
    }
    if (ids == nullptr || payloads == nullptr || items == nullptr ||
        count == 0 || length == 0) {
        return DriverResult<uint8_t>::failure(DriverError::InvalidParameter);
    }
    // Drive sync-read TX once, then collect each status packet independently.
    // The stock GroupSyncRead treats the first missing servo as a total failure.
    std::vector<uint8_t> param(count);
    for (uint8_t i = 0; i < count; ++i) {
        if (!IsDeviceId(ids[i])) {
            return DriverResult<uint8_t>::failure(DriverError::InvalidId);
        }
        param[i] = ids[i];
        items[i] = SyncReadItem{};
        items[i].id = ids[i];
        items[i].valid = false;
        if (payloads[i] == nullptr) {
            return DriverResult<uint8_t>::failure(DriverError::InvalidParameter);
        }
    }
    const int tx = impl_->ph_->syncReadTx(&impl_->adapter_, address, length,
                                          param.data(), count);
    if (tx != COMM_SUCCESS) {
        impl_->last_ = MakeDetail(kBroadcastId, tx, 0, static_cast<uint16_t>(length * count),
                                  0, impl_->adapter_.transport().now_us(), false);
        return DriverResult<uint8_t>::failure(MapCommCode(tx), impl_->last_);
    }
    uint8_t valid = 0;
    int last_comm = COMM_SUCCESS;
    for (uint8_t i = 0; i < count; ++i) {
        impl_->adapter_.setPacketTimeout(static_cast<uint16_t>(length + 11));
        uint8_t err = 0;
        const int rx = impl_->ph_->readRx(&impl_->adapter_, ids[i], length, payloads[i], &err);
        last_comm = rx;
        if (rx == COMM_SUCCESS) {
            items[i].valid = (err == 0);
            items[i].device_error = err;
            if (err == 0) {
                ++valid;
            }
        }
    }
    impl_->last_ = MakeDetail(kBroadcastId, last_comm, 0,
                              static_cast<uint16_t>(length * count),
                              static_cast<uint16_t>(length * valid),
                              impl_->adapter_.transport().now_us(),
                              valid > 0);
    return DriverResult<uint8_t>::success(valid, impl_->last_);
}

}  // namespace dynamixel
