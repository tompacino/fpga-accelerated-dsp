#pragma once

#include <cstdint>
#include <memory>

#include <fcntl.h>
#include <sys/mman.h>

#define MM2S_CONTROL_REGISTER 0x00
#define MM2S_STATUS_REGISTER 0x04
#define MM2S_SRC_ADDRESS_REGISTER 0x18
#define MM2S_TRNSFR_LENGTH_REGISTER 0x28

#define S2MM_CONTROL_REGISTER 0x30
#define S2MM_STATUS_REGISTER 0x34
#define S2MM_DST_ADDRESS_REGISTER 0x48
#define S2MM_BUFF_LENGTH_REGISTER 0x58

#define IOC_IRQ_FLAG 1 << 12
#define IDLE_FLAG 1 << 1

#define STATUS_HALTED 0x00000001
#define STATUS_IDLE 0x00000002
#define STATUS_SG_INCLDED 0x00000008
#define STATUS_DMA_INTERNAL_ERR 0x00000010
#define STATUS_DMA_SLAVE_ERR 0x00000020
#define STATUS_DMA_DECODE_ERR 0x00000040
#define STATUS_SG_INTERNAL_ERR 0x00000100
#define STATUS_SG_SLAVE_ERR 0x00000200
#define STATUS_SG_DECODE_ERR 0x00000400
#define STATUS_IOC_IRQ 0x00001000
#define STATUS_DELAY_IRQ 0x00002000
#define STATUS_ERR_IRQ 0x00004000

#define HALT_DMA 0x00000000
#define RUN_DMA 0x00000001
#define RESET_DMA 0x00000004
#define ENABLE_IOC_IRQ 0x00001000
#define ENABLE_DELAY_IRQ 0x00002000
#define ENABLE_ERR_IRQ 0x00004000
#define ENABLE_ALL_IRQ 0x00007000

struct Mmap
{
    volatile unsigned int *mem = nullptr;
    size_t size = 0;
    Mmap(void *start, size_t size, int prot, int flags, int fd, uint32_t baddr)
    {
        mem = static_cast<unsigned int *>(mmap(start, size, prot, flags, fd, baddr));
        this->size = size;
    };

    ~Mmap()
    {
        if (mem != nullptr && mem != MAP_FAILED && munmap(const_cast<unsigned int*>(mem), size) == -1)
            std::runtime_error("Failed to delete MmapPointer memory!");
    };
};

enum class Status
{
    STOPPED,
    INITIALISED,
    ACTIVE,
    ERROR
};

struct AxiStreamDmaAddresses
{
    const uint32_t ctrl_baddr;
    const uint32_t ctrl_asize;

    const uint32_t mm2s_baddr;
    const uint32_t s2mm_baddr;
    const uint32_t saxi_asize;
};

class AxiStreamDma
{
    std::optional<int> ddr_memory_fd = std::nullopt;
    std::unique_ptr<Mmap> ctrl_vaddr;
    std::unique_ptr<Mmap> mm2s_vaddr;
    std::unique_ptr<Mmap> s2mm_vaddr;

public:
    Status status = Status::STOPPED;
    std::vector<std::uint32_t> buffer;

    const AxiStreamDmaAddresses addresses;

    AxiStreamDma(AxiStreamDmaAddresses addresses);

    int spoofData(int transfers);
    int fillBuffer();

    unsigned int read(volatile unsigned int *virtual_addr, int offset);
    void write(volatile unsigned int *virtual_addr, int offset, unsigned int value);
    void sync(volatile unsigned int *virtual_addr, int status_register);
};
