#include <optional>

#include <boost/log/trivial.hpp>

#include "SerialPort.hpp"

#include "config.h"
#include "ui.h"

namespace mn::CppLinuxSerial::ui
{
    std::optional<SerialPort> start()
    {
        BOOST_LOG_TRIVIAL(info) << "Optrode main process starting. Initialising serial device " << SERIAL_DEVICE;

        SerialPort serialPort(SERIAL_DEVICE, BaudRate::B_57600, NumDataBits::EIGHT, Parity::EVEN, NumStopBits::ONE, HardwareFlowControl::ON, SoftwareFlowControl::OFF);

        try
        {
            serialPort.Available();
        }
        catch (...)
        {
            BOOST_LOG_TRIVIAL(error) << "Serial device " << SERIAL_DEVICE << " not available";
            return std::nullopt;
        }

        BOOST_LOG_TRIVIAL(error) << "Serial device " << SERIAL_DEVICE << " available";

        return serialPort;
    }
}
