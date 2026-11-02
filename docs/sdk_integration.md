# SDK integration

Pinned engine: ROBOTIS DynamixelSDK **4.1.0** /
`f838bc90f72fcf5b9c279432d6e12fc24969daa8`.

HF does **not** fork or reimplement Protocol 2.0. The wrapper:

- Instantiates `Protocol2PacketHandler::getInstance()` (SDK singleton).
- Implements a private `PortHandler` subclass. It does **not** call
  `getPortHandler()` (that factory is desktop-port specific).
- Maps `COMM_*` to `dynamixel::DriverError` in `MapCommCode`.
- Compiles only the protocol / group TUs listed in
  `cmake/hf_dynamixel_sdk_sources.cmake`. No stock Linux / Windows / Arduino
  port.

## Limits we inherited and document

| Behavior | Consequence |
|----------|-------------|
| Per-call `malloc` inside the SDK | Heap must exist; we do not patch it out |
| `is_using_` is a flag, not a mutex | `Bus` is not thread-safe; hf-core adds `RtosMutex` |
| `clearPort` before TX | Stale RX is discarded at the transaction boundary only |
| GroupSyncRead `last_result_` is all-or-nothing | HF `SyncRead` uses TX + per-id RX |

Apache-2.0 notices for the SDK live in `THIRD_PARTY_NOTICES`. This
repository is MIT.

## ESP-IDF include guards

SDK `.cpp` files only include their headers when `__linux__` / `__APPLE__` /
`WIN32` / `ARDUINO` is defined. The ESP-IDF component (and hf-core, on those
translation units only) defines `__linux__` so the TUs compile on Xtensa and
RISC-V. That define is **not** applied to the rest of hf-core.
