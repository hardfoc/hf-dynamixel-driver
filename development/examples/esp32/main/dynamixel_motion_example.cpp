// =============================================================================
// HF-Dynamixel — small unloaded motion example
// =============================================================================
// Reads present position, programs a nearby goal with a slow profile, then
// enables torque. Never jumps to an implicit zero. Works for XC430-W150 and
// any larger X-series that shares the Protocol 2.0 position table (or the
// fallback bind).
// =============================================================================

#include "hf_dynamixel_example_app.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdlib>

namespace {

constexpr const char* TAG = "DxlMotion";
using hf_dynamixel_examples::ExampleUart;

}  // namespace

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "hf-dynamixel motion %s target %s", dynamixel::GetDriverVersion(),
             CONFIG_IDF_TARGET);
    ESP_ERROR_CHECK(ExampleUart::Install());

    ExampleUart uart;
    dynamixel::Bus bus{dynamixel::TransportView{uart}};
    if (!hf_dynamixel_examples::OpenExampleBus(bus, TAG)) {
        return;
    }

    dynamixel::Device dev(bus, static_cast<uint8_t>(CONFIG_HF_DYNAMIXEL_SERVO_ID));
    if (!hf_dynamixel_examples::IdentifyOrBind(dev, TAG).ok()) {
        return;
    }

    const auto pos = dev.ReadPosition();
    if (!pos.ok()) {
        ESP_LOGE(TAG, "read position failed: %s", dynamixel::ToString(pos.error).data());
        return;
    }
    const int32_t start = pos.value;
    const int32_t goal = start + 64;
    ESP_LOGI(TAG, "start=%d goal=%d (small step, unloaded)", start, goal);

    if (!dev.SetOperatingMode(dynamixel::OperatingMode::Position).ok()) {
        ESP_LOGW(TAG, "operating mode write failed (torque may already be on)");
    }
    if (!dev.SetMotionProfile(10, 50).ok()) {
        ESP_LOGE(TAG, "profile failed");
        return;
    }
    if (!dev.SetGoalPosition(goal).ok()) {
        ESP_LOGE(TAG, "goal failed");
        return;
    }
    if (!dev.SetTorqueEnabled(true).ok()) {
        ESP_LOGE(TAG, "torque enable failed");
        return;
    }

    for (int i = 0; i < 50; ++i) {
        const auto now = dev.ReadPosition();
        if (now.ok()) {
            ESP_LOGI(TAG, "position %d err %d", now.value, std::abs(now.value - goal));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    (void)dev.SetTorqueEnabled(false);
    ESP_LOGI(TAG, "torque disabled");
}
