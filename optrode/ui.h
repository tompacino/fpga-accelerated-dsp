#pragma once

#include <optional>
#include "SerialPort.hpp"

namespace mn::CppLinuxSerial
{
    namespace ui
    {
        std::optional<SerialPort> start();
    }
}
