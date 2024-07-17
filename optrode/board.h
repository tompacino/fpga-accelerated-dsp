#pragma once

namespace LRB
{
    enum class Status
    {
        STOPPED,
        ACTIVE
    };

    class Board
    {

    public:
        Board() {};
        ~Board() {};

        Status getStatus();

    };
}


