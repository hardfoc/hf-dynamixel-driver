# ESP32 examples

Native ESP-IDF adapters live in `main/include/hf_dynamixel_esp_uart.hpp`.
They are **not** part of the portable library. Build **only** through the
`scripts/` submodule (`hf-espidf-project-tools`).

## Wiring (transistor / TTL bring-up)

| Chip | UART | TX | RX | Baud |
|------|------|----|----|------|
| ESP32-C6 | UART1 | GPIO4 | GPIO5 | 57600 8N1 |
| ESP32-S3 | UART1 | GPIO17 | GPIO18 | 57600 8N1 |

Optional direction GPIO for a buffered interface
(`CONFIG_HF_DYNAMIXEL_UART_DIR_GPIO`). Do not treat a transistor board as a
long-cable or high-rate reference.

## Build

```bash
git submodule update --init --recursive
cd examples/esp32
./scripts/build_app.sh list
./scripts/build_app.sh dynamixel_readonly_example Debug
CONFIG_TARGET=esp32s3 ./scripts/build_app.sh dynamixel_readonly_example Debug
./scripts/build_app.sh dynamixel_scan_example Debug
./scripts/build_app.sh dynamixel_motion_example Debug
```

- `dynamixel_readonly_example` never enables torque.
- `dynamixel_scan_example` never enables torque; use it when swapping in a
  larger X-series.
- `dynamixel_motion_example` reads present position, sets a small nearby
  goal, then enables torque.

Pins, baud, servo ID, echo, and X-series fallback are under
`idf.py menuconfig` → **HF Dynamixel Example**. Full walkthrough:
[docs/esp32_examples.md](../../docs/esp32_examples.md).
