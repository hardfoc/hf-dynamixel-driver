---
hide:
  - navigation
  - toc
---

<div class="hf-hero" markdown>
<span class="hf-kicker">HardFOC driver</span>

# DYNAMIXEL Driver

<p>A C++17 driver for ROBOTIS DYNAMIXEL X-series smart servos over Protocol 2.0. It pings and identifies servos,
sets the operating mode, commands position, velocity and PWM, reads telemetry and runs sync reads and writes, on
any MCU with a half-duplex UART.</p>

[Quickstart](quickstart.md){ .md-button .md-button--primary }
[API guide](api-guide.md){ .md-button }
[Source on GitHub](https://github.com/hardfoc/hf-dynamixel-driver){ .md-button }
</div>

The driver has no platform code. You give it a small class that writes and reads bytes on a UART and tells the
time; it does the rest: Protocol 2.0 packets and CRC (through the official ROBOTIS SDK), reply timeouts, local-echo
handling, the X-series control table and error mapping.

<div class="hf-chip" markdown>
<div class="hf-chip__head">
<span class="hf-chip__kicker">Smart servo actuators, Protocol 2.0</span>
<span class="hf-chip__name">DYNAMIXEL X-series</span>
<span class="hf-chip__vendor">ROBOTIS</span>
</div>

Servos
:   XC430-W150 (validated), XC430-W240, XL430-W250, XM430-W210 / W350, XH430-W210 / W350, other X-series by fallback

Bus
:   Half-duplex TTL (3-pin) or RS-485 (4-pin) multidrop, 8 data bits, no parity, 1 stop bit

Speed
:   9600 bit/s to 4.5 Mbit/s; factory default 57600 bit/s

Addressing
:   IDs 0 to 252, factory ID 1; broadcast ID 254 (`0xFE`)

Supply (servo)
:   XC430-W150: 6.5 V to 14.8 V, 12 V recommended; XM430-W350: 10.0 V to 14.8 V

Modes the driver uses
:   Position, extended position, velocity, PWM; current is a raw register write

[XC430-W150 e-Manual](https://emanual.robotis.com/docs/en/dxl/x/xc430-w150/) ·
[XM430-W350 e-Manual](https://emanual.robotis.com/docs/en/dxl/x/xm430-w350/) ·
[Protocol 2.0](https://emanual.robotis.com/docs/en/dxl/protocol2/)
</div>

## What you get

- **A bus and a device.** `dynamixel::Bus` owns one serial line; `dynamixel::Device` is one servo on it, with
  typed calls for torque, mode, goals and telemetry. See [API guide](api-guide.md).
- **Model-aware.** `Identify()` pings the servo and binds its control table from a catalog of seven X-series
  models, or a shared X-series fallback. See [Model support](models.md).
- **The official protocol engine.** Packets, CRC and sync instructions come from the ROBOTIS DYNAMIXEL SDK 4.1.0,
  hidden behind the public headers. See [SDK integration](sdk-integration.md).
- **Results you can act on.** Every call returns a `DriverResult` with a `DriverError` and the raw SDK and device
  codes; a write whose reply was lost is `UncertainOutcome`, not a silent failure. See
  [API guide](api-guide.md#handle-errors).
- **Any half-duplex wiring.** Direction buffer, auto-direction transceiver or single wire with local echo: the
  transport decides, the driver never touches a pin. See [Porting](porting.md).
- **Safe by construction.** No constructor opens the UART, enables torque or moves a servo. See
  [Quickstart](quickstart.md).
- **Tested without hardware.** Host tests run the real SDK against a scripted transport. See
  [Testing and CI](testing.md).

## Where to go next

<div class="grid cards" markdown>

-   :material-rocket-launch-outline: **[Quickstart](quickstart.md)**

    ---

    Wire a servo, write the transport, read it in about 20 lines.

-   :material-chip: **[Hardware](hardware.md)**

    ---

    Connectors, half-duplex wiring, bus settings and power.

-   :material-layers-outline: **[Architecture](architecture.md)**

    ---

    The three layers and what each one owns.

-   :material-code-braces: **[API guide](api-guide.md)**

    ---

    Identify, configure, move, read and handle errors, task by task.

-   :material-swap-horizontal: **[Porting](porting.md)**

    ---

    Implement the transport for your MCU.

-   :material-book-open-variant: **[API reference](api-reference.md)**

    ---

    Every class and member, generated from the headers.

</div>
