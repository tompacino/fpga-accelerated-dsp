#include <stdio.h>
#include <stdlib.h>
#include <optional>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>

#include <fcntl.h>
#include <sys/mman.h>

#include <boost/log/trivial.hpp>

#include "config.h"
#include "AxiStreamDma.h"

void AxiStreamDma::dma_s2mm_status(volatile uint32_t *virtual_addr)
{
    uint32_t status = read(virtual_addr, S2MM_STATUS_REGISTER);

    printf("Stream to memory-mapped status (0x%08x@0x%02x):", status, S2MM_STATUS_REGISTER);

    if (status & STATUS_HALTED) printf(" Halted.\n");
    else printf(" Running.\n");
    if (status & STATUS_IDLE) printf(" Idle.\n");
    if (status & STATUS_SG_INCLDED) printf(" SG is included.\n");
    if (status & STATUS_DMA_INTERNAL_ERR) printf(" DMA internal error.\n");
    if (status & STATUS_DMA_SLAVE_ERR) printf(" DMA slave error.\n");
    if (status & STATUS_DMA_DECODE_ERR) printf(" DMA decode error.\n");
    if (status & STATUS_SG_INTERNAL_ERR) printf(" SG internal error.\n");
    if (status & STATUS_SG_SLAVE_ERR) printf(" SG slave error.\n");
    if (status & STATUS_SG_DECODE_ERR) printf(" SG decode error.\n");
    if (status & STATUS_IOC_IRQ) printf(" IOC interrupt occurred.\n");
    if (status & STATUS_DELAY_IRQ) printf(" Interrupt on delay occurred.\n");
    if (status & STATUS_ERR_IRQ) printf(" Error interrupt occurred.\n");
}

void AxiStreamDma::dma_mm2s_status(volatile uint32_t *virtual_addr)
{
    uint32_t status = read(virtual_addr, MM2S_STATUS_REGISTER);

    printf("Memory-mapped to stream status (0x%08x@0x%02x):", status, MM2S_STATUS_REGISTER);

    if (status & STATUS_HALTED) printf(" Halted.\n");
    else printf(" Running.\n");
    if (status & STATUS_IDLE) printf(" Idle.\n");
    if (status & STATUS_SG_INCLDED) printf(" SG is included.\n");
    if (status & STATUS_DMA_INTERNAL_ERR) printf(" DMA internal error.\n");
    if (status & STATUS_DMA_SLAVE_ERR) printf(" DMA slave error.\n");
    if (status & STATUS_DMA_DECODE_ERR) printf(" DMA decode error.\n");
    if (status & STATUS_SG_INTERNAL_ERR) printf(" SG internal error.\n");
    if (status & STATUS_SG_SLAVE_ERR) printf(" SG slave error.\n");
    if (status & STATUS_SG_DECODE_ERR) printf(" SG decode error.\n");
    if (status & STATUS_IOC_IRQ) printf(" IOC interrupt occurred.\n");
    if (status & STATUS_DELAY_IRQ) printf(" Interrupt on delay occurred.\n");
    if (status & STATUS_ERR_IRQ) printf(" Error interrupt occurred.\n");
}

AxiStreamDma::AxiStreamDma(AxiStreamDmaAddresses addresses) : addresses(addresses)
{
    std::ostringstream debugStream;
    debugStream << "\n\tInitialise AxiStreamDma:";

    ddr_memory_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (ddr_memory_fd < 0)
    {
        ddr_memory_fd = std::nullopt;
        status = Status::ERROR;
        throw std::runtime_error("Failed to open /dev/mem");
    }
    debugStream << "\n\t\tdma.ddr_memory_fd " << ddr_memory_fd.value();

    ctrl_vaddr = std::make_unique<Mmap>(nullptr, addresses.ctrl_asize, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory_fd.value(), addresses.ctrl_baddr);
    if (ctrl_vaddr->mem == MAP_FAILED)
    {
        ctrl_vaddr.reset();
        status = Status::ERROR;
        throw std::runtime_error("Failed to mmap ctrl_baddr");
    }
    else
    {
        debugStream << "\n\t\tdma.ctrl_vaddr    " << ctrl_vaddr
                    << "\n\t\tdma.ctrl_asize    " << addresses.ctrl_asize;
    }

    mm2s_vaddr = std::make_unique<Mmap>(nullptr, addresses.saxi_asize, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory_fd.value(), addresses.mm2s_baddr);
    if (mm2s_vaddr->mem == MAP_FAILED)
    {
        mm2s_vaddr.reset();
        status = Status::ERROR;
        throw std::runtime_error("Failed to mmap mm2s_baddr");
    }
    else
    {
        debugStream << "\n\t\tdma.mm2s_vaddr    " << mm2s_vaddr
                    << "\n\t\tdma.saxi_asize    " << addresses.saxi_asize;
    }

    s2mm_vaddr = std::make_unique<Mmap>(nullptr, addresses.saxi_asize, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory_fd.value(), addresses.s2mm_baddr);
    if (s2mm_vaddr->mem == MAP_FAILED)
    {
        s2mm_vaddr.reset();
        status = Status::ERROR;
        throw std::runtime_error("Failed to mmap s2mm_baddr");
    }
    else
    {
        debugStream << "\n\t\tdma.s2mm_vaddr    " << s2mm_vaddr
                    << "\n\t\tdma.saxi_asize    " << addresses.saxi_asize;
    }

    write(ctrl_vaddr->mem, MM2S_CONTROL_REGISTER, RUN_DMA);
    write(ctrl_vaddr->mem, S2MM_CONTROL_REGISTER, RUN_DMA);
    status = Status::INITIALISED;
    BOOST_LOG_TRIVIAL(debug) << debugStream.str();
};

