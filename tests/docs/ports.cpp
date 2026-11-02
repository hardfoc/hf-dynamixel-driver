/**
 * @file ports.cpp
 * @brief Instantiates the documented transport in full, and the type-erased view the bus binds.
 */
#include "my_uart.hpp"

template class dynamixel::Transport<MyUart>;

dynamixel::TransportView view_of(MyUart& uart) noexcept {
  return dynamixel::TransportView{uart};
}
