# Installation

Add the driver to a CMake or ESP-IDF project. It is a static C++17 library that compiles the ROBOTIS DYNAMIXEL SDK
sources it needs; nothing else.

## Requirements

- A C++17 compiler. CI builds with GCC on Ubuntu and with the ESP-IDF v5.5 toolchains (ESP32-C6).
- CMake 3.16 or newer.
- A heap: the bus allocates its state once at construction, and the SDK allocates packet buffers on every call
  ([SDK integration](sdk-integration.md)).
- No RTOS and no exceptions: every call is `noexcept` and returns a `DriverResult`.

## Get the source

A git submodule keeps the driver pinned to a version you chose. The driver has its own submodule, the SDK:

```bash
git submodule add https://github.com/hardfoc/hf-dynamixel-driver.git external/hf-dynamixel-driver
git submodule update --init --recursive external/hf-dynamixel-driver
```

| Submodule | Source | Needed for |
| --- | --- | --- |
| `external/DynamixelSDK` | ROBOTIS-GIT/DynamixelSDK, tag 4.1.0 | Every build |
| `examples/esp32/scripts` | hardfoc/hf-espidf-project-tools | The ESP32 examples only |

## CMake

```cmake
add_subdirectory(external/hf-dynamixel-driver)
target_link_libraries(my_app PRIVATE hf::dynamixel)
```

The `hf::dynamixel` target (`hf_dynamixel`) carries the include directories, the generated version header and
`cxx_std_17`. The SDK headers stay private: your code never includes them.

| Option | Default | Effect |
| --- | --- | --- |
| `HF_DYNAMIXEL_ENABLE_WARNINGS` | `ON` | `-Wall -Wextra -Wpedantic` |
| `HF_DYNAMIXEL_BUILD_TESTS` | `OFF` | Host tests, doc snippets ([Testing](testing.md)) |
| `HF_DYNAMIXEL_BUILD_HOST_EXAMPLE` | `OFF` | `examples/host/ping_read.cpp` |
| `HF_DYNAMIXEL_ENABLE_SANITIZERS` | `ON` | ASan and UBSan on the host tests |

Link one copy of the SDK per firmware image. Do not also link a separate upstream `dynamixel_sdk` target.

`cmake --install` exports the same target as `hf::dynamixel` (`lib/cmake/hf_dynamixel`) with the public headers.

## ESP-IDF

The ESP-IDF component wrapper is
[`components/hf_dynamixel/CMakeLists.txt`](../components/hf_dynamixel/CMakeLists.txt) (the examples use the copy in
[`examples/esp32/components/hf_dynamixel/`](../examples/esp32/components/hf_dynamixel/CMakeLists.txt)). Point
`EXTRA_COMPONENT_DIRS` at it, or copy it into your project's `components/`, and set the `HF_DYNAMIXEL_ROOT` cache
variable to the driver checkout. It requires the `driver` and `esp_timer` components. Your component then declares
`REQUIRES hf_dynamixel`.

The SDK sources only include their headers when `__linux__`, `__APPLE__`, `WIN32` or `ARDUINO` is defined, so the
build settings define `__linux__` for the library's own sources on any toolchain other than Linux, macOS or
Windows. The define is private: your code never sees it ([SDK integration](sdk-integration.md)).

## Inside hf-core

[hf-core](https://github.com/hardfoc/hf-core) can compile the driver with its `DynamixelHandler` and a transport
over `BaseUart`. Turn on `HF_CORE_ENABLE_DYNAMIXEL` (off by default); the handler and its documentation live in
hf-core under `handlers/dynamixel/` and `docs/handlers/dynamixel_handler.md`.

## How the build is organised

| File | Role |
| --- | --- |
| [`cmake/hf_dynamixel_build_settings.cmake`](../cmake/hf_dynamixel_build_settings.cmake) | Sources, include directories, version, SDK pin: the single source of truth |
| [`cmake/hf_dynamixel_sdk_sources.cmake`](../cmake/hf_dynamixel_sdk_sources.cmake) | The SDK translation units compiled and their defines |
| [`CMakeLists.txt`](../CMakeLists.txt) | Static library, options, install and export |

The version and the SDK pin are set once in the settings file and written into `dynamixel_version.h` at configure
time. Read them at run time:

```cpp
const char* v = dynamixel::GetDriverVersion();   // "0.4.0"
const char* sdk = dynamixel::GetSdkTag();        // "4.1.0"; GetSdkCommit() gives the hash
```

## Next

[Quickstart](quickstart.md) wires a servo and reads it; [Porting](porting.md) writes the transport for your MCU.
