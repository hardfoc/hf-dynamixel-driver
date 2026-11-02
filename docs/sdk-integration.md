# SDK integration

How the driver uses the ROBOTIS DYNAMIXEL SDK, which version, and the SDK behaviour you inherit.

## The pinned SDK

| Item | Value |
| --- | --- |
| Repository | [ROBOTIS-GIT/DynamixelSDK](https://github.com/ROBOTIS-GIT/DynamixelSDK), submodule `external/DynamixelSDK` |
| Tag | 4.1.0 |
| Commit | `f838bc90f72fcf5b9c279432d6e12fc24969daa8` |
| Licence | Apache-2.0 ([`THIRD_PARTY_NOTICES`](../THIRD_PARTY_NOTICES)); the driver is MIT |

`dynamixel::GetSdkTag()` and `GetSdkCommit()` return the pin the binary was built with.

## What the driver takes from the SDK

- `Protocol2PacketHandler::getInstance()`, the SDK's singleton, for every packet.
- `GroupSyncWrite` for `Bus::SyncWrite()`.
- Four source files, listed in [`cmake/hf_dynamixel_sdk_sources.cmake`](../cmake/hf_dynamixel_sdk_sources.cmake):
  `protocol2_packet_handler.cpp`, `group_handler.cpp`, `group_sync_read.cpp`, `group_sync_write.cpp`. None of
  the SDK's Linux, Windows or Arduino port handlers is compiled.

The SDK talks to a `PortHandler`. The driver's private
[`PortHandlerAdapter`](../src/backend/port_handler_adapter.hpp) implements it over your transport: `writePort()`
loops on `write_some()`, then calls `finish_transmit()` and checks the echo; `readPort()` calls `read_some()`; the
packet timeout is measured with `now_us()`. It never calls the SDK's `getPortHandler()` factory.

`MapCommCode()` turns the SDK's result codes into `DriverError`:

| SDK code | `DriverError` |
| --- | --- |
| `COMM_SUCCESS` | `None` |
| `COMM_PORT_BUSY` | `Busy` |
| `COMM_TX_FAIL`, `COMM_RX_FAIL` | `SerialError` |
| `COMM_TX_ERROR` | `InvalidParameter` |
| `COMM_RX_TIMEOUT` | `Timeout` (a write with a reply: `UncertainOutcome`) |
| `COMM_RX_CORRUPT` | `ChecksumError` |
| `COMM_NOT_AVAILABLE` | `UnsupportedRequest` |
| anything else | `MalformedResponse` |

The raw code is kept in `DriverResult::detail.comm_code`.

## Behaviour you inherit

| SDK behaviour | Consequence |
| --- | --- |
| Packet buffers are allocated with `malloc` on each call | The target needs a heap; the driver does not patch it out |
| `is_using_` is a flag, not a mutex | `Bus` is not thread-safe; one owner, or a mutex around every call |
| `clearPort()` before each transmission | Stale input is dropped at the start of a transaction only |
| `GroupSyncRead` fails the whole group when one servo is missing | `Bus::SyncRead()` sends with `syncReadTx()` and reads each reply with `readRx()`, so one silent servo leaves the others valid |

## Building the SDK sources on an MCU

The SDK's source files include their headers only when `__linux__`, `__APPLE__`, `WIN32` or `ARDUINO` is defined.
On any other toolchain (ESP-IDF on Xtensa or RISC-V, bare-metal GCC) the build settings add `__linux__` as a
private define of the driver library. It never reaches your code.
