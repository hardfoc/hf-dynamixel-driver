# Configuration

Every setting the driver exposes, its default and where it lives: the transport configuration, the write policy,
model descriptors, and the Kconfig options of the ESP32 examples.

## Transport configuration

`Bus::Open()` takes a `TransportConfig` and hands it to your transport's `open()`.

| Field | Default | Meaning |
| --- | --- | --- |
| `baud_rate` | `57600` (`kDefaultBaud`) | Host rate in bit/s; must match the servos' Baud Rate register |
| `echo_policy` | `EchoPolicy::None` | `ExpectedLocalEcho` when RX hears every TX byte (single-wire link) |
| `latency_ms` | `2` | Slack added to every reply timeout, twice, on top of 2 ms and the packet's byte time |
| `max_packet_bytes` | `256` (`kMaxPacketBytes`) | Passed to your transport; the driver itself does not read it |

For reads, writes and pings of one servo, the reply timeout for n expected bytes is n × byte time +
2 × `latency_ms` + 2 ms, measured with your transport's `now_us()`. Raise `latency_ms` when the transport adds
buffering of its own (a USB serial adapter, an RTOS queue); lower it, and the servos' Return Delay Time, for faster
polling.

A single-wire link at 1 Mbit/s:

```cpp
--8<-- "tests/docs/configuration.cpp:echo"
```

With `ExpectedLocalEcho` the driver reads back each transmitted packet and compares it before it waits for the
reply; a missing or different echo is `EchoMismatch`.

## Write policy

Every `WriteRegister()` takes a `WritePolicy`, `TxRx` by default: `TxOnly` for fire-and-forget and broadcast,
`ReadbackVerify` for EEPROM values you must be sure of. The [API guide](api-guide.md#use-any-register-directly)
compares them. The typed `Device` setters always use `TxRx`.

## A servo of your own

A `ModelDescriptor` holds a servo's register addresses, widths and capabilities. For an X-series servo the catalog
does not list, `MakeXSeries()` builds one from the shared table:

```cpp
--8<-- "tests/docs/configuration.cpp:model"
```

| `MakeXSeries()` argument | Set it to |
| --- | --- |
| `number` | The model number from the e-Manual (Model Number, address 0) |
| `name` | Any label; `Device::model()->name` returns it |
| `supports_current` | `true` when the model has Goal Current (address 102) |
| `effort_is_current` | `true` when address 126 is Present Current rather than Present Load |

A servo with a different control table needs a `ModelDescriptor` written field by field; see
[Model support](models.md).

## Compile-time constants

| Constant | Value | Use |
| --- | --- | --- |
| `kBroadcastId` | `0xFE` | Broadcast, `TxOnly` writes |
| `kMaxDeviceId` | `0xFC` | Highest ID, 252 |
| `kDefaultBaud` | `57600` | Factory rate |
| `kMaxPacketBytes` | `256` | Default packet limit |

## ESP32 example options

`idf.py menuconfig`, menu **HF Dynamixel Example**
([`Kconfig.projbuild`](../examples/esp32/main/Kconfig.projbuild)):

Every option is named `CONFIG_HF_DYNAMIXEL_` followed by the name in the first column.

| Option | Default | Meaning |
| --- | --- | --- |
| `UART_PORT` | 1 | UART number |
| `UART_TX_GPIO` | 4 (C6), 17 (S3) | MCU TX |
| `UART_RX_GPIO` | 5 (C6), 18 (S3) | MCU RX |
| `UART_DIR_GPIO` | -1 | Buffer direction pin; -1: none |
| `DIR_ACTIVE_HIGH` | y | DIR high while transmitting |
| `BAUD` | 57600 | Bus rate |
| `SERVO_ID` | 1 | ID used when the scan finds nothing |
| `SCAN_LAST_ID` | 20 | Scan IDs 1 to this |
| `EXPECT_LOCAL_ECHO` | n | `EchoPolicy::ExpectedLocalEcho` |
| `BIND_XSERIES_FALLBACK` | y | Fallback for an unlisted model |

The per-chip defaults are in `sdkconfig.defaults.esp32c6` and `sdkconfig.defaults.esp32s3`.
