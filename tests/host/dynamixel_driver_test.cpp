#include "dynamixel.hpp"
#include "fake_transport.hpp"

#include <cstdio>
#include <cstring>

namespace {

int g_failures = 0;

#define CHECK(cond)                                                                    \
    do {                                                                               \
        if (!(cond)) {                                                                 \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                \
            ++g_failures;                                                              \
        }                                                                              \
    } while (0)

using hf_dxl_test::FakeTransport;

void TestPingAndRead() {
    FakeTransport uart;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    const auto ping = bus.Ping(1);
    CHECK(ping.ok());
    CHECK(ping.value.model_number == 1070);
    CHECK(ping.value.firmware_valid);
    CHECK(ping.value.firmware_version == 46);
    CHECK(std::strcmp(dynamixel::GetSdkTag(), "4.1.0") == 0);

    uint8_t mode = 0;
    const auto rd = bus.ReadRegister(1, 11, &mode, 1);
    CHECK(rd.ok());
    CHECK(mode == 3);
}

void TestTimeout() {
    FakeTransport uart;
    uart.auto_reply = false;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    const auto ping = bus.Ping(1);
    CHECK(!ping.ok());
    CHECK(ping.error == dynamixel::DriverError::Timeout);
    CHECK(ping.detail.comm_code != 0);
}

void TestCorruptChecksum() {
    FakeTransport uart;
    uart.corrupt_next_crc = true;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    const auto ping = bus.Ping(1);
    CHECK(!ping.ok());
    CHECK(ping.error == dynamixel::DriverError::ChecksumError);
}

void TestWrongId() {
    FakeTransport uart;
    uart.auto_reply = false;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    uart.Queue(hf_dxl_test::PingStatus(9, 1070));
    const auto ping = bus.Ping(1);
    CHECK(!ping.ok());
}

void TestEchoMatchAndMismatch() {
    FakeTransport uart;
    dynamixel::TransportConfig cfg;
    cfg.echo_policy = dynamixel::EchoPolicy::ExpectedLocalEcho;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open(cfg).ok());
    CHECK(bus.Ping(1).ok());

    FakeTransport bad;
    bad.echo_policy = dynamixel::EchoPolicy::ExpectedLocalEcho;
    bad.auto_reply = false;
    dynamixel::Bus bus2{dynamixel::TransportView{bad}};
    CHECK(bus2.Open(cfg).ok());
    bad.Queue({0x00});  // leftover that will not match TX; write also appends echo then...
    // Force mismatch by injecting a wrong first echo byte ahead of write.
    // After clearPort the leftover is gone; instead corrupt by failing to echo.
    bad.echo_policy = dynamixel::EchoPolicy::None;
    const auto ping = bus2.Ping(1);
    CHECK(!ping.ok());
    CHECK(ping.error == dynamixel::DriverError::EchoMismatch ||
          ping.error == dynamixel::DriverError::Timeout);
}

void TestPartialWriteAndDrain() {
    FakeTransport uart;
    uart.fail_drain = true;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    CHECK(!bus.Ping(1).ok());

    FakeTransport uart2;
    uart2.fail_write = true;
    dynamixel::Bus bus2{dynamixel::TransportView{uart2}};
    CHECK(bus2.Open({}).ok());
    CHECK(!bus2.Ping(1).ok());
}

void TestFragmentedRx() {
    FakeTransport uart;
    uart.max_read = 1;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    CHECK(bus.Ping(1).ok());
}

void TestDeviceTypedApi() {
    FakeTransport uart;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    dynamixel::Device dev(bus, 1);
    CHECK(!dev.SetTorqueEnabled(true).ok());  // no descriptor yet
    CHECK(dev.Identify().ok());
    CHECK(dev.identified());
    if (dev.model() == nullptr) {
        return;
    }
    CHECK(dev.model()->model_number == 1070);
    CHECK(!dev.model()->supports_current);

    CHECK(dev.SetOperatingMode(dynamixel::OperatingMode::Position).ok());
    CHECK(dev.SetMotionProfile(10, 50).ok());
    CHECK(dev.SetGoalPosition(2048).ok());
    CHECK(!dev.SetGoalVelocity(100).ok());  // wrong mode

    uart.registers[132] = 0x00;
    uart.registers[133] = 0x08;
    uart.registers[134] = 0x00;
    uart.registers[135] = 0x00;
    const auto pos = dev.ReadPosition();
    CHECK(pos.ok());
    CHECK(pos.value == 2048);

    CHECK(dev.SetTorqueEnabled(true).ok());
    CHECK(uart.registers[64] == 1);
}

