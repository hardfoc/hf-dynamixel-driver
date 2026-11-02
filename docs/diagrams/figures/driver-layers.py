"""The three layers of the driver: hardware-agnostic core, transport interface, platform transport.

Reads: From top to bottom. Your application talks to one Bus per serial line and one Device per servo; the public
members listed are the Bus and Device methods from inc/dynamixel_bus.hpp and inc/dynamixel_device.hpp. The bus
encodes and decodes Protocol 2.0 packets with the ROBOTIS DYNAMIXEL SDK, behind a private PortHandler adapter, and
reaches the line through TransportView, the type-erased view of a Transport<Derived> class
(inc/dynamixel_transport.hpp). That class is the only code you write: it moves bytes over a half-duplex UART and
reads a microsecond clock. Below it are the servos.
"""

import re
from pathlib import Path

from hardfoc_mkdocs_theme.figkit import Fig

ROOT = Path(__file__).resolve().parents[3]
BUS = "inc/dynamixel_bus.hpp"
DEVICE = "inc/dynamixel_device.hpp"
TRANSPORT = "inc/dynamixel_transport.hpp"


def _public_methods(path, cls):
    text = (ROOT / path).read_text()
    body = re.search(rf"class {cls} \{{\npublic:\n(.*?)\nprivate:", text, re.S).group(1)
    names = re.findall(r"^\s+(?:[\w:<>, ]+?)\s+([A-Z]\w*)\(", body, re.M)
    return list(dict.fromkeys(n for n in names if n != cls))


def data():
    hooks_doc = re.search(r"Derived must implement:\n(.*?)\n \*\n", (ROOT / TRANSPORT).read_text(), re.S).group(1)
    hooks = re.findall(r"(\w+)\(", hooks_doc)
    return {
        "bus": _public_methods(BUS, "Bus"),
        "device": _public_methods(DEVICE, "Device"),
        "port_hooks": hooks,
        "sources": [f"{BUS}:Bus members", f"{DEVICE}:Device members", f"{TRANSPORT}:Transport hooks"],
    }


def build(d):
    f = Fig(
        "driver-layers",
        "Three layers, one of them yours",
        alt="Layer diagram: the application calls the hardware-agnostic Bus and Device classes, which encode "
        "Protocol 2.0 packets with the ROBOTIS SDK and reach the line through the transport interface, which calls "
        "the UART transport you write, which drives the half-duplex bus of the DYNAMIXEL servos.",
        width=1200,
        kicker="Architecture",
        subtitle="The core and the interface are the same on every MCU. The transport is a small class with eight "
        "methods.",
        profile="overview",
        legend=[("brand", "this library"), ("teal", "you write"), ("slate", "hardware")],
    )
    W = f.body_w
    app = f.card(0, 0, W, "Your application", ["control loop, state machine, CLI ..."], tone="slate")
    top = app.b + 40
    names = [f"Bus::{n}" for n in d["bus"]] + [f"Device::{n}" for n in d["device"]]
    chips_h = f.chips(16, f.region_inner_top(top), W - 32, names, tone="brand")
    reg = f.region(0, top, W, f.region_inner_top(top) - top + chips_h + 18, "Hardware-agnostic core", tone="brand",
                   sub="Bus and Device, inc/dynamixel_bus.hpp, inc/dynamixel_device.hpp")
    comm = f.card(0, reg.b + 40, W, "Protocol engine and transport interface",
                  ["ROBOTIS DYNAMIXEL SDK 4.1.0 (Protocol 2.0 packets, CRC) behind a private PortHandler,",
                   "reaching the line through TransportView, the type-erased view of Transport<Derived>"],
                  tone="brand", accent=True)
    port = f.card(0, comm.b + 40, W, "Platform transport (your class)",
                  ["  ".join(h + "()" for h in d["port_hooks"])], tone="teal", accent=True, mono=True)
    hw = f.card(0, port.b + 40, W, "DYNAMIXEL X-series servos",
                ["Half-duplex TTL or RS-485, 8N1, 57600 bit/s factory default, IDs 0 to 252"], tone="slate")
    for a, b in ((app, reg), (reg, comm), (comm, port), (port, hw)):
        f.connect(a, b, "bottom", "top", tone="rail")
    return f
