# DYNAMIXEL Driver API reference {#mainpage}

The classes, members and types of the HardFOC DYNAMIXEL driver, generated from the headers.

Start with dynamixel::Bus, the shared Protocol 2.0 bus, and dynamixel::Device, one servo on it. Your platform code
implements dynamixel::Transport; the bus binds it through dynamixel::TransportView. Results are
dynamixel::DriverResult with a dynamixel::DriverError; the X-series register map is dynamixel::ModelDescriptor.

The guides, with examples that compile, are on the documentation site: <a href="../">DYNAMIXEL Driver
documentation</a>.
