# Model support

Which servos the driver knows by model number, what "knows" gives you, and how to run a servo it does not list.

## The catalog

`Device::Identify()` pings the servo and looks its model number up in `kKnownModels`
([`inc/dynamixel_model.hpp`](../inc/dynamixel_model.hpp)). Every entry is built by `MakeXSeries()` and shares the
X-series control table ([Hardware](hardware.md#control-table-used-by-the-driver)); they differ only in two flags.

| Constant | Model number | Address 126 is |
| --- | --- | --- |
| `kXc430W150` | 1070 | Present Load |
| `kXc430W240` | 1080 | Present Load |
| `kXl430W250` | 1060 | Present Load |
| `kXm430W210` | 1030 | Present Current |
| `kXm430W350` | 1020 | Present Current |
| `kXh430W210` | 1010 | Present Current |
| `kXh430W350` | 1000 | Present Current |
| `kXSeriesFallback` | 0 (any) | treated as Present Load |

The XC430-W150 is the servo the driver was validated on. The XM and XH entries set `supports_current` and
`effort_is_current`; read `servo.model()->effort_is_current` before treating `Telemetry::load` as current.

## What a bound model gives you

Without a model, `Device`'s typed calls return `UnsupportedRequest`; `ReadRegister()` and `WriteRegister()` still
work. With one:

| Call | Needs |
| --- | --- |
| `SetTorqueEnabled()`, `SetMotionProfile()`, `ReadPosition()`, `ReadTelemetry()` | A model |
| `SetOperatingMode()` | A model, and torque off (else `Busy`) |
| `SetGoalPosition()` | Mode Position or Extended Position |
| `SetGoalVelocity()` | Mode Velocity |
| `SetGoalPwm()` | Mode PWM (16), and a model with `supports_pwm` |

`Identify()` also reads the servo's Operating Mode and caches it; `SetOperatingMode()` updates the cache, and
`operating_mode()` returns it. A register value without a typed mode (current 0, current-based position 5, or a
value the control table does not define) is cached as `OperatingMode::Unknown`, with the raw byte in
`operating_mode_raw()`. A goal for another mode, or any goal while the mode is `Unknown`, is refused before
anything is sent.

## A servo the catalog does not list

1. `Identify()` returns `UnsupportedRequest` with `ping.value.model_number` filled in: the servo answered.
2. Call `BindXSeriesFallback()` to use the shared X-series addresses, or bind a descriptor of your own with
   `BindModel()` before calling `Identify()` again. A bound descriptor survives `Identify()` when its
   `model_number` is 0 or matches the ping.
3. When the part is settled, add a `MakeXSeries()` entry to `kKnownModels`, with `supports_current` and
   `effort_is_current` from the model's e-Manual page, so `Identify()` binds it by itself.

[Configuration](configuration.md#a-servo-of-your-own) shows a descriptor for the XM540-W270 (model 1120).

## Out of scope

- Protocol 1.0, the MX, AX and DYNAMIXEL-P series: their control tables differ.
- Typed current control: write Goal Current (address 102) with `WriteRegister()` and set Operating Mode 0 or 5
  yourself. `OperatingMode` lists only velocity (1), position (3), extended position (4) and PWM (16).
- Bulk Read and Bulk Write instructions.
