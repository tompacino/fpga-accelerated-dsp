#pragma once

#include <optional>

#include <boost/log/trivial.hpp>

#include "SerialPort.hpp"

#define DEVICE "/dev/ttyACM0"

namespace mn::CppLinuxSerial
{
    namespace ui
    {
        std::optional<SerialPort> start();
    }
}