void TestUnknownModel() {
    FakeTransport uart;
    uart.model_number = 9999;
    uart.registers[0] = 0x0F;
    uart.registers[1] = 0x27;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    dynamixel::Device dev(bus, 1);
    const auto id = dev.Identify();
    CHECK(!id.ok());
    CHECK(id.error == dynamixel::DriverError::UnsupportedRequest);
    CHECK(id.value.model_number == 9999);
    CHECK(!dev.identified());
}

void TestBindXSeriesFallback() {
    FakeTransport uart;
    uart.model_number = 9999;
    uart.registers[0] = 0x0F;
    uart.registers[1] = 0x27;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    dynamixel::Device dev(bus, 1);
    CHECK(!dev.Identify().ok());
    dev.BindXSeriesFallback();
    CHECK(dev.identified());
    CHECK(dev.model()->model_number == 0);
    CHECK(dev.SetTorqueEnabled(false).ok());
}

void TestKnownLargerXSeries() {
    CHECK(dynamixel::FindModel(1020) == &dynamixel::kXm430W350);
    CHECK(dynamixel::kXm430W350.supports_current);
    CHECK(dynamixel::kXm430W350.effort_is_current);
    CHECK(!dynamixel::kXc430W150.supports_current);
    CHECK(dynamixel::FindModel(1070) == &dynamixel::kXc430W150);
}

void TestSyncWriteRead() {
    FakeTransport uart;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    const uint8_t ids[1] = {1};
    uint8_t payload[4] = {0x10, 0x00, 0x00, 0x00};
    const uint8_t* payloads[1] = {payload};
    CHECK(bus.SyncWrite(116, 4, ids, payloads, 1).ok());

    uint8_t back[4] = {0, 0, 0, 0};
    uint8_t* outs[1] = {back};
    dynamixel::SyncReadItem items[1];
    const auto sr = bus.SyncRead(116, 4, ids, 1, outs, items);
    CHECK(sr.ok());
    CHECK(sr.value >= 1);
    CHECK(items[0].valid);
}

void TestBroadcastWriteIsTxOnly() {
    FakeTransport uart;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    const uint8_t v = 0;
    CHECK(!bus.WriteRegister(dynamixel::kBroadcastId, 64, &v, 1,
                             dynamixel::WritePolicy::TxRx).ok());
    CHECK(bus.WriteRegister(dynamixel::kBroadcastId, 64, &v, 1,
                            dynamixel::WritePolicy::TxOnly).ok());
}

// Protocol 2.0 section 2.3: ID 0 is a valid device ID (0 to 252); 253 is not.
void TestScanIncludesIdZero() {
    FakeTransport uart;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    uint8_t found[4] = {0xAA, 0xAA, 0xAA, 0xAA};
    const auto scan = bus.Scan(found, 4, 0, 3);
    CHECK(scan.ok());
    CHECK(scan.value == 4);
    CHECK(found[0] == 0);
    CHECK(found[3] == 3);

    uint8_t first = 0xAA;
    const auto def = bus.Scan(&first, 1);  // default range starts at ID 0
    CHECK(def.ok());
    CHECK(def.value == 1);
    CHECK(first == 0);

    CHECK(!bus.Scan(found, 4, 0, 253).ok());
    CHECK(bus.Ping(0).ok());
    uint8_t mode = 0;
    CHECK(bus.ReadRegister(0, 11, &mode, 1).ok());
    CHECK(bus.ReadRegister(253, 11, &mode, 1).error == dynamixel::DriverError::InvalidId);

    const uint8_t ids[2] = {0, 1};
    uint8_t a[1] = {0};
    uint8_t b[1] = {0};
    uint8_t* outs[2] = {a, b};
    dynamixel::SyncReadItem items[2];
    const auto sr = bus.SyncRead(11, 1, ids, 2, outs, items);
    CHECK(sr.ok());
    CHECK(sr.value == 2);
    const uint8_t bad_ids[1] = {253};
    CHECK(bus.SyncRead(11, 1, bad_ids, 1, outs, items).error == dynamixel::DriverError::InvalidId);
    const uint8_t* ins[1] = {a};
    CHECK(bus.SyncWrite(64, 1, bad_ids, ins, 1).error == dynamixel::DriverError::InvalidId);
}

