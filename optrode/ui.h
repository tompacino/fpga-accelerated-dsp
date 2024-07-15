#pragma once

#include <optional>

#include <boost/log/trivial.hpp>
#include "../lib_optrode/SerialPort.hpp"

#define DEVICE "/dev/ttyACM0"

using namespace mn::CppLinuxSerial;

namespace ui
{
    std::optional<SerialPort> start()
    {
        BOOST_LOG_TRIVIAL(info) << "Optrode main process starting. Initialising serial device " << DEVICE;

        SerialPort serialPort(DEVICE, BaudRate::B_57600, NumDataBits::EIGHT, Parity::EVEN, NumStopBits::ONE, HardwareFlowControl::ON, SoftwareFlowControl::OFF);

        try {
            serialPort.Available();
        } catch (...) {
            BOOST_LOG_TRIVIAL(error) << "Serial device " << DEVICE << " not available";
            return std::nullopt;
        }

        BOOST_LOG_TRIVIAL(error) << "Serial device " << DEVICE << " available";

        return serialPort;
    }
}
