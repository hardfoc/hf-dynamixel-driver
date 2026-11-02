#pragma once

#include "dynamixel_transport.hpp"
#include "protocol2_status.hpp"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <vector>

namespace hf_dxl_test {

class FakeTransport : public dynamixel::Transport<FakeTransport> {
public:
    dynamixel::EchoPolicy echo_policy{dynamixel::EchoPolicy::None};
    bool fail_write{false};
    bool fail_read{false};
    bool fail_open{false};
    bool fail_drain{false};
    std::size_t max_write{1024};
    std::size_t max_read{1024};
    uint64_t tick_us{100};
    uint64_t time_us{0};
    bool auto_reply{true};
    bool corrupt_next_crc{false};
    uint16_t model_number{1070};
    uint8_t firmware{46};
    uint8_t hardware_error{0};  ///< Non-zero: latched fault, Alert bit in every status.
    int reboots{0};
    int next_error{-1};  ///< >= 0: error byte for the next status packet only.
    std::vector<std::vector<uint8_t>> tx_log;
    std::deque<uint8_t> rx;

    uint8_t registers[256]{};
    bool present[256]{};

    FakeTransport() {
        registers[0] = static_cast<uint8_t>(model_number & 0xFF);
        registers[1] = static_cast<uint8_t>((model_number >> 8) & 0xFF);
        registers[6] = firmware;
        registers[7] = 1;
        registers[11] = 3;
        registers[64] = 0;
        present[0] = present[1] = present[6] = present[7] = present[11] = present[64] = true;
    }

    dynamixel::DriverError open(const dynamixel::TransportConfig& cfg) noexcept {
        echo_policy = cfg.echo_policy;
        return fail_open ? dynamixel::DriverError::SerialError : dynamixel::DriverError::None;
    }
    void close() noexcept {}
    dynamixel::DriverError set_baud_rate(uint32_t) noexcept {
        return dynamixel::DriverError::None;
    }
    dynamixel::TransportIo write_some(const uint8_t* data, std::size_t length) noexcept {
        if (fail_write) {
            return {0, dynamixel::DriverError::SerialError};
        }
        const std::size_t n = (length < max_write) ? length : max_write;
        tx_log.emplace_back(data, data + n);
        if (echo_policy == dynamixel::EchoPolicy::ExpectedLocalEcho) {
            rx.insert(rx.end(), data, data + n);
        }
        if (auto_reply) {
            QueueReply(data, n);
            if (corrupt_next_crc && !rx.empty()) {
                rx.back() = static_cast<uint8_t>(rx.back() ^ 0xFF);
                corrupt_next_crc = false;
            }
        }
        return {n, dynamixel::DriverError::None};
    }
    dynamixel::DriverError finish_transmit() noexcept {
        return fail_drain ? dynamixel::DriverError::SerialError : dynamixel::DriverError::None;
    }
    dynamixel::TransportIo read_some(uint8_t* out, std::size_t max) noexcept {
        if (fail_read) {
            return {0, dynamixel::DriverError::SerialError};
        }
        const std::size_t n = std::min(max, std::min(max_read, rx.size()));
        for (std::size_t i = 0; i < n; ++i) {
            out[i] = rx.front();
            rx.pop_front();
        }
        return {n, dynamixel::DriverError::None};
    }
    void discard_stale_input() noexcept { rx.clear(); }
    uint64_t now_us() noexcept {
        time_us += tick_us;
        return time_us;
    }

    void Queue(const std::vector<uint8_t>& bytes) {
        rx.insert(rx.end(), bytes.begin(), bytes.end());
    }

private:
    void QueueReply(const uint8_t* tx, std::size_t n) {
        if (n < 8) {
            return;
        }
        const uint8_t inst = TxInstruction(tx, n);
        const uint8_t id = TxId(tx, n);
        uint8_t err = (hardware_error != 0) ? 0x80 : 0x00;
        if (next_error >= 0) {
            err = static_cast<uint8_t>(next_error);
            next_error = -1;
        }
        registers[70] = hardware_error;
        if (inst == 1) {  // PING
            Queue(PingStatus(id, model_number, err));
            return;
        }
        if (inst == 0x08) {  // REBOOT: status first, then RAM back to defaults
            if (id != 0xFE) {
                Queue(WriteStatus(id, err));
            }
            ++reboots;
            hardware_error = 0;
            registers[70] = 0;
            registers[64] = 0;
            return;
        }
        if (inst == 2 && n >= 12) {  // READ
            const uint16_t addr = static_cast<uint16_t>(tx[8] | (tx[9] << 8));
            const uint16_t len = static_cast<uint16_t>(tx[10] | (tx[11] << 8));
            std::vector<uint8_t> data(len, 0);
            for (uint16_t i = 0; i < len && (addr + i) < sizeof(registers); ++i) {
                data[i] = registers[addr + i];
            }
            Queue(ReadStatus(id, data.data(), static_cast<uint16_t>(data.size()), err));
            return;
        }
        if (inst == 3 && n >= 10) {  // WRITE
            const uint16_t addr = static_cast<uint16_t>(tx[8] | (tx[9] << 8));
            const uint16_t payload = static_cast<uint16_t>(n - 12);
            for (uint16_t i = 0; i < payload && (addr + i) < sizeof(registers); ++i) {
                registers[addr + i] = tx[10 + i];
                present[addr + i] = true;
            }
            if (id != 0xFE) {
                Queue(WriteStatus(id, err));
            }
            return;
        }
        if (inst == 0x82) {  // SYNC READ
            const uint16_t addr = static_cast<uint16_t>(tx[8] | (tx[9] << 8));
            const uint16_t len = static_cast<uint16_t>(tx[10] | (tx[11] << 8));
            for (std::size_t i = 12; i + 2 <= n; ++i) {
                const uint8_t sid = tx[i];
                if (sid == 0x00 && i + 2 >= n) {
                    break;
                }
                std::vector<uint8_t> data(len, 0);
                for (uint16_t b = 0; b < len && (addr + b) < sizeof(registers); ++b) {
                    data[b] = registers[addr + b];
                }
                Queue(ReadStatus(sid, data.data(), static_cast<uint16_t>(data.size()), err));
            }
        }
    }
};

}  // namespace hf_dxl_test
