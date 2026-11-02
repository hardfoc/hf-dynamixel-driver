// Links against the installed package only: public headers, version header and the static library.
#include "dynamixel.hpp"

#include <cstdio>

int main() {
    std::printf("hf-dynamixel SDK %s\n", dynamixel::GetSdkTag());
    return dynamixel::IsDeviceId(dynamixel::kMaxDeviceId) ? 0 : 1;
}
