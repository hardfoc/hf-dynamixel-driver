# Testing and CI

How the driver is checked: host tests that run the real ROBOTIS SDK against a scripted servo, compiled
documentation, ESP32 builds and lint, on every pull request.

## Host tests

The tests build the driver and the SDK for the host and drive them through a fake transport that answers like an
X-series servo: it decodes each instruction packet, keeps a register file and replies with a correct status
packet, or with the fault a test asks for (silence, a bad CRC, a wrong ID, a partial write, a fragmented reply, a
local echo). Time is virtual, so timeouts cost nothing.

```bash
git submodule update --init external/DynamixelSDK
cmake -S . -B build-host -DHF_DYNAMIXEL_BUILD_TESTS=ON -DHF_DYNAMIXEL_BUILD_HOST_EXAMPLE=ON
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

The test binary is built with AddressSanitizer and UBSan on GCC and Clang (`HF_DYNAMIXEL_ENABLE_SANITIZERS`).

| File | Covers |
| --- | --- |
| [`tests/host/dynamixel_driver_test.cpp`](../tests/host/dynamixel_driver_test.cpp) | Ping and read, timeout, bad CRC, wrong ID, echo, partial writes, fragmented replies, typed `Device` calls, unknown and fallback models, sync read and write, broadcast writes, ID 0 scans, Reboot, unknown Operating Mode values, `SetGoalPwm` checks, `max_packet_bytes` limits, the one-block `ReadTelemetry()` |
| [`tests/host/fake_transport.hpp`](../tests/host/fake_transport.hpp) | The scripted servo |
| [`tests/host/protocol2_status.hpp`](../tests/host/protocol2_status.hpp) | Status packets and CRC for the fake |
| [`examples/host/ping_read.cpp`](../examples/host/ping_read.cpp) | A ping and a position read over the fake, run by CI |

## Documentation snippets

Every C++ block on this site is included from [`tests/docs/`](../tests/docs/my_uart.hpp), which the host build
compiles (target `hf_dynamixel_doc_snippets`, `-Wall -Wextra -Wpedantic`) with `Transport<MyUart>` instantiated.
A change to the API that breaks a page breaks the build first.

## CI workflows

| Workflow | What it checks |
| --- | --- |
| Host tests | Build, tests, the host example, the doc snippets, an install into a prefix, and a `find_package()` consumer of it ([`tests/install/`](../tests/install/CMakeLists.txt)) |
| ESP32 Examples Build CI | Every app in `examples/esp32/app_config.yml`, for each listed ESP-IDF version and build type |
| C++ Lint CI | clang-format and clang-tidy |
| Docs | Strict site build, figure and generated-table drift, Doxygen, link check, Lighthouse budget |
| Markdown lint, YAML lint, link check | Repository text files |

The workflows call the shared, versioned workflows of
[hf-general-ci-tools](https://github.com/hardfoc/hf-general-ci-tools) and
[hf-espidf-ci-tools](https://github.com/hardfoc/hf-espidf-ci-tools).

## On hardware

Hardware runs are not part of CI. The gate before a release: `dynamixel_readonly_example` for 1000 reads at
57600 bit/s with no unexplained errors, then unplug and noisy-cable cases. Record the board, wiring, supply,
baud rate, servo firmware, SDK commit and compiler with the result.
