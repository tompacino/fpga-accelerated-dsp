#pragma once

enum class WriterStatus
{
    STOPPED,
    WRITING,
};

class DataWriter
{
    WriterStatus status;

public:
    DataWriter();
};
