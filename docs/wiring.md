# Wiring

Initial bring-up: **57600** bit/s, 8N1, short cable.

| Chip | UART | TX (MCU → servo) | RX (servo → MCU) | Optional DIR |
|------|------|------------------|------------------|--------------|
| ESP32-C6 | UART1 | GPIO4 | GPIO5 | unused |
| ESP32-S3 | UART1 | GPIO17 | GPIO18 | unused |

Override in `menuconfig` or `sdkconfig.defaults.<target>`. Do not infer C6
runtime results from an S3 build or the other way around.

## Electrical modes

Implemented in the transport, not in `Device`:

- **Direction-controlled buffer** — assert DIR, send, `finish_transmit`,
  release DIR (`CONFIG_HF_DYNAMIXEL_UART_DIR_GPIO`)
- **Open-drain single wire** — keep the receiver on; set
  `CONFIG_HF_DYNAMIXEL_EXPECT_LOCAL_ECHO`
- **Automatic-direction transceiver** — leave DIR at `-1`

A transistor TTL board is acceptable for bench ping/read. Qualify a proper
buffered interface before high-rate or long-cable claims.

## Power

XC430-W150 and XM430-class servos are **not** 3.3 V motors. Use the rated
Dynamixel supply (typically 12 V) on the servo power pins. Share ground with
the ESP32. Logic must be 3.3 V compatible toward the MCU.

{: .important }
The motion example enables torque. Run it unloaded, after a clean read-only
session, and only after `IdentifyOrBind` has a descriptor.

## Hardware gate (not run in CI)

1000 read-only transactions at 57600 with zero unexplained errors, plus
unplug and corrupt-response cases. Record board, wiring, supply, baud, servo
firmware, SDK hash, and compiler.
