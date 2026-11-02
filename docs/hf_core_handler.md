# hf-core handler

This repository stays portable. The HAL-facing wrapper lives in
**hf-core**, not here.

| Piece | Location |
|-------|----------|
| Toggle | `HF_CORE_ENABLE_DYNAMIXEL` (default **OFF**) |
| Compile define | `HARDFOC_DYNAMIXEL_SUPPORT=1` |
| Auto-enables | `HF_CORE_ENABLE_UART` |
| Handler | `handlers/dynamixel/DynamixelHandler.h` |
| Transport | `HalUartDynamixelComm` over `BaseUart&` |

`examples/esp32/components/hf_core` in hf-core turns the toggle **ON** so
core firmware examples compile the handler. Product images should keep it
OFF until they own a UART and a servo.

```cpp
#include "DynamixelHandler.h"

DynamixelHandlerConfig cfg;
cfg.servo_id = 1;
cfg.transport.baud_rate = 57600;
cfg.bind_xseries_fallback = true;

DynamixelHandler dxl(uart, cfg);  // uart already configured
if (!dxl.EnsureInitialized()) {
    return;
}
auto tel = dxl.ReadTelemetry();   // still no torque
```

`EnsureInitialized()` opens the bus and identifies (or binds the X-series
fallback). It does **not** enable torque. Share `RtosMutex*` when more than
one protocol shares the UART — Dynamixel half-duplex almost never does.

See hf-core `docs/handlers/dynamixel_handler.md` and
`docs/cmake_integration.md`.
