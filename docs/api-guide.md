# API guide

How to do the common jobs with the driver: open the bus, find and identify servos, move them, read them, talk to
several at once and handle errors.

Every snippet on this page is compiled by the host CI from
[`tests/docs/api_tasks.cpp`](../tests/docs/api_tasks.cpp), with `MyUart` the transport from
[Porting](porting.md) and `DriverError` standing for `dynamixel::DriverError`. The
[API reference](api-reference.md) lists every member.

## The shape of the API

| Class | Owns |
| --- | --- |
| `Bus` | One serial line: open, baud rate, ping, scan, register reads and writes by ID, sync read and write |
| `Device` | One servo on a bus: identify, torque, operating mode, motion profile, goals, telemetry |
| `ModelDescriptor` | A servo's control-table addresses and capabilities; the catalog is in `dynamixel_model.hpp` |
| `Transport<Derived>`, `TransportView` | Your UART class and the view the bus holds of it |

Every call is `noexcept` and returns `DriverResult<T>`: `value`, `error` and `detail` (the SDK result code, the
servo's error byte, byte counts and a timestamp). It converts to `bool`: `if (!servo.ReadPosition())` means it
failed. `ToString(error)` names the error.

## Open the bus

```cpp
--8<-- "tests/docs/api_tasks.cpp:open"
```

Create the bus with braces, `dynamixel::Bus bus{dynamixel::TransportView{uart}};`, and keep `uart` alive as long
as the bus. `Open()` calls your transport's `open()`; it sends nothing.
[Configuration](configuration.md) lists every `TransportConfig` field.

## Find the servos

```cpp
--8<-- "tests/docs/api_tasks.cpp:scan"
```

`Scan()` pings each ID in turn and costs one reply timeout per absent ID, so keep the range short. Any device ID
from 0 to 252 is valid (Protocol 2.0, section 2.3); the default range is all of them. ID 253 is not a device ID
and 254 is the broadcast ID, so `Scan()`, `Ping()`, `ReadRegister()` and the sync calls reject them.

## Identify a servo

```cpp
--8<-- "tests/docs/api_tasks.cpp:identify"
```

`Identify()` binds the servo's `ModelDescriptor` and caches its operating mode. Until a model is bound, the typed
calls below return `UnsupportedRequest`. [Model support](models.md) explains the catalog and the fallback.

## Move to a position

```cpp
--8<-- "tests/docs/api_tasks.cpp:move"
```

Operating Mode is an EEPROM register: `SetOperatingMode()` reads Torque Enable first and returns `Busy` if torque
is on. It never turns torque off for you. A goal for a mode the servo is not in returns `UnsupportedRequest`
without sending anything.

| `OperatingMode` | Goal call | Unit (XC430-W150) |
| --- | --- | --- |
| `Position` (3, factory): 0 to 360° | `SetGoalPosition()` | 1 pulse, 4096 per turn |
| `ExtendedPosition` (4): multi-turn | `SetGoalPosition()` | 1 pulse |
| `Velocity` (1) | `SetGoalVelocity()` | 0.229 rev/min |
| `Pwm` (16): voltage | `SetGoalPwm()` | 0.113 % |

`SetMotionProfile(acceleration, velocity)` writes Profile Acceleration (address 108) and Profile Velocity (112),
which shape every position move; 0 means unlimited.

## Spin at a velocity

```cpp
--8<-- "tests/docs/api_tasks.cpp:velocity"
```

## Read state

```cpp
--8<-- "tests/docs/api_tasks.cpp:read"
```

`ReadTelemetry()` makes two reads: one of the contiguous Present block, Present PWM (124) to Present Temperature
(146), 23 bytes, and one of Hardware Error Status (70). It fails when the block read fails; `hardware_error` stays
0 when its own read fails, and `detail` is the block read's. When the block reply would not fit `max_packet_bytes`
or the servo refuses the range with an error number, it falls back to one read per register, which fails only when
the position read fails. For one value, `ReadPosition()` is a single read. For several servos at once, use a sync
read.

## Read or write several servos at once

```cpp
--8<-- "tests/docs/api_tasks.cpp:sync"
```

A sync write is one broadcast packet and no replies. A sync read is one request and one status packet per servo;
`SyncRead()` collects each reply on its own, so a missing servo marks only its own `SyncReadItem` invalid and the
return value is the number of valid replies. Both take one address and length for all servos, little-endian
payloads.

## Use any register directly

```cpp
--8<-- "tests/docs/api_tasks.cpp:raw"
```

`ReadRegister()` and `WriteRegister()` work with or without a bound model, on the `Device` or on the `Bus` by ID.
The write policy says how sure you want to be:

| `WritePolicy` | Sends | Returns success when |
| --- | --- | --- |
| `TxOnly` | Write, no reply expected | The packet left the host (the only policy allowed for the broadcast ID) |
| `TxRx` (default) | Write, waits for the status packet | The servo acknowledged it without an error |
| `ReadbackVerify` | Write, then a read of the same bytes | The bytes read back match (up to 64 bytes) |

## Change the baud rate

```cpp
--8<-- "tests/docs/api_tasks.cpp:baud"
```

Baud Rate (address 8) takes an index, 0 to 7 ([Hardware](hardware.md#bus-settings)). Once the servo runs at the
new rate, `SetHostBaudRate()` moves the host. Every servo on the bus has to follow, or the host loses the ones
left behind.

## Handle errors

```cpp
--8<-- "tests/docs/api_tasks.cpp:errors"
```

| `DriverError` | Meaning | What to do |
| --- | --- | --- |
| `Timeout` | No status packet in time | Check ID, baud, wiring, power |
| `ChecksumError`, `MalformedResponse` | A reply arrived, damaged | Cable, ground, baud too high for the circuit |
| `EchoMismatch` | Local echo expected and not heard, or wrong | Match `EchoPolicy` to the wiring |
| `UncertainOutcome` | A write was sent and its reply lost | Read the register back before retrying |
| `DeviceError` | The servo answered with an error byte | `detail.device_error`: bit 7 hardware alert, bits 0 to 6 the error number |
| `UnsupportedRequest` | No model bound, or wrong mode for the call | `Identify()` or bind a model; change mode |
| `Busy` | Torque on during a mode change, or the bus is mid-transaction | Torque off first |
| `BufferTooSmall` | The request's packet is longer than `max_packet_bytes`; nothing was sent | Split it, or raise the limit ([Configuration](configuration.md#transport-configuration)) |
| `NotInitialized`, `InvalidParameter`, `InvalidId` | Bus not open, a null buffer or zero length, ID above 252 | Fix the call |

When bit 7 of the error byte is set, read Hardware Error Status (address 70, also in `Telemetry::hardware_error`):
bit 0 input voltage, bit 2 overheating, bit 3 motor encoder, bit 4 electrical shock, bit 5 overload. On the
faults enabled in Shutdown (address 63) the servo clears Torque Enable, and only a reboot turns it back on: a power
cycle, or the Reboot instruction.
[Troubleshooting](troubleshooting.md) maps symptoms to causes.

## Recover from a hardware error

```cpp
--8<-- "tests/docs/api_tasks.cpp:reboot"
```

`Reboot()` (on the `Device`, or `Bus::Reboot(id)`) sends the Reboot instruction, 0x08 (Protocol 2.0, section
5.7). The servo answers before it restarts; the Alert bit in that reply is the fault being cleared, so it does not
count as an error, and `detail.device_error` keeps it. A non-zero error number is `DeviceError`; no reply is
`UncertainOutcome`, because the servo may have rebooted anyway. After a reboot the RAM area is back to its
defaults: torque is off and Hardware Error Status is 0. The broadcast ID reboots every servo and gets no reply.
Fix the cause of the fault first, or it latches again.