// Protocol 2.0 section 5.7: Reboot (0x08) clears a latched Hardware Error Status.
void TestRebootClearsHardwareError() {
    FakeTransport uart;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    dynamixel::Device dev(bus, 1);
    CHECK(dev.Identify().ok());
    uart.hardware_error = 0x04;  // overheating, latched
    const uint8_t on = 1;
    const auto wr = bus.WriteRegister(1, 64, &on, 1);
    CHECK(wr.error == dynamixel::DriverError::DeviceError);
    CHECK((wr.detail.device_error & dynamixel::kStatusAlertBit) != 0);

    const auto rb = dev.Reboot();
    CHECK(rb.ok());
    CHECK(uart.reboots == 1);
    CHECK(rb.detail.device_error == dynamixel::kStatusAlertBit);  // fault reported, then cleared
    CHECK(!uart.tx_log.empty() && hf_dxl_test::TxInstruction(uart.tx_log.back().data(),
                                                               uart.tx_log.back().size()) == 0x08);
    CHECK(bus.WriteRegister(1, 64, &on, 1).ok());

    // No reply: the servo may already be rebooting.
    uart.auto_reply = false;
    CHECK(bus.Reboot(1).error == dynamixel::DriverError::UncertainOutcome);
    uart.auto_reply = true;

    // A real error number (here Instruction Error, 0x02) is a failure.
    uart.next_error = 0x02;
    CHECK(bus.Reboot(1).error == dynamixel::DriverError::DeviceError);

    CHECK(bus.Reboot(253).error == dynamixel::DriverError::InvalidId);
    CHECK(bus.Reboot(dynamixel::kBroadcastId).ok());
    CHECK(uart.reboots == 3);  // the 0x02 reply above still rebooted in the fake
}

// X-series section 2.4 Operating Mode (11): only 0, 1, 3, 4, 5, 16 are defined.
void TestUnknownOperatingMode() {
    CHECK(dynamixel::OperatingModeFromRaw(3) == dynamixel::OperatingMode::Position);
    CHECK(dynamixel::OperatingModeFromRaw(16) == dynamixel::OperatingMode::Pwm);
    CHECK(dynamixel::OperatingModeFromRaw(0) == dynamixel::OperatingMode::Unknown);
    CHECK(dynamixel::OperatingModeFromRaw(5) == dynamixel::OperatingMode::Unknown);
    CHECK(dynamixel::OperatingModeFromRaw(2) == dynamixel::OperatingMode::Unknown);

    const uint8_t raws[3] = {2, 5, 200};
    for (const uint8_t raw : raws) {
        FakeTransport uart;
        uart.registers[11] = raw;
        dynamixel::Bus bus{dynamixel::TransportView{uart}};
        CHECK(bus.Open({}).ok());
        dynamixel::Device dev(bus, 1);
        CHECK(dev.operating_mode() == dynamixel::OperatingMode::Unknown);
        CHECK(dev.Identify().ok());
        CHECK(dev.operating_mode() == dynamixel::OperatingMode::Unknown);
        CHECK(dev.operating_mode_raw() == raw);
        const std::size_t sent = uart.tx_log.size();
        CHECK(dev.SetGoalPosition(0).error == dynamixel::DriverError::UnsupportedRequest);
        CHECK(dev.SetGoalVelocity(0).error == dynamixel::DriverError::UnsupportedRequest);
        CHECK(dev.SetGoalPwm(0).error == dynamixel::DriverError::UnsupportedRequest);
        CHECK(uart.tx_log.size() == sent);  // nothing sent
        CHECK(dev.SetOperatingMode(dynamixel::OperatingMode::Unknown).error ==
              dynamixel::DriverError::InvalidParameter);
        CHECK(dev.SetOperatingMode(dynamixel::OperatingMode::Position).ok());
        CHECK(dev.operating_mode() == dynamixel::OperatingMode::Position);
        CHECK(dev.operating_mode_raw() == 3);
    }
}

