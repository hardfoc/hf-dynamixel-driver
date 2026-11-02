# Troubleshooting

## `Timeout` on every ping

- Baud mismatch (servo EEPROM vs `CONFIG_HF_DYNAMIXEL_BAUD`). Factory XC430
  is often 57600.
- TX/RX swapped for the chip you actually flashed (C6 4/5 vs S3 17/18).
- No common ground, or the servo is unpowered.
- Half-duplex DIR stuck in TX. Leave DIR at `-1` unless the buffer needs it.
- Single-wire open-drain without `CONFIG_HF_DYNAMIXEL_EXPECT_LOCAL_ECHO`.

## `EchoMismatch`

The UART heard its own TX but the bytes did not match, or echo was expected
and none arrived. Flip `CONFIG_HF_DYNAMIXEL_EXPECT_LOCAL_ECHO` to match the
electrical mode.

## `UnsupportedRequest` from `Identify()`

Ping succeeded; the model number is not in `kKnownModels`. The result still
carries `ping.value.model_number`. Enable the Kconfig fallback or call
`BindXSeriesFallback()` / `BindModel(kXm430W350)` as appropriate.

## `Busy` from `SetOperatingMode`

Torque is on. Disable torque, then change mode. The driver will not do that
implicitly.

## Motion jumps or slams

The motion example is supposed to add 64 ticks to **present** position. If
you see a dash to 0 / 2048, the read failed or you wrote a raw goal without
reading first. Stop and go back to the read-only example.

## Examples will not configure

```
APP_TYPE not defined. Use build_app.sh
```

You invoked CMake or `idf.py` without the scripts. From `examples/esp32`:

```bash
./scripts/build_app.sh list
./scripts/build_app.sh dynamixel_readonly_example Debug
```

If `scripts/` is empty: `git submodule update --init --recursive`.

## S3 build still uses C6 pins

`CONFIG_TARGET` was not set, so `metadata.target` (esp32c6) won. Use:

```bash
CONFIG_TARGET=esp32s3 ./scripts/build_app.sh dynamixel_readonly_example Debug
```

Confirm the log line `uart1 tx=17 rx=18`.
