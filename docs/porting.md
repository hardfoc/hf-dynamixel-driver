# Porting

Implement `dynamixel::Transport<Derived>`:

| Hook | Meaning |
|---|---|
| `open` / `close` | Configure or release the UART. No servo motion. |
| `set_baud_rate` | Host rate only. |
| `write_some` | Partial writes are explicit. |
| `finish_transmit` | Last stop bit has left the wire; release the driver for RX. |
| `read_some` | Currently available bytes. `0` is not a failure. |
| `discard_stale_input` | Only at a transaction boundary (SDK `clearPort`). |
| `now_us` | Monotonic clock. |
| `delay_ms_impl` / `yield_impl` | Optional cooperative wait. |

`TransportConfig.echo_policy`:

- `None` — buffered or separate RX path
- `ExpectedLocalEcho` — adapter matches the TX bytes, then leaves the rest

Electrical mode (DIR GPIO, open-drain, auto-direction transceiver) stays in
the adapter. `Device` never sees a pin.

Reference adapters:

- ESP-IDF example: `examples/esp32/main/include/hf_dynamixel_esp_uart.hpp`
- hf-core: `HalUartDynamixelComm` in `handlers/dynamixel/DynamixelHandler.h`
  (wraps `BaseUart&`, same pattern as `HalUartFdo2Comm`)
