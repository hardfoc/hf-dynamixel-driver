# Quickstart

Read a DYNAMIXEL servo from your MCU: wire three signals, write a small transport class, call four driver
functions.

!!! warning "Before you power the servo"
    Feed the servo from its own supply (12 V for an XC430-W150), never from the MCU's rail, and join the grounds.
    The example below never enables torque; keep it that way until reads are clean.

## 1. Wire it

| MCU | Servo (TTL, 3-pin) | Notes |
| --- | --- | --- |
| UART TX and RX | Pin 3, DATA | Through a half-duplex circuit ([Hardware](hardware.md#half-duplex-wiring)) |
| GND | Pin 1, GND | Common with the servo supply ground |
| | Pin 2, VDD | Servo supply only, not the MCU rail |

Start at 57600 bit/s, 8N1, with one servo on a short cable: that is the factory setting (ID 1). The
[Hardware](hardware.md) page has the RS-485 pinout, the half-duplex options and the supply ranges.

## 2. Add the driver

Add the repository as a submodule, with its own submodule (the ROBOTIS SDK), and link the `hf::dynamixel` CMake
target. [Installation](installation.md) covers CMake and ESP-IDF.

```bash
git submodule add https://github.com/hardfoc/hf-dynamixel-driver.git external/hf-dynamixel-driver
git submodule update --init --recursive external/hf-dynamixel-driver
```

## 3. Write the transport

The transport is the only platform code. It writes and reads UART bytes, waits for the end of transmission, flips
the bus direction if your circuit has a direction pin, and reads a microsecond clock. Replace the `platform_*`
calls with your SDK's.

```cpp title="my_uart.hpp"
--8<-- "tests/docs/my_uart.hpp:port"
```

[Porting](porting.md) explains each method and the three half-duplex wirings.

## 4. Read the servo

```cpp title="main.cpp"
--8<-- "tests/docs/quickstart.cpp:quickstart"
```

What each step does:

1. `Bus::Open()` calls your transport's `open()` with the configuration. A default `TransportConfig` is
   57600 bit/s with no local echo. Nothing is sent yet.
2. `Device::Identify()` pings ID 1, looks the model number up in the catalog and reads the operating mode. An
   X-series model the catalog does not list answers with `UnsupportedRequest`; `BindXSeriesFallback()` then
   attaches the shared X-series control table.
3. `ReadTelemetry()` reads position, velocity, load, PWM, input voltage, temperature and the hardware error
   byte. It writes nothing.

Write `dynamixel::Bus bus{dynamixel::TransportView{uart}};` with braces. With parentheses,
`Bus bus(TransportView(uart));` declares a function, not a bus.

This example is built by the host CI, so it compiles against the current headers.

## If it does not work

| Symptom | First thing to check |
| --- | --- |
| `Identify()` returns `Timeout` | Baud rate and ID, the half-duplex circuit, a common ground, servo power |
| `EchoMismatch` | Your RX hears your TX: set `EchoPolicy::ExpectedLocalEcho` ([Configuration](configuration.md)) |
| `ChecksumError` now and then | Cable length, baud too high for the circuit, ground noise |
| `UnsupportedRequest` | Model not in the catalog: bind the fallback ([Model support](models.md)) |

## Next

- [API guide](api-guide.md): change the mode, move, sync read and write, handle errors.
- [Examples](examples.md): ESP32-C6 and ESP32-S3 applications for read-only, scan and motion.
- [Model support](models.md): the catalog and how to add a servo.
