/**
 * @file dynamixel_transport.hpp
 * @brief CRTP serial transport and type-erased bus view.
 *
 * @details Concrete adapters implement the derived hooks. `Bus` binds them
 *          through `TransportView` so public headers never include the SDK.
 *
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 */
#pragma once

#include "dynamixel_types.hpp"

#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace dynamixel {

struct TransportConfig {
    uint32_t    baud_rate{kDefaultBaud};
    /// Largest packet (before byte stuffing) the bus sends or asks for. A
    /// request whose instruction or status packet is longer returns
    /// `BufferTooSmall` without sending. `Bus::Open()` rejects a value below
    /// `kMinPacketBytes` and caps one above `kSdkMaxPacketBytes`.
    std::size_t max_packet_bytes{kMaxPacketBytes};
    EchoPolicy  echo_policy{EchoPolicy::None};
    uint32_t    latency_ms{2};  ///< Slack added to SDK packet timeouts (not USB 16 ms).
};

/**
 * @brief CRTP base for a Dynamixel half-duplex byte transport.
 *
 * Derived must implement:
 *   DriverError open(const TransportConfig&);
 *   void close();
 *   DriverError set_baud_rate(uint32_t);
 *   TransportIo write_some(const uint8_t*, std::size_t);
 *   DriverError finish_transmit();
 *   TransportIo read_some(uint8_t*, std::size_t);
 *   void discard_stale_input();
 *   uint64_t now_us();
 *
 * Optional:
 *   void delay_ms_impl(uint32_t);
 *   void yield_impl();
 */
template <typename Derived>
class Transport {
public:
    DriverError open(const TransportConfig& cfg) noexcept {
        return static_cast<Derived*>(this)->open(cfg);
    }
    void close() noexcept { static_cast<Derived*>(this)->close(); }

    DriverError set_baud_rate(uint32_t baud) noexcept {
        return static_cast<Derived*>(this)->set_baud_rate(baud);
    }

    TransportIo write_some(const uint8_t* data, std::size_t length) noexcept {
        return static_cast<Derived*>(this)->write_some(data, length);
    }
    DriverError finish_transmit() noexcept {
        return static_cast<Derived*>(this)->finish_transmit();
    }
    TransportIo read_some(uint8_t* out, std::size_t max) noexcept {
        return static_cast<Derived*>(this)->read_some(out, max);
    }
    void discard_stale_input() noexcept {
        static_cast<Derived*>(this)->discard_stale_input();
    }
    uint64_t now_us() noexcept { return static_cast<Derived*>(this)->now_us(); }

    void delay_ms(uint32_t ms) noexcept {
        if constexpr (HasDelay<Derived>::value) {
            static_cast<Derived*>(this)->delay_ms_impl(ms);
        } else {
            (void)ms;
        }
    }
    void yield() noexcept {
        if constexpr (HasYield<Derived>::value) {
            static_cast<Derived*>(this)->yield_impl();
        }
    }

private:
    /// @cond INTERNAL
    template <typename, typename = void>
    struct HasDelay : std::false_type {};
    template <typename T>
    struct HasDelay<T, std::void_t<decltype(std::declval<T>().delay_ms_impl(0U))>>
        : std::true_type {};

    template <typename, typename = void>
    struct HasYield : std::false_type {};
    template <typename T>
    struct HasYield<T, std::void_t<decltype(std::declval<T>().yield_impl())>>
        : std::true_type {};
    /// @endcond
};

/**
 * @brief Non-owning type-erased view of a CRTP transport.
 *
 * The referenced adapter must outlive every `Bus` that uses this view.
 */
class TransportView {
public:
    TransportView() noexcept = default;

    template <typename Derived,
              typename = std::enable_if_t<!std::is_same<std::decay_t<Derived>, TransportView>::value>>
    explicit TransportView(Derived& transport) noexcept
        : self_(&transport),
          open_([](void* s, const TransportConfig& c) noexcept {
              return static_cast<Derived*>(s)->open(c);
          }),
          close_([](void* s) noexcept { static_cast<Derived*>(s)->close(); }),
          set_baud_([](void* s, uint32_t b) noexcept {
              return static_cast<Derived*>(s)->set_baud_rate(b);
          }),
          write_some_([](void* s, const uint8_t* d, std::size_t n) noexcept {
              return static_cast<Derived*>(s)->write_some(d, n);
          }),
          finish_transmit_([](void* s) noexcept {
              return static_cast<Derived*>(s)->finish_transmit();
          }),
          read_some_([](void* s, uint8_t* d, std::size_t n) noexcept {
              return static_cast<Derived*>(s)->read_some(d, n);
          }),
          discard_([](void* s) noexcept {
              static_cast<Derived*>(s)->discard_stale_input();
          }),
          now_us_([](void* s) noexcept { return static_cast<Derived*>(s)->now_us(); }),
          delay_ms_([](void* s, uint32_t ms) noexcept {
              if constexpr (std::is_base_of<Transport<Derived>, Derived>::value) {
                  static_cast<Derived*>(s)->delay_ms(ms);
              } else {
                  (void)s;
                  (void)ms;
              }
          }),
          yield_([](void* s) noexcept {
              if constexpr (std::is_base_of<Transport<Derived>, Derived>::value) {
                  static_cast<Derived*>(s)->yield();
              } else {
                  (void)s;
              }
          }) {}

    bool valid() const noexcept { return self_ != nullptr; }

    DriverError open(const TransportConfig& cfg) const noexcept { return open_(self_, cfg); }
    void close() const noexcept { close_(self_); }
    DriverError set_baud_rate(uint32_t baud) const noexcept { return set_baud_(self_, baud); }
    TransportIo write_some(const uint8_t* data, std::size_t n) const noexcept {
        return write_some_(self_, data, n);
    }
    DriverError finish_transmit() const noexcept { return finish_transmit_(self_); }
    TransportIo read_some(uint8_t* out, std::size_t n) const noexcept {
        return read_some_(self_, out, n);
    }
    void discard_stale_input() const noexcept { discard_(self_); }
    uint64_t now_us() const noexcept { return now_us_(self_); }
    void delay_ms(uint32_t ms) const noexcept { delay_ms_(self_, ms); }
    void yield() const noexcept { yield_(self_); }

private:
    void* self_{nullptr};
    DriverError (*open_)(void*, const TransportConfig&) noexcept {nullptr};
    void (*close_)(void*) noexcept {nullptr};
    DriverError (*set_baud_)(void*, uint32_t) noexcept {nullptr};
    TransportIo (*write_some_)(void*, const uint8_t*, std::size_t) noexcept {nullptr};
    DriverError (*finish_transmit_)(void*) noexcept {nullptr};
    TransportIo (*read_some_)(void*, uint8_t*, std::size_t) noexcept {nullptr};
    void (*discard_)(void*) noexcept {nullptr};
    uint64_t (*now_us_)(void*) noexcept {nullptr};
    void (*delay_ms_)(void*, uint32_t) noexcept {nullptr};
    void (*yield_)(void*) noexcept {nullptr};
};

}  // namespace dynamixel
