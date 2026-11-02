# Architecture

HF owns the public API, transport contract, X-series descriptors, and tests.
ROBOTIS Dynamixel SDK 4.1.0 owns Protocol 2.0 encode/decode.

```
Application  →  Device  →  Bus  →  PortHandlerAdapter (private)
                                      ↓
                               Protocol2PacketHandler
                                      ↓
                               TransportView  →  CRTP adapter
```

## Boundaries

| Layer | Lives in | May include SDK? | May enable torque? |
|-------|----------|------------------|--------------------|
| `inc/dynamixel_*.hpp` | this repo | No | No |
| `src/backend/port_handler_adapter.hpp` | this repo | Yes (private) | No |
| `dynamixel_sdk` TUs | submodule | Yes | No |
| ESP-IDF UART adapter | `examples/esp32/main/include` | No | No |
| `DynamixelHandler` | hf-core `handlers/dynamixel` | No | Only when the app calls `SetTorqueEnabled` |

- Callers implement `dynamixel::Transport<Derived>`. `Bus` binds it through
  `TransportView` so `Bus` is not `Bus<TransportT>`.
- The transport outlives the bus; the bus outlives devices.
- `Bus` is not copyable and not thread-safe. One caller owns each transaction.
  hf-core serializes with `RtosMutex`.
- Constructors never open UART, enable torque, or move a servo.
- Sync read is implemented with `syncReadTx` plus per-id `readRx` so one
  missing status does not invalidate the whole group.

See [SDK integration](sdk_integration.md) for the pinned commit and measured
limitations (`is_using_` is not a mutex; the SDK mallocs per call).
