# Changelog

All notable changes to this driver are listed here. Versions follow
[Semantic Versioning](https://semver.org/).

## Unreleased

- Documentation moved to the HardFOC MkDocs platform: rewritten pages, the code on the site compiled by the host
  build from `tests/docs/`, a layer figure drawn from the headers, the Doxygen reference under `api/`. The
  Jekyll site and its files are gone.
- README rewritten to the HardFOC README standard.

### Added

- `Bus::Reboot(id)` and `Device::Reboot()`: the Protocol 2.0 Reboot instruction (0x08, e-Manual section 5.7),
  the software way to clear a latched Hardware Error Status. An Alert-only status reply counts as success; no
  reply is `UncertainOutcome`. New `kStatusAlertBit` and `kStatusErrorNumberMask` (section 3.2).

### Fixed

- `Device::ReadTelemetry()` reads Present PWM (124) to Present Temperature (146), contiguous in the X-series RAM
  control table, in one Read instead of six, then Hardware Error Status (70): two packets instead of seven, same
  values. It falls back to per-register reads when the block does not fit `max_packet_bytes` or the servo refuses
  the range. The returned `detail` is now the block read's.

- `TransportConfig::max_packet_bytes` now bounds packet sizes; before, nothing read it. `ReadRegister()`,
  `WriteRegister()`, `SyncRead()` and `SyncWrite()` return `BufferTooSmall` without sending when the instruction
  or status packet (10 or 11 bytes plus parameters, e-Manual Protocol 2.0 section 2.4) would be longer.
  `Bus::Open()` returns `InvalidParameter` below `kMinPacketBytes` (14) and caps the limit at
  `kSdkMaxPacketBytes` (1024, the SDK's buffers). New `InstructionPacketBytes()` and `StatusPacketBytes()`.

- `Device::SetGoalPwm()` runs the same checks as the other goal setters: it needs a model whose descriptor
  supports PWM Control Mode (Operating Mode 16) as well as a cached mode of PWM, and refuses before sending.

- `Device::Identify()` no longer casts an Operating Mode register value without a typed mode (0, 5 or an
  undefined value; X-series e-Manual section 2.4, Operating Mode (11)) into `OperatingMode`. It maps it to the
  new `OperatingMode::Unknown`, which every typed goal refuses. New `OperatingModeFromRaw()`,
  `Device::operating_mode()` and `Device::operating_mode_raw()`; `SetOperatingMode(Unknown)` returns
  `InvalidParameter`.

- `Bus::Scan()` accepts ID 0, and its default range now starts at 0: Protocol 2.0 device IDs are 0 to 252
  (e-Manual, Protocol 2.0, section 2.3). `ReadRegister()` returns `InvalidId` (was `InvalidParameter`) for an
  ID above 252, and `SyncRead()` / `SyncWrite()` now reject such IDs with `InvalidId`. New `IsDeviceId()` and
  `kMinDeviceId`.

### Breaking changes

- With the default `max_packet_bytes` of 256, a single read above 245 bytes, a write above 244 bytes or a sync
  write above 242 bytes of IDs and data now returns `BufferTooSmall` instead of being sent. Raise
  `max_packet_bytes` (up to 1024) for longer transfers. `Bus::Open()` fails for a `max_packet_bytes` below 14.
- `Bus::ReadRegister()` returns `InvalidId` (was `InvalidParameter`) for an ID above 252; `SyncRead()` and
  `SyncWrite()` reject such IDs.
- `Bus::Scan()` with default arguments now pings ID 0 too (253 IDs instead of 252).

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
