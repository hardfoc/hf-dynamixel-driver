# Changelog

## 0.4.0

- X-series catalog (XC430, XL430, XM430, XH430) plus `BindModel` /
  `BindXSeriesFallback` for a later larger servo.
- ESP32-C6 and ESP32-S3 examples share one source tree: Kconfig pins,
  `sdkconfig.defaults.<target>`, `CONFIG_TARGET=esp32s3` for the scripts.
- Added `dynamixel_scan_example`. Read-only and motion examples stream
  telemetry / bind fallback.
- Jekyll + Doxygen GitHub Pages (`_config/`, `ci-docs-publish.yml`).
- hf-core aggregation: `HF_CORE_ENABLE_DYNAMIXEL` and `DynamixelHandler`
  are documented here and implemented in hf-core.

## 0.3.0

- Pin DynamixelSDK 4.1.0 and wrap Protocol 2.0 through a private PortHandler adapter.
- Public CRTP transport, Bus/Device API, XC430-W150 descriptor, sync read/write.
- Host tests drive the real protocol engine through a scripted transport.
- ESP-IDF UART example adapter (C6 bring-up, S3 compile target).
