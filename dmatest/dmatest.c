/*
 * Copyright (c) 2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/mman.h>

#define MM2S_CONTROL_REGISTER       0x00
#define MM2S_STATUS_REGISTER        0x04
#define MM2S_SRC_ADDRESS_REGISTER   0x18
#define MM2S_TRNSFR_LENGTH_REGISTER 0x28

#define S2MM_CONTROL_REGISTER       0x30
#define S2MM_STATUS_REGISTER        0x34
#define S2MM_DST_ADDRESS_REGISTER   0x48
#define S2MM_BUFF_LENGTH_REGISTER   0x58

#define IOC_IRQ_FLAG                1<<12
#define IDLE_FLAG                   1<<1

#define STATUS_HALTED               0x00000001
#define STATUS_IDLE                 0x00000002
#define STATUS_SG_INCLDED           0x00000008
#define STATUS_DMA_INTERNAL_ERR     0x00000010
#define STATUS_DMA_SLAVE_ERR        0x00000020
#define STATUS_DMA_DECODE_ERR       0x00000040
#define STATUS_SG_INTERNAL_ERR      0x00000100
#define STATUS_SG_SLAVE_ERR         0x00000200
#define STATUS_SG_DECODE_ERR        0x00000400
#define STATUS_IOC_IRQ              0x00001000
#define STATUS_DELAY_IRQ            0x00002000
#define STATUS_ERR_IRQ              0x00004000

#define HALT_DMA                    0x00000000
#define RUN_DMA                     0x00000001
#define RESET_DMA                   0x00000004
#define ENABLE_IOC_IRQ              0x00001000
#define ENABLE_DELAY_IRQ            0x00002000
#define ENABLE_ERR_IRQ              0x00004000
#define ENABLE_ALL_IRQ              0x00007000

uint32_t write_status(uint32_t *virtual_addr, int offset, uint32_t value)
{
    virtual_addr[offset>>2] = value;

    return 0;
}

uint32_t read_status(uint32_t *virtual_addr, int offset)
{
    return virtual_addr[offset>>2];
}

void dma_s2mm_status(uint32_t *virtual_addr)
{
    uint32_t status = read_status(virtual_addr, S2MM_STATUS_REGISTER);

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

void dma_mm2s_status(uint32_t *virtual_addr)
{
    uint32_t status = read_status(virtual_addr, MM2S_STATUS_REGISTER);

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

int dma_mm2s_sync(uint32_t *virtual_addr)
{
    uint32_t mm2s_status =  read_status(virtual_addr, MM2S_STATUS_REGISTER);

	// sit in this while loop as long as the status does not read back 0x00001002 (4098)
	// 0x00001002 = IOC interrupt has occured and DMA is idle
	while(!(mm2s_status & IOC_IRQ_FLAG) || !(mm2s_status & IDLE_FLAG))
	{
        dma_s2mm_status(virtual_addr);
        dma_mm2s_status(virtual_addr);

        mm2s_status =  read_status(virtual_addr, MM2S_STATUS_REGISTER);
    }

	return 0;
}

int dma_s2mm_sync(uint32_t *virtual_addr)
{
    uint32_t s2mm_status = read_status(virtual_addr, S2MM_STATUS_REGISTER);

	// sit in this while loop as long as the status does not read back 0x00001002 (4098)
	// 0x00001002 = IOC interrupt has occured and DMA is idle
	while (!(s2mm_status & IOC_IRQ_FLAG) || !(s2mm_status & IDLE_FLAG))
	{
        dma_s2mm_status(virtual_addr);
        dma_mm2s_status(virtual_addr);

        s2mm_status = read_status(virtual_addr, S2MM_STATUS_REGISTER);
	}

	return 0;
}

void print_mem(void *virtual_address, int byte_count) {
    uint32_t *data_ptr = (uint32_t *)virtual_address;
	printf("\n");
    for (int i = 0; i < byte_count / 4; i++) {
        printf("%08X ", data_ptr[i]);
		printf("\n");
    }
    printf("\n");
}

//void load_mem(void *virtual_address, int byte_count, uint32_t data)
//{
//	char *data_ptr = data;
//
//	for(int i=0;i<byte_count;i++){
//		data_ptr[i] = data;
//	}
//
//	memset(virtual_address, *data_ptr, byte_count);
//}

int main()
{
    printf("Hello World! - Running DMA transfer test application.\n");

	printf("Opening a character device file of the Arty's DDR memeory...\n");
	int ddr_memory = open("/dev/mem", O_RDWR | O_SYNC);

	printf("Memory map the address of the DMA AXI IP via its AXI lite control interface register block.\n");
    uint32_t *dma_virtual_addr = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, 0x40400000);

	printf("Memory map the MM2S source address register block.\n");
    uint32_t *virtual_src_addr  = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, 0x0e000000);

	printf("Memory map the S2MM destination address register block.\n");
    uint32_t *virtual_dst_addr = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, 0x0f000000);

	printf("Writing random data to source register block...\n");
	const int nbytes = 32;
	virtual_src_addr[0] = 0xEFBEADDE;
	virtual_src_addr[1] = 0x11223344;
	virtual_src_addr[2] = 0xABABABAB;
	virtual_src_addr[3] = 0xCDCDCDCD;
	virtual_src_addr[4] = 0x00001111;
	virtual_src_addr[5] = 0x22223333;
	virtual_src_addr[6] = 0x44445555;
	virtual_src_addr[7] = 0x66667777;

	printf("Clearing the destination register block...\n");
    memset(virtual_dst_addr, 0, nbytes);


    printf("Source memory block data:      \n");
	print_mem(virtual_src_addr, nbytes);

    printf("Destination memory block data: \n");
	print_mem(virtual_dst_addr, nbytes);

	// Initialise
    printf("Reset the DMA.\n");
    write_status(dma_virtual_addr, MM2S_CONTROL_REGISTER, RESET_DMA);
	printf("Halt the DMA.\n");
	write_status(dma_virtual_addr, MM2S_CONTROL_REGISTER, HALT_DMA);
	printf("Enable all interrupts.\n");
	write_status(dma_virtual_addr, MM2S_CONTROL_REGISTER, ENABLE_ALL_IRQ);
	printf("Run the MM2S channel.\n");
	write_status(dma_virtual_addr, MM2S_CONTROL_REGISTER, RUN_DMA);

	dma_s2mm_status(dma_virtual_addr);
    dma_mm2s_status(dma_virtual_addr);

	printf("Reset the DMA.\n");
	write_status(dma_virtual_addr, S2MM_CONTROL_REGISTER, RESET_DMA);
	printf("Halt the DMA.\n");
    write_status(dma_virtual_addr, S2MM_CONTROL_REGISTER, HALT_DMA);
	printf("Enable all interrupts.\n");
    write_status(dma_virtual_addr, S2MM_CONTROL_REGISTER, ENABLE_ALL_IRQ);
	printf("Run the S2MM channel.\n");
    write_status(dma_virtual_addr, S2MM_CONTROL_REGISTER, RUN_DMA);

	dma_s2mm_status(dma_virtual_addr);
    dma_mm2s_status(dma_virtual_addr);


	// MM2S
	write_status(dma_virtual_addr, MM2S_CONTROL_REGISTER, RUN_DMA);
	printf("Writing source address of the data from MM2S in DDR...\n");
    write_status(dma_virtual_addr, MM2S_SRC_ADDRESS_REGISTER, 0x0e000000);
	printf("Writing MM2S transfer length of %d bytes...\n", nbytes);
    write_status(dma_virtual_addr, MM2S_TRNSFR_LENGTH_REGISTER, nbytes);
	printf("Waiting for MM2S synchronization...\n");
    dma_mm2s_sync(dma_virtual_addr);

	dma_s2mm_status(dma_virtual_addr);
    dma_mm2s_status(dma_virtual_addr);

	// S2MM
	write_status(dma_virtual_addr, S2MM_CONTROL_REGISTER, RUN_DMA);
	printf("Writing the destination address for the data from S2MM in DDR...\n");
	write_status(dma_virtual_addr, S2MM_DST_ADDRESS_REGISTER, 0x0f000000);
	printf("Writing S2MM transfer length of %d bytes...\n", nbytes);
    write_status(dma_virtual_addr, S2MM_BUFF_LENGTH_REGISTER, nbytes);
	printf("Waiting for S2MM sychronization...\n");
    dma_s2mm_sync(dma_virtual_addr);

	dma_s2mm_status(dma_virtual_addr);
    dma_mm2s_status(dma_virtual_addr);

    printf("Destination memory block: ");
	print_mem(virtual_dst_addr, nbytes);

	printf("\n");

    return 0;
}
