#include <iostream>
#include <thread>
#include <optional>
#include <csignal>

#include <boost/log/trivial.hpp>

#include "optrode/board.h"
#include "optrode/datawriter.h"
#include "optrode/ui.h"

#define STATISTIC_INTERVAL 1024

using namespace mn::CppLinuxSerial;

struct TransferStatistic
{
    int packetSize;
    int packetSent;

    TransferStatistic(int interval) : packetSize(interval), packetSent(0) {};

    int total()
    {
        return packetSize * packetSent;
    };
};

std::ostream& operator<<(std::ostream& os, TransferStatistic& ts)
{
    os << "Packets sent: " << ts.packetSent << "\tTotal sent: " << ts.total();
    return os;
}

volatile std::sig_atomic_t stopSignal = 0;

void handler(int signal) {
    if (signal == SIGINT) {
        BOOST_LOG_TRIVIAL(info) << "Stopping data feed safely...";
        stopSignal = 1;
    }
}

int main() {
    BOOST_LOG_TRIVIAL(info) << "Optrode main process starting. Initialising serial device " << DEVICE;

    SerialPort serialPort(DEVICE, BaudRate::B_57600, NumDataBits::EIGHT, Parity::EVEN, NumStopBits::ONE, HardwareFlowControl::ON, SoftwareFlowControl::OFF);

    // serialPort.Open();

    // if (!serialPort.Available())
    // {
    //     BOOST_LOG_TRIVIAL(error) << "Serial device " << DEVICE << " not available!";
    //     return -1;
    // }
    // BOOST_LOG_TRIVIAL(info) << "Serial device " << DEVICE << " available";

    LRB::Board fpga = LRB::Board();
    if (fpga.getStatus() != LRB::Status::ACTIVE)
    {
        BOOST_LOG_TRIVIAL(error) << "FPGA LRB not available!";
        return -1;
    }
    BOOST_LOG_TRIVIAL(error) << "FPGA LRB available";

    bool active = fpga.getStatus() == LRB::Status::ACTIVE;

    TransferStatistic stats(STATISTIC_INTERVAL);
    BOOST_LOG_TRIVIAL(info) << "Printing transfer statistic every " << STATISTIC_INTERVAL << " bits";
    std::signal(SIGINT, handler);
    while (!stopSignal)
    {
        if (fpga.getStatus()!= LRB::Status::ACTIVE)
        {
            BOOST_LOG_TRIVIAL(fatal) << "FPGA LRB not available!";
            return -1;
        }
        // check for serial...

        // send packet

        stats.packetSent++;
        if (!stats.total() % STATISTIC_INTERVAL)
            std::cout << "\r" << stats << std::flush;
        // BOOST_LOG_TRIVIAL(info) << stats;
    }
    return 0;
};

// enum severity_level
// {
//     trace,
//     debug,
//     info,
//     warning,
//     error,
//     fatal
// };
