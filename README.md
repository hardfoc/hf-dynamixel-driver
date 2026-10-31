---
layout: home
title: "HardFOC Dynamixel Driver"
nav_order: 1
description: "Protocol 2.0 Dynamixel wrapper around ROBOTIS SDK 4.1.0"
permalink: /
---

# hf-dynamixel-driver

[![Host tests](https://github.com/hardfoc/hf-dynamixel-driver/actions/workflows/host-tests.yml/badge.svg)](https://github.com/hardfoc/hf-dynamixel-driver/actions/workflows/host-tests.yml)
[![ESP32 examples](https://github.com/hardfoc/hf-dynamixel-driver/actions/workflows/esp32-examples-build-ci.yml/badge.svg)](https://github.com/hardfoc/hf-dynamixel-driver/actions/workflows/esp32-examples-build-ci.yml)
[![Docs](https://github.com/hardfoc/hf-dynamixel-driver/actions/workflows/ci-docs-publish.yml/badge.svg)](https://hardfoc.github.io/hf-dynamixel-driver/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Protocol 2.0 Dynamixel driver for HardFOC. The official ROBOTIS Dynamixel SDK
is the packet engine. HF owns the transport contract, result model, X-series
descriptors, and tests.

**Docs:** [https://hardfoc.github.io/hf-dynamixel-driver/](https://hardfoc.github.io/hf-dynamixel-driver/)

```cmake
add_subdirectory(hf-dynamixel-driver)
target_link_libraries(app PRIVATE hf::dynamixel)
```

```cpp
MyUart uart;  // implements dynamixel::Transport<MyUart>
dynamixel::Bus bus{dynamixel::TransportView{uart}};
bus.Open({.baud_rate = 57600});
auto ping = bus.Ping(1);
dynamixel::Device servo(bus, 1);
if (!servo.Identify().ok()) {
    servo.BindXSeriesFallback();  // later larger X-series
}
```

- C++17, no exceptions in HF code
- Public headers never include `dynamixel_sdk/*`
- First device: **XC430-W150** (model 1070). Current control is not a capability.
- Catalog also lists XC430-W240, XL430-W250, XM430-W210/W350, XH430-W210/W350
- Standalone: this repo does not depend on hf-core or hf-internal-interface-wrap
- ESP32-C6 and ESP32-S3 examples build with `examples/esp32/scripts/build_app.sh`

SDK pin: **4.1.0** / `f838bc90f72fcf5b9c279432d6e12fc24969daa8`.
See [docs/sdk_integration.md](docs/sdk_integration.md).

```bash
git submodule update --init --recursive
cmake -S . -B build -DHF_DYNAMIXEL_BUILD_TESTS=ON
cmake --build build && ctest --test-dir build

cd examples/esp32
./scripts/build_app.sh dynamixel_readonly_example Debug
CONFIG_TARGET=esp32s3 ./scripts/build_app.sh dynamixel_readonly_example Debug
```