int AxiStreamDma::spoofData(int bytesToTransfer)
{
    mm2s_vaddr->mem[0] = 0xDEADBEEF;
    mm2s_vaddr->mem[1] = 0x12345678;

    int bytesTransferred = 0;
    for (; bytesTransferred < bytesToTransfer; bytesTransferred += transfer_size_bytes)
    {
        write(ctrl_vaddr->mem, MM2S_CONTROL_REGISTER, RESET_DMA);
		write(ctrl_vaddr->mem, MM2S_CONTROL_REGISTER, HALT_DMA);
		write(ctrl_vaddr->mem, MM2S_CONTROL_REGISTER, ENABLE_ALL_IRQ);
        write(ctrl_vaddr->mem, MM2S_CONTROL_REGISTER, RUN_DMA);
        write(ctrl_vaddr->mem, MM2S_SRC_ADDRESS_REGISTER, mm2s_baddr);
        write(ctrl_vaddr->mem, MM2S_TRNSFR_LENGTH_REGISTER, transfer_size_bytes);
        sync(ctrl_vaddr->mem, MM2S_STATUS_REGISTER);
        dma_s2mm_status(ctrl_vaddr->mem);
        dma_mm2s_status(ctrl_vaddr->mem);
    }
    return bytesTransferred;
}

int AxiStreamDma::sendData(std::vector<int32_t> &data, uint32_t idx)
{
    mm2s_vaddr->mem[0] = data[idx];
    mm2s_vaddr->mem[1] = data[idx + 1];

    write(ctrl_vaddr->mem, MM2S_CONTROL_REGISTER, RESET_DMA);
    write(ctrl_vaddr->mem, MM2S_CONTROL_REGISTER, HALT_DMA);
    write(ctrl_vaddr->mem, MM2S_CONTROL_REGISTER, ENABLE_ALL_IRQ);
    write(ctrl_vaddr->mem, MM2S_CONTROL_REGISTER, RUN_DMA);
    write(ctrl_vaddr->mem, MM2S_SRC_ADDRESS_REGISTER, mm2s_baddr);
    write(ctrl_vaddr->mem, MM2S_TRNSFR_LENGTH_REGISTER, transfer_size_bytes);
    sync(ctrl_vaddr->mem, MM2S_STATUS_REGISTER);
    dma_s2mm_status(ctrl_vaddr->mem);
    dma_mm2s_status(ctrl_vaddr->mem);

    return transfer_size_bytes;
}

int AxiStreamDma::fillBuffer()
{
    int bytesTransferred = 0;
    while (read(ctrl_vaddr->mem, S2MM_STATUS_REGISTER) != 0 && read(ctrl_vaddr->mem, S2MM_STATUS_REGISTER) != STATUS_IOC_IRQ)
    {
        write(ctrl_vaddr->mem, S2MM_CONTROL_REGISTER, RESET_DMA);
        write(ctrl_vaddr->mem, S2MM_CONTROL_REGISTER, HALT_DMA);
        write(ctrl_vaddr->mem, S2MM_CONTROL_REGISTER, ENABLE_ALL_IRQ);
        write(ctrl_vaddr->mem, S2MM_CONTROL_REGISTER, RUN_DMA);
        write(ctrl_vaddr->mem, S2MM_DST_ADDRESS_REGISTER, s2mm_baddr + bytesTransferred);
        write(ctrl_vaddr->mem, S2MM_BUFF_LENGTH_REGISTER, transfer_size_bytes);
        sync(ctrl_vaddr->mem, S2MM_STATUS_REGISTER);
        dma_s2mm_status(ctrl_vaddr->mem);
        dma_mm2s_status(ctrl_vaddr->mem);
        bytesTransferred += transfer_size_bytes;
    }

    if (bytesTransferred > 0)
    {
        size_t bufferSize = buffer.size();
        buffer.resize(bufferSize + bytesTransferred / 4);
        std::memcpy(buffer.data() + bufferSize,  const_cast<const unsigned int *>(s2mm_vaddr->mem), bytesTransferred);
    }

    return bytesTransferred;
}

unsigned int AxiStreamDma::read(volatile unsigned int *virtual_addr, int offset)
{
	return virtual_addr[offset >> 2];
};

void AxiStreamDma::write(volatile unsigned int *virtual_addr, int offset, unsigned int value)
{
	virtual_addr[offset >> 2] = value;
};

void AxiStreamDma::sync(volatile unsigned int *virtual_addr, int status_register)
{
    volatile unsigned int status = read(virtual_addr, status_register);
    // Wait until both IOC_IRQ and IDLE are set
    while (( (status & IOC_IRQ_FLAG) == 0 ) || ( (status & IDLE_FLAG) == 0 ))
    {
        status = read(virtual_addr, status_register);
    }
};
