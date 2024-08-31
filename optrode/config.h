#pragma once

#include "board.h"

constexpr auto SERIAL_DEVICE = "/dev/sda1";

// PL configuration
constexpr auto pl_unload = "/usr/firmware/pl_unload.sh";
constexpr auto pl_load_default = "/usr/firmware/pl_load_default.sh";

// memory map
constexpr uint32_t ctrl_baddr = 0x40400000;
constexpr uint32_t ctrl_asize = 0xFFFF;
constexpr uint32_t mm2s_baddr = 0x0e000000;
constexpr uint32_t s2mm_baddr = 0x0f000000;
constexpr uint32_t saxi_asize = 0xFFFF;

constexpr size_t transfer_size_bytes = 8;

// data collection
constexpr std::string FILENAME_DATACOLLECTION = "data_coll";
constexpr auto TOTAL_CHANNELS_DATACOLLECTION = 4U;
constexpr auto BIT_DEPTH_DATACOLLECTION = 24U;
constexpr auto SPS_DATACOLLECTION = 64000U;
constexpr auto BUFFER_SAVE_SECONDS_DATACOLLECTION = 30U;

// recordings
constexpr std::string FILENAME_ANC = "optrode_record";
constexpr auto NUM_CHANNELS = 1U;
constexpr auto BIT_DEPTH = 24;
constexpr auto SAMPLE_RATE = 10000;

constexpr auto RECORD_ALL_CHANNELS = true;
constexpr auto RECORD_SAVE_INTERVAL_SECONDS = 1U;

constexpr auto TOTAL_CHANNELS = RECORD_ALL_CHANNELS ? 4 * NUM_CHANNELS : NUM_CHANNELS;

constexpr std::size_t MEM_BYTES = 1024 * 1024 * 32; // 64MB this should be inherited from system...
constexpr std::size_t BYTES_PER_SAMPLE = 4U;
constexpr std::size_t MEMORY_PER_SECOND = BYTES_PER_SAMPLE * 3 * NUM_CHANNELS * SAMPLE_RATE;
constexpr std::size_t SEGMENT_MEMORY = MEMORY_PER_SECOND * RECORD_SAVE_INTERVAL_SECONDS;
constexpr auto RECORD_FILE_INTERVAL = MEM_BYTES / SEGMENT_MEMORY;

static_assert(RECORD_FILE_INTERVAL > 0, "Insufficient memory for recording!");

constexpr auto SAMPLES_PER_FILE = TOTAL_CHANNELS * SAMPLE_RATE * RECORD_SAVE_INTERVAL_SECONDS * RECORD_FILE_INTERVAL;
