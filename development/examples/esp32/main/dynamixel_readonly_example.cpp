// =============================================================================
// HF-Dynamixel — read-only ESP-IDF discovery example
// =============================================================================
// Builds for ESP32-C6 (default) and ESP32-S3 via the project scripts:
//   ./scripts/build_app.sh dynamixel_readonly_example Debug
//   CONFIG_TARGET=esp32s3 ./scripts/build_app.sh dynamixel_readonly_example Debug
//
// Never enables torque. Safe for XC430-W150 and a later larger X-series.
// =============================================================================

#include "hf_dynamixel_example_app.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace {

constexpr const char* TAG = "DxlRo";
using hf_dynamixel_examples::ExampleUart;

}  // namespace

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "hf-dynamixel %s sdk %s (%s) target %s",
             dynamixel::GetDriverVersion(), dynamixel::GetSdkTag(),
             dynamixel::GetSdkCommit(), CONFIG_IDF_TARGET);
    ESP_ERROR_CHECK(ExampleUart::Install());

    ExampleUart uart;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    if (!hf_dynamixel_examples::OpenExampleBus(bus, TAG)) {
        return;
    }

    uint8_t ids[16]{};
    const auto scan = bus.Scan(ids, 16, 1, CONFIG_HF_DYNAMIXEL_SCAN_LAST_ID);
    ESP_LOGI(TAG, "scan 1..%d found %u device(s)", CONFIG_HF_DYNAMIXEL_SCAN_LAST_ID,
             scan.ok() ? scan.value : 0);

    const uint8_t id = (scan.ok() && scan.value > 0)
                           ? ids[0]
                           : static_cast<uint8_t>(CONFIG_HF_DYNAMIXEL_SERVO_ID);
    dynamixel::Device dev(bus, id);
    if (!hf_dynamixel_examples::IdentifyOrBind(dev, TAG).ok()) {
        return;
    }

    const uint16_t pos_addr = (dev.model() != nullptr)
                                  ? dev.model()->present_position_reg.address
                                  : dynamixel::kXc430W150.present_position_reg.address;

    while (true) {
        if (dev.identified()) {
            const auto tel = dev.ReadTelemetry();
            if (tel.ok()) {
                ESP_LOGI(TAG,
                         "pos=%d vel=%d effort=%d pwm=%d volt=%.1fV temp=%uC hw=0x%02x",
                         tel.value.position, tel.value.velocity, tel.value.load,
                         tel.value.pwm, tel.value.voltage_0p1_v / 10.0f,
                         tel.value.temperature_c, tel.value.hardware_error);
            } else {
                ESP_LOGW(TAG, "telemetry failed: %s comm=%d",
                         dynamixel::ToString(tel.error).data(), tel.detail.comm_code);
            }
        } else {
            uint8_t raw[4] = {0, 0, 0, 0};
            const auto rd = bus.ReadRegister(id, pos_addr, raw, 4);
            if (!rd.ok()) {
                ESP_LOGW(TAG, "read failed: %s comm=%d",
                         dynamixel::ToString(rd.error).data(), rd.detail.comm_code);
            } else {
                const int32_t ticks = static_cast<int32_t>(
                    raw[0] | (raw[1] << 8) | (raw[2] << 16) | (raw[3] << 24));
                ESP_LOGI(TAG, "present position %d", ticks);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
