---
layout: default
title: "ESP32 examples"
description: "Build and flash with hf-espidf-project-tools on ESP32-C6 or ESP32-S3"
parent: "Documentation"
nav_order: 3
permalink: /docs/esp32_examples/
---

# ESP32 examples

The examples live in `examples/esp32/` and **must** be built with the
[hf-espidf-project-tools](https://github.com/hardfoc/hf-espidf-project-tools)
scripts checked out as `examples/esp32/scripts`. Direct `idf.py` from the
repo root is not the supported path.

## Apps

| App | Source | Torque | Purpose |
|-----|--------|--------|---------|
| `dynamixel_readonly_example` | `dynamixel_readonly_example.cpp` | Never | Scan, identify, stream telemetry |
| `dynamixel_scan_example` | `dynamixel_scan_example.cpp` | Never | Dump every ID and catalog vs fallback |
| `dynamixel_motion_example` | `dynamixel_motion_example.cpp` | After a small nearby goal | Unloaded XC430 / X-series step |

`app_config.yml` lists those names. `./scripts/build_app.sh list` prints them.

## Targets

CI and `metadata.target` default to **esp32c6**. The same sources build for
**esp32s3** because pins come from Kconfig / `sdkconfig.defaults.<target>`:

| Chip | UART | TX | RX | DIR | Baud |
|------|------|----|----|-----|------|
| ESP32-C6 | UART1 | GPIO4 | GPIO5 | unused (`-1`) | 57600 |
| ESP32-S3 | UART1 | GPIO17 | GPIO18 | unused (`-1`) | 57600 |

The scripts read the chip from `get_target()` (the YAML `metadata.target`)
unless you override it:

```bash
cd examples/esp32

# ESP32-C6 (default, what CI builds)
./scripts/build_app.sh dynamixel_readonly_example Debug

# ESP32-S3 — same script, different target folder
CONFIG_TARGET=esp32s3 ./scripts/build_app.sh dynamixel_readonly_example Debug

# Other apps
./scripts/build_app.sh dynamixel_scan_example Debug
./scripts/build_app.sh dynamixel_motion_example Debug
```

`CONFIG_TARGET` is the supported override in `build_app.sh`. It sets
`IDF_TARGET` and the build directory pattern
`build-app-{app}-type-{type}-target-{target}-idf-{idf}`.

Flash and monitor with the matching script helpers (`flash_app.sh`,
`monitor_app.sh`) from the same `scripts/` tree.

## Kconfig

`idf.py menuconfig` → **HF Dynamixel Example**:

- TX / RX / optional DIR GPIO
- baud, preferred servo ID, scan last ID
- local-echo (single-wire open-drain)
- bind X-series fallback when the model number is unknown

Defaults are chip-specific (`sdkconfig.defaults.esp32c6` /
`sdkconfig.defaults.esp32s3`).

## First hardware session (XC430-W150)

1. 12 V (or the servo’s rated supply) on the Dynamixel power pins. Logic
   is 3.3 V TTL toward the ESP32.
2. Common ground. Short cable. 57600 8N1.
3. Flash `dynamixel_readonly_example`. Confirm ping model **1070**.
4. Leave torque off until a 1000-read session is clean.
5. Only then run `dynamixel_motion_example` unloaded, small step from
   *present* position (never an implicit zero).

## Later larger servo

1. Run `dynamixel_scan_example`. Note the ping model number and firmware.
2. If it is XM430-W350 (1020) or another catalog entry, typed APIs bind
   automatically.
3. If it is not in the table yet, keep
   `CONFIG_HF_DYNAMIXEL_BIND_XSERIES_FALLBACK=y` and use the shared
   X-series addresses. Then add a named `ModelDescriptor` when you decide
   the part.

See [model support](model_support.md) and [wiring](wiring.md).
