# Troubleshooting

Symptoms you may see while bringing up a DYNAMIXEL bus, their causes and the fix for each.

Start with the `DriverError` and `detail` of the failing call: `detail.comm_code` is the SDK's result,
`detail.device_error` the servo's error byte. `Timeout` is a link problem; `DeviceError` means the servo heard you.

## Every ping times out

| Cause | Fix |
| --- | --- |
| Baud rate differs from the servo's | Try 57600 (factory), then the other seven rates; or scan with ROBOTIS's DYNAMIXEL Wizard |
| Wrong ID | Factory ID is 1; `Scan()` IDs 0 to 20 |
| TX and RX swapped, or wired for the wrong board | ESP32-C6 uses GPIO4 / GPIO5, ESP32-S3 GPIO17 / GPIO18 |
| Direction pin stuck in transmit | Release it in `finish_transmit()`; leave the DIR GPIO at -1 for an auto-direction transceiver |
| No common ground, or servo unpowered | Join the grounds; the servo needs its own supply (6.5 V to 14.8 V for an XC430-W150) |
| 3.3 V MCU straight on a 5 V TTL line | Use a buffer or level shifter ([Hardware](hardware.md#half-duplex-wiring)) |

## `EchoMismatch`

The transport heard its own transmission when it should not have, or did not hear it when it should. Set
`EchoPolicy::ExpectedLocalEcho` exactly when TX and RX are joined on one wire
(`CONFIG_HF_DYNAMIXEL_EXPECT_LOCAL_ECHO` in the examples).

## Replies are sometimes corrupt

`ChecksumError` or `MalformedResponse` now and then: the cable is too long or too noisy for the rate, the circuit
is too slow for it, or `finish_transmit()` releases the line too early and clips the last byte. Drop to 57600 to
confirm, then fix the circuit before raising the rate again.

## `Identify()` returns `UnsupportedRequest`

The ping worked and the model number is not in the catalog. `ping.value.model_number` has it. Bind
`BindXSeriesFallback()` for an X-series servo, or a descriptor of your own ([Model support](models.md)).

## A goal call returns `UnsupportedRequest`

The servo is in another operating mode (`SetGoalVelocity()` in Position mode, for example), or no model is bound.
Change the mode with torque off, or call `Identify()` first. A servo left in current mode (0) or current-based
position mode (5) by another tool refuses every typed goal: set a supported mode first.

## `SetOperatingMode()` returns `Busy`

Torque is on. Call `SetTorqueEnabled(false)` first; the driver will not do it for you.

## A write returns `UncertainOutcome`

The write went out and no status packet came back. The servo may have applied it. Read the register back before
you retry; for an EEPROM change of ID or baud rate, the servo may now answer at the new value.

## Torque turns itself off

The servo hit a fault enabled in Shutdown (address 63): input voltage, overheating, encoder, electrical shock or
overload. `Telemetry::hardware_error` has the bits ([API guide](api-guide.md#handle-errors)). Fix the cause, then
call `Reboot()` or power-cycle the servo: only a reboot clears the fault and lets torque back on
([Recover from a hardware error](api-guide.md#recover-from-a-hardware-error)).

## The motion example jumps

`dynamixel_motion_example` moves 64 ticks from the present position. A dash towards 0 or 2048 means the position
read failed and a raw goal was written anyway. Go back to the read-only example until reads are clean.

## The examples do not configure

`APP_TYPE not defined` means CMake or `idf.py` ran without the scripts. Build from `examples/esp32` with
`./scripts/build_app.sh <app> Debug`; if `scripts/` is empty, run `git submodule update --init --recursive`. An S3
build that still uses C6 pins was started without `CONFIG_TARGET=esp32s3`.
