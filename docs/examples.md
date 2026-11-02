# Examples

Complete ESP32 applications that exercise the driver on an X-series servo, from a read-only session to a small
move.

They run on an ESP32-C6 (CI's target) or an ESP32-S3, UART1, wired as on the [Hardware](hardware.md#example-boards)
page; the transport is [`hf_dynamixel_esp_uart.hpp`](../examples/esp32/main/include/hf_dynamixel_esp_uart.hpp).
CI builds every app below on each pull request.

## The apps

<!-- generated: examples -->
| App | What it shows | Source | CI |
| --- | --- | --- | --- |
| `dynamixel_readonly_example` :material-star:{ title="featured" } | Scan, identify and stream telemetry; never enables torque | [`dynamixel_readonly_example.cpp`](../examples/esp32/main/dynamixel_readonly_example.cpp) | yes |
| `dynamixel_scan_example` :material-star:{ title="featured" } | List every servo with its model number and catalog entry or fallback | [`dynamixel_scan_example.cpp`](../examples/esp32/main/dynamixel_scan_example.cpp) | yes |
| `dynamixel_motion_example` | Small unloaded move from the present position, then torque off | [`dynamixel_motion_example.cpp`](../examples/esp32/main/dynamixel_motion_example.cpp) | yes |

Build and flash one (from `examples/esp32`, ESP-IDF environment loaded):

```bash
./scripts/build_app.sh dynamixel_readonly_example Release
./scripts/flash_app.sh flash_monitor dynamixel_readonly_example Release
```

Source of this table: [`examples/esp32/app_config.yml`](../examples/esp32/app_config.yml).
<!-- /generated -->

## Set up once

```bash
git submodule update --init --recursive              # the SDK and the HardFOC ESP-IDF build scripts
cd examples/esp32
. $IDF_PATH/export.sh                                # ESP-IDF v5.5
./scripts/build_app.sh list                          # every app and build type
```

The scripts read [`app_config.yml`](../examples/esp32/app_config.yml) (the same file this page's table comes
from) and build each app as its own firmware image. Calling `idf.py` directly stops with
`APP_TYPE not defined`.

For an ESP32-S3, set the target for the scripts:

```bash
CONFIG_TARGET=esp32s3 ./scripts/build_app.sh dynamixel_readonly_example Debug
```

The log line `uart1 tx=17 rx=18` confirms the S3 pins.

## First session on a servo

1. Power the servo from its own supply, join the grounds, 57600 bit/s, one servo, short cable.
2. Flash `dynamixel_readonly_example`. It scans, identifies and streams telemetry; it never enables torque.
3. Leave it running for 1000 reads with no unexplained errors.
4. Only then run `dynamixel_motion_example`, unloaded. It reads the present position, sets a goal 64 ticks
   (about 5.6°) from it, enables torque, then disables it again.

## What to read in an example

| You want to | Read |
| --- | --- |
| Bring a bus up and read telemetry safely | `dynamixel_readonly_example.cpp` |
| List every servo with its model and catalog entry | `dynamixel_scan_example.cpp` |
| Make a first move from the present position | `dynamixel_motion_example.cpp` |
| Bind the fallback for an unlisted model | `IdentifyOrBind()` in `main/include/hf_dynamixel_example_app.hpp` |
| Write an ESP-IDF transport | `main/include/hf_dynamixel_esp_uart.hpp` |

The [example folder's README](../examples/esp32/README.md) has the build commands and the wiring notes.
