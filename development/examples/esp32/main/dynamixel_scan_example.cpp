// =============================================================================
// HF-Dynamixel — bus scan + identity dump
// =============================================================================
// Walks IDs 1..SCAN_LAST, pings each responder, and prints catalog vs
// fallback binding. Read-only. Use this when swapping XC430 for a larger
// X-series whose model number is not yet in the table.
// =============================================================================

#include "hf_dynamixel_example_app.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace {

constexpr const char* TAG = "DxlScan";
using hf_dynamixel_examples::ExampleUart;

}  // namespace

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "hf-dynamixel scan %s target %s", dynamixel::GetDriverVersion(),
             CONFIG_IDF_TARGET);
    ESP_ERROR_CHECK(ExampleUart::Install());

    ExampleUart uart;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    if (!hf_dynamixel_examples::OpenExampleBus(bus, TAG)) {
        return;
    }

    uint8_t ids[32]{};
    const auto scan =
        bus.Scan(ids, 32, 1, static_cast<uint8_t>(CONFIG_HF_DYNAMIXEL_SCAN_LAST_ID));
    if (!scan.ok() || scan.value == 0) {
        ESP_LOGW(TAG, "no devices on 1..%d: %s", CONFIG_HF_DYNAMIXEL_SCAN_LAST_ID,
                 scan.ok() ? "empty" : dynamixel::ToString(scan.error).data());
        return;
    }

    ESP_LOGI(TAG, "found %u device(s)", scan.value);
    for (uint8_t i = 0; i < scan.value; ++i) {
        dynamixel::Device dev(bus, ids[i]);
        const auto ping = hf_dynamixel_examples::IdentifyOrBind(dev, TAG);
        if (!ping.ok()) {
            continue;
        }
        const dynamixel::ModelDescriptor* model = dev.model();
        ESP_LOGI(TAG, "  id=%u catalog=%s current=%s effort=%s pos_reg=%u",
                 ids[i], (model != nullptr) ? model->name : "none",
                 (model != nullptr && model->supports_current) ? "yes" : "no",
                 (model != nullptr && model->effort_is_current) ? "current" : "load",
                 (model != nullptr) ? model->present_position_reg.address : 0);
        if (dev.identified()) {
            const auto tel = dev.ReadTelemetry();
            if (tel.ok()) {
                ESP_LOGI(TAG, "  pos=%d temp=%uC volt=%.1fV", tel.value.position,
                         tel.value.temperature_c, tel.value.voltage_0p1_v / 10.0f);
            }
        }
    }

    ESP_LOGI(TAG, "scan complete");
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
