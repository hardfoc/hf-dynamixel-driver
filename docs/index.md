---
layout: default
title: "Documentation"
description: "HardFOC Dynamixel Protocol 2.0 driver — architecture, models, ESP32-C6/S3 examples, and GitHub Pages"
nav_order: 2
parent: "HardFOC Dynamixel Driver"
permalink: /docs/
has_children: true
---

# HF-Dynamixel documentation

This site documents the portable **hf-dynamixel-driver**: a C++17 wrapper around
ROBOTIS Dynamixel SDK **4.1.0**. HF owns the transport contract, result model,
and X-series descriptors. The SDK owns Protocol 2.0 encode/decode.

The first bench servo is **XC430-W150** (model 1070). A later larger X-series
servo can use the same examples via the catalog (XM430/XH430) or
`BindXSeriesFallback()` after a successful ping.

> Browse on GitHub: [repository](https://github.com/hardfoc/hf-dynamixel-driver) ·
> [issues](https://github.com/hardfoc/hf-dynamixel-driver/issues) ·
> [Doxygen API]({{ '/html/' | relative_url }})

## Start here

1. [Installation](installation.md) — CMake, submodules, `hf::dynamixel`
2. [Quick start](quickstart.md) — CRTP transport, ping, typed XC430 path
3. [ESP32 examples](esp32_examples.md) — `build_app.sh` on C6 or S3
4. [Wiring](wiring.md) — pins, baud, half-duplex, supplies
5. [Model support](model_support.md) — XC430 vs a larger X-series

## Design and integration

6. [Architecture](architecture.md) — Bus / Device / TransportView boundaries
7. [SDK integration](sdk_integration.md) — pin, COMM codes, known limits
8. [Porting](porting.md) — implement `dynamixel::Transport<Derived>`
9. [hf-core handler](hf_core_handler.md) — `HF_CORE_ENABLE_DYNAMIXEL` + `DynamixelHandler`

## Publishing

10. [GitHub Pages](github_pages.md) — Jekyll + Doxygen via `hf-general-ci-tools`
11. [Troubleshooting](troubleshooting.md) — timeouts, echo, torque, unknown models