// PWM Control Mode is Operating Mode 16 (X-series section 2.4, Operating Mode (11)).
void TestGoalPwmChecksMode() {
    static dynamixel::ModelDescriptor no_pwm = dynamixel::kXSeriesFallback;
    no_pwm.supports_pwm = false;

    FakeTransport uart;
    uart.model_number = 9999;
    uart.registers[0] = 0x0F;
    uart.registers[1] = 0x27;
    uart.registers[11] = 16;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());

    dynamixel::Device unread(bus, 1);
    unread.BindModel(no_pwm);  // mode not read yet
    std::size_t sent = uart.tx_log.size();
    CHECK(unread.SetGoalPwm(100).error == dynamixel::DriverError::UnsupportedRequest);
    CHECK(uart.tx_log.size() == sent);

    dynamixel::Device dev(bus, 1);
    dev.BindModel(no_pwm);
    CHECK(dev.Identify().ok());
    CHECK(dev.operating_mode() == dynamixel::OperatingMode::Pwm);
    sent = uart.tx_log.size();
    CHECK(dev.SetGoalPwm(100).error == dynamixel::DriverError::UnsupportedRequest);
    CHECK(uart.tx_log.size() == sent);

    dynamixel::Device ok_dev(bus, 1);
    ok_dev.BindXSeriesFallback();
    CHECK(ok_dev.Identify().ok());
    CHECK(ok_dev.SetGoalPwm(100).ok());
    CHECK(uart.registers[100] == 100);
    uart.registers[11] = 3;
    CHECK(ok_dev.Identify().ok());
    CHECK(ok_dev.SetGoalPwm(100).error == dynamixel::DriverError::UnsupportedRequest);
}

void TestMaxPacketBytesBoundsPackets() {
    // Protocol 2.0 section 2.4: Length = parameters + 3, so an instruction is
    // 10 + parameters bytes and a status packet 11 + parameters bytes.
    {
        FakeTransport uart;
        dynamixel::Bus bus{dynamixel::TransportView{uart}};
        dynamixel::TransportConfig cfg{};
        cfg.max_packet_bytes = dynamixel::kMinPacketBytes - 1;  // a ping reply cannot fit
        CHECK(bus.Open(cfg).error == dynamixel::DriverError::InvalidParameter);
        CHECK(!bus.IsOpen());
    }

    FakeTransport uart;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    dynamixel::TransportConfig cfg{};
    cfg.max_packet_bytes = 64;
    CHECK(bus.Open(cfg).ok());
    uint8_t buf[300] = {};

    std::size_t sent = uart.tx_log.size();
    CHECK(bus.ReadRegister(1, 0, buf, 53).ok());  // 11 + 53 = 64
    CHECK(uart.tx_log.size() == sent + 1);
    sent = uart.tx_log.size();
    CHECK(bus.ReadRegister(1, 0, buf, 54).error == dynamixel::DriverError::BufferTooSmall);
    CHECK(uart.tx_log.size() == sent);

    CHECK(bus.WriteRegister(1, 150, buf, 52).ok());  // 12 + 52 = 64
    sent = uart.tx_log.size();
    CHECK(bus.WriteRegister(1, 150, buf, 53).error == dynamixel::DriverError::BufferTooSmall);
    CHECK(bus.WriteRegister(dynamixel::kBroadcastId, 150, buf, 53,
                            dynamixel::WritePolicy::TxOnly)
              .error == dynamixel::DriverError::BufferTooSmall);
    CHECK(uart.tx_log.size() == sent);

    // Sync write: 14 + count * (1 + length). Two servos, 24 bytes each = 64.
    const uint8_t ids[3] = {1, 2, 3};
    const uint8_t* payloads[3] = {buf, buf, buf};
    CHECK(bus.SyncWrite(150, 24, ids, payloads, 2).ok());
    sent = uart.tx_log.size();
    CHECK(bus.SyncWrite(150, 24, ids, payloads, 3).error == dynamixel::DriverError::BufferTooSmall);
    CHECK(uart.tx_log.size() == sent);

    // Sync read: each status is 11 + length.
    uint8_t a[64] = {};
    uint8_t* outs[1] = {a};
    dynamixel::SyncReadItem items[1];
    CHECK(bus.SyncRead(0, 54, ids, 1, outs, items).error == dynamixel::DriverError::BufferTooSmall);
    CHECK(uart.tx_log.size() == sent);
    CHECK(bus.SyncRead(0, 53, ids, 1, outs, items).ok());

    // The default (256) bounds a plain read at 245 bytes.
    FakeTransport uart2;
    dynamixel::Bus bus2{dynamixel::TransportView{uart2}};
    CHECK(bus2.Open({}).ok());
    CHECK(bus2.config().max_packet_bytes == dynamixel::kMaxPacketBytes);
    CHECK(bus2.ReadRegister(1, 0, buf, 245).ok());
    sent = uart2.tx_log.size();
    CHECK(bus2.ReadRegister(1, 0, buf, 246).error == dynamixel::DriverError::BufferTooSmall);
    CHECK(uart2.tx_log.size() == sent);
}

