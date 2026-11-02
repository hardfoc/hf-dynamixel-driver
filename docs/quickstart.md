# Quick start

The driver never opens UART, enables torque, or moves a servo from a constructor.
You supply a `dynamixel::Transport` adapter; `Bus` talks Protocol 2.0 through it.

```cpp
#include "dynamixel.hpp"

MyUart uart;  // implements dynamixel::Transport<MyUart>
dynamixel::Bus bus{dynamixel::TransportView{uart}};  // brace-init, not Bus(View)

dynamixel::TransportConfig cfg;
cfg.baud_rate = 57600;
cfg.echo_policy = dynamixel::EchoPolicy::None;
if (!bus.Open(cfg).ok()) {
    return;
}

auto ping = bus.Ping(1);
if (!ping.ok()) {
    return;
}

dynamixel::Device servo(bus, 1);
auto idn = servo.Identify();
if (!idn.ok()) {
    // Ping worked but the model number is not in the catalog.
    servo.BindXSeriesFallback();
}

auto pos = servo.ReadPosition();  // torque stays off
```

{: .warning }
Do not write `Bus bus(TransportView(uart));` — that is a most-vexing-parse and
declares a function.

## What Identify does not do

- It does not enable torque.
- It does not change operating mode, ID, or baud.
- It does not recover a servo that is already in an error state.

EEPROM writes (ID, baud, operating mode) are always explicit
`WriteRegister` / `SetOperatingMode` calls. A timeout on those writes is
`UncertainOutcome`, not proof the old value remains.

## Next

- ESP32-C6 / S3 firmware: [ESP32 examples](esp32_examples.md)
- Adapter hooks: [Porting](porting.md)
- Catalog and fallback: [Model support](model_support.md)
