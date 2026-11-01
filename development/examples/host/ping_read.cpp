#include "dynamixel.hpp"
#include "../../tests/host/fake_transport.hpp"

#include <cstdio>

int main() {
    hf_dxl_test::FakeTransport uart;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    const auto opened = bus.Open({});
    if (!opened.ok()) {
        std::printf("open failed: %s\n", dynamixel::ToString(opened.error).data());
        return 1;
    }
    const auto ping = bus.Ping(1);
    if (!ping.ok()) {
        std::printf("ping failed: %s comm=%d\n", dynamixel::ToString(ping.error).data(),
                    ping.detail.comm_code);
        return 1;
    }
    std::printf("ping id=%u model=%u fw=%u driver=%s sdk=%s\n", ping.value.id,
                ping.value.model_number, ping.value.firmware_version,
                dynamixel::GetDriverVersion(), dynamixel::GetSdkCommit());

    uint8_t pos[4] = {0, 0, 0, 0};
    uart.registers[132] = 0x00;
    uart.registers[133] = 0x08;
    const auto rd = bus.ReadRegister(1, 132, pos, 4);
    if (!rd.ok()) {
        std::printf("read failed: %s\n", dynamixel::ToString(rd.error).data());
        return 1;
    }
    const int32_t ticks = static_cast<int32_t>(
        pos[0] | (pos[1] << 8) | (pos[2] << 16) | (pos[3] << 24));
    std::printf("present position %d\n", ticks);
    return 0;
}
