---
layout: default
title: "Installation"
description: "CMake, submodules, and hf::dynamixel export"
parent: "Documentation"
nav_order: 1
permalink: /docs/installation/
---

# Installation

C++17, CMake 3.16+. The library is **static** (SDK sources + HF wrapper).
This repository does **not** depend on hf-core.

```bash
git clone --recurse-submodules https://github.com/hardfoc/hf-dynamixel-driver.git
cd hf-dynamixel-driver
# or, if already cloned:
git submodule update --init --recursive

cmake -S . -B build -DHF_DYNAMIXEL_BUILD_TESTS=ON -DHF_DYNAMIXEL_BUILD_HOST_EXAMPLE=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Required submodules:

| Path | Remote | Role |
|------|--------|------|
| `external/DynamixelSDK` | ROBOTIS-GIT/DynamixelSDK | Protocol 2.0 engine, pin **4.1.0** |
| `examples/esp32/scripts` | hardfoc/hf-espidf-project-tools | `build_app.sh`, matrix, flash helpers |

## Consume from another CMake project

```cmake
add_subdirectory(external/hf-dynamixel-driver)
target_link_libraries(my_firmware PRIVATE hf::dynamixel)
```

Or include `cmake/hf_dynamixel_build_settings.cmake` and compile
`${HF_DYNAMIXEL_SOURCE_FILES}` yourself (ESP-IDF component style).

Firmware must contain **one** Dynamixel SDK copy. Do not also link a second
upstream `dynamixel_sdk` target into the same image.

## Install / export

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/tmp/hf-dynamixel
cmake --build build --target install
```

The exported package is `hf::dynamixel`. Public headers never include
`dynamixel_sdk/*`.

## ESP-IDF

See [ESP32 examples](esp32_examples.md). The example component
`examples/esp32/components/hf_dynamixel` includes the same build settings
and defines `__linux__` on the SDK translation units so their include
guards resolve on Xtensa / RISC-V.

## hf-core

When aggregated through hf-core, enable `HF_CORE_ENABLE_DYNAMIXEL` (default
**OFF**; ESP32 core examples turn it **ON**). That pulls the driver sources
and `DynamixelHandler`. See [hf-core handler](hf_core_handler.md).
