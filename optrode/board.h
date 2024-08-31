#pragma once

#include "AxiStreamDma.h"

namespace LRB
{
    class Board
    {
    public:
        AxiStreamDma dma;

        Board(AxiStreamDmaAddresses addresses);
        void logDebugInformation();

    private:
        void init();
        void programLogic();
    };
}
