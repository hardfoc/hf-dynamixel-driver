# Model support

The first validated bench servo is **XC430-W150**, model number **1070**.

Common Protocol 2.0 X-series parts share the same control-table addresses
(operating mode 11, torque 64, goal position 116, present position 132, …).
They are listed in `inc/dynamixel_model.hpp`:

| Constant | Model | Number | Current register | Address 126 |
|----------|-------|--------|------------------|-------------|
| `kXc430W150` | XC430-W150 | 1070 | no | present **load** |
| `kXc430W240` | XC430-W240 | 1080 | no | present load |
| `kXl430W250` | XL430-W250 | 1060 | no | present load |
| `kXm430W210` | XM430-W210 | 1030 | yes (addr 102) | present **current** |
| `kXm430W350` | XM430-W350 | 1020 | yes | present current |
| `kXh430W210` | XH430-W210 | 1010 | yes | present current |
| `kXh430W350` | XH430-W350 | 1000 | yes | present current |
| `kXSeriesFallback` | unlisted X-series | 0 | treated as no | treated as load |

`Identify()` calls `FindModel(ping.model_number)`. On a hit, typed
torque / mode / position APIs are available.

## The servo you have not picked yet

A later larger servo can still run the same ESP32 examples:

1. Ping succeeds and prints the real model number.
2. If that number is not in the table, `Identify()` returns
   `UnsupportedRequest` **with the ping payload still filled**.
3. `BindXSeriesFallback()` (or `CONFIG_HF_DYNAMIXEL_BIND_XSERIES_FALLBACK`)
   attaches the shared X-series map so present/goal position work.
4. When you decide the part, add a `MakeXSeries(...)` row (set
   `supports_current` / `effort_is_current` from the e-Manual) and
   `FindModel` will bind it automatically.

`BindModel(desc)` before `Identify()` is kept when `desc.model_number` is 0
or matches the ping.

## What is not assumed

- Current control is **not** exposed as a typed API yet. XM/XH set
  `supports_current` so callers can gate their own writes to address 102.
- `Telemetry.load` is the raw value at address 126. Read
  `model()->effort_is_current` before treating it as milliamps.
- Protocol 1.0, MX-series, and DYNAMIXEL-P tables are out of scope.
- EEPROM writes stay explicit. Torque is never enabled by `Identify()` or
  `Bus` construction.

Unknown models may always use `Bus::ReadRegister` / `WriteRegister`.
