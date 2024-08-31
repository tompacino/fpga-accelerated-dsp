#include <stdio.h>
#include <stdlib.h>
#include <optional>
#include <sstream>
#include <iostream>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>

#include <boost/log/trivial.hpp>

#include "config.h"
#include "board.h"

namespace LRB
{

    Board::Board(AxiStreamDmaAddresses addresses) : dma(addresses){};

    void Board::logDebugInformation()
    {
        std::ostringstream debugStream;
        debugStream << "\n\tLRB.dmaAddresses:"

                    << "\n\t\tctrl_baddr         " << dma.addresses.ctrl_baddr
                    << "\n\t\tctrl_asize         " << dma.addresses.ctrl_asize
                    << "\n\t\tmm2s_baddr         " << dma.addresses.mm2s_baddr
                    << "\n\t\ts2mm_baddr         " << dma.addresses.s2mm_baddr
                    << "\n\t\tsaxi_asize         " << dma.addresses.saxi_asize;

        BOOST_LOG_TRIVIAL(debug) << debugStream.str();
    };

    void Board::init()
    {
        BOOST_LOG_TRIVIAL(info) << "\n\tLRB.init():";
        logDebugInformation();

    };

    void Board::programLogic()
    {
        BOOST_LOG_TRIVIAL(info) << "\n\n\tProgramming PL:";
        try
        {
            BOOST_LOG_TRIVIAL(info) << "\n\n\tUnloading...";
            // system(pl_unload);
        }
        catch (...)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to unload PL";
            throw std::runtime_error("Failed to unload PL");
        }
        try
        {
            BOOST_LOG_TRIVIAL(info) << "\n\n\tLoading default configuration...";
            // system(pl_load_default);
        }
        catch (...)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to load PL";
            throw std::runtime_error("Failed to load PL");
        }
    };
}