void SetTelemetryRegisters(FakeTransport& uart) {
    uart.registers[70] = 0x00;                                   // Hardware Error Status
    uart.registers[124] = 0x9C; uart.registers[125] = 0xFF;      // Present PWM -100
    uart.registers[126] = 0x2C; uart.registers[127] = 0x01;      // Present Load/Current 300
    uart.registers[128] = 0xF6; uart.registers[129] = 0xFF;      // Present Velocity -10
    uart.registers[130] = 0xFF; uart.registers[131] = 0xFF;
    uart.registers[132] = 0x00; uart.registers[133] = 0x08;      // Present Position 2048
    uart.registers[134] = 0x00; uart.registers[135] = 0x00;
    uart.registers[144] = 0x78; uart.registers[145] = 0x00;      // Present Input Voltage 12.0 V
    uart.registers[146] = 37;                                    // Present Temperature
}

void CheckTelemetryValues(const dynamixel::Telemetry& tel) {
    CHECK(tel.position == 2048);
    CHECK(tel.velocity == -10);
    CHECK(tel.load == 300);
    CHECK(tel.pwm == -100);
    CHECK(tel.voltage_0p1_v == 120);
    CHECK(tel.temperature_c == 37);
    CHECK(tel.hardware_error == 0);
}

void TestReadTelemetrySingleRead() {
    // X-series control table, RAM area: Present PWM (124) through Present
    // Temperature (146) is one contiguous range of registers.
    FakeTransport uart;
    SetTelemetryRegisters(uart);
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    CHECK(bus.Open({}).ok());
    dynamixel::Device dev(bus, 1);
    CHECK(dev.Identify().ok());

    const std::size_t sent = uart.tx_log.size();
    const auto tel = dev.ReadTelemetry();
    CHECK(tel.ok());
    CheckTelemetryValues(tel.value);
    // One Read of Present PWM (124) .. Present Temperature (146), 23 bytes,
    // and one of Hardware Error Status (70): two packets, not seven.
    CHECK(uart.tx_log.size() == sent + 2);
    if (uart.tx_log.size() == sent + 2) {
        const auto& block = uart.tx_log[sent];
        CHECK(block.size() == 14);
        CHECK(hf_dxl_test::TxInstruction(block.data(), block.size()) == 0x02);
        CHECK(block.size() >= 12 && (block[8] | (block[9] << 8)) == 124);
        CHECK(block.size() >= 12 && (block[10] | (block[11] << 8)) == 23);
        const auto& hw = uart.tx_log[sent + 1];
        CHECK(hw.size() >= 12 && (hw[8] | (hw[9] << 8)) == 70);
        CHECK(hw.size() >= 12 && (hw[10] | (hw[11] << 8)) == 1);
    }

    // When the block does not fit max_packet_bytes, the per-register reads
    // give the same values.
    FakeTransport small_uart;
    SetTelemetryRegisters(small_uart);
    dynamixel::Bus small_bus{dynamixel::TransportView{small_uart}};
    dynamixel::TransportConfig cfg{};
    cfg.max_packet_bytes = 32;  // the 34-byte block reply does not fit
    CHECK(small_bus.Open(cfg).ok());
    dynamixel::Device small_dev(small_bus, 1);
    CHECK(small_dev.Identify().ok());
    const auto tel2 = small_dev.ReadTelemetry();
    CHECK(tel2.ok());
    CheckTelemetryValues(tel2.value);

    // A latched hardware fault sets the Alert bit in every status packet, so
    // the block read fails and ReadTelemetry() fails as before.
    uart.hardware_error = 0x20;
    CHECK(dev.ReadTelemetry().error == dynamixel::DriverError::DeviceError);
    uart.hardware_error = 0;

    // No reply: the call still fails with the read's error.
    uart.auto_reply = false;
    CHECK(dev.ReadTelemetry().error == dynamixel::DriverError::Timeout);
}

}  // namespace

int main() {
    TestPingAndRead();
    TestTimeout();
    TestCorruptChecksum();
    TestWrongId();
    TestEchoMatchAndMismatch();
    TestPartialWriteAndDrain();
    TestFragmentedRx();
    TestDeviceTypedApi();
    TestUnknownModel();
    TestBindXSeriesFallback();
    TestKnownLargerXSeries();
    TestSyncWriteRead();
    TestBroadcastWriteIsTxOnly();
    TestScanIncludesIdZero();
    TestRebootClearsHardwareError();
    TestUnknownOperatingMode();
    TestGoalPwmChecksMode();
    TestMaxPacketBytesBoundsPackets();
    TestReadTelemetrySingleRead();
    if (g_failures != 0) {
        std::printf("%d check(s) failed\n", g_failures);
        return 1;
    }
    std::printf("hf-dynamixel host tests passed\n");
    return 0;
}
