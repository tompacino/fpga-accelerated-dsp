#include <iostream>
#include <thread>
#include <optional>
#include <csignal>
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>
#include <cstdint>

#include <boost/log/trivial.hpp>

#include "AudioFile.h"
#include "board.h"
#include "datawriter.h"
#include "ui.h"
#include "config.h"

using namespace mn::CppLinuxSerial;

struct Recording
{
    std::string name;
    std::size_t file;
    std::size_t bytesPerSample;
    std::size_t samplesPerFile;
    std::size_t recordedSamples;

    Recording(std::string name, std::size_t bytesPerSample, std::size_t samplesPerFile) : name(name), file(0), bytesPerSample(bytesPerSample), samplesPerFile(samplesPerFile), recordedSamples(0) {};

    std::string filename() const
    {
        return name + '_' + std::to_string(file) + ".wav";
    }

    std::size_t total() const
    {
        return bytesPerSample * (file * samplesPerFile + recordedSamples);
    };

    std::string to_str() const
    {
        std::stringstream ss;
        ss << name << " - " << bytesPerSample << " bytes per sample - " << samplesPerFile << " samples per file";
        return ss.str();
    }
};

std::ostream &operator<<(std::ostream &os, Recording &r)
{
    os << "Audio file: "
       << r.filename()
       << "\tSamples this file: "
       << r.recordedSamples
       << "\tTotal size in bytes: "
       << r.total();
    return os;
}

volatile std::sig_atomic_t recordingStopSignal = 0;

void handler(int signal)
{
    if (signal == SIGINT)
    {
        BOOST_LOG_TRIVIAL(info) << "Stopping data feed safely...";
        recordingStopSignal = 1;
    }
}

template <typename T>
void initialiseAudioFile(AudioFile<T> &af, typename AudioFile<T>::AudioBuffer &ab, Recording &r)
{
    af.setNumChannels(TOTAL_CHANNELS_DATACOLLECTION);
    af.setSampleRate(SPS_DATACOLLECTION);
    af.setBitDepth(BIT_DEPTH_DATACOLLECTION);

    af.setAudioBuffer(ab);
    af.save(r.filename(), AudioFileFormat::Wave);

    BOOST_LOG_TRIVIAL(debug)
        << "init audiofile " << r.filename();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        BOOST_LOG_TRIVIAL(error) << "Usage: " << argv[0] << " <wav_file_name>";
        return 1;
    }

    std::string wavFileName = argv[1];
    std::ifstream file(wavFileName);
    if (!file.good()) {
        BOOST_LOG_TRIVIAL(error) << "File does not exist or cannot be opened.";
        return 2;
    }

    AudioFile<int32_t> inputFile;
    if (!inputFile.load(wavFileName)) {
        BOOST_LOG_TRIVIAL(error) << "Unable to load the WAV file.";
        return 3;
    }

    if (inputFile.getBitDepth() != BIT_DEPTH_DATACOLLECTION) {
        BOOST_LOG_TRIVIAL(error) << "Bit depth is not " << BIT_DEPTH << " bits.";
        return 4;
    }

    if (inputFile.getSampleRate() < SPS_DATACOLLECTION) {
        BOOST_LOG_TRIVIAL(error) << "Sample rate is less than " << SPS_DATACOLLECTION << " Hz.";
        return 5;
    }

    BOOST_LOG_TRIVIAL(info) << "WAV file loaded successfully and meets requirements.";

    std::ostringstream logStream;
    logStream << "\n\tdatacollection:               " << wavFileName
              << "\n\tinit serial device            " << SERIAL_DEVICE
              << "\n\ttotal_channels                " << TOTAL_CHANNELS_DATACOLLECTION
              << "\n\tmem_bytes                     " << MEM_BYTES
              << "\n\tbytes_per_sample              " << BYTES_PER_SAMPLE
              << "\n\tsamples_per_file              " << SAMPLES_PER_FILE
              << "\n\trecord_file_interval          " << RECORD_FILE_INTERVAL;

    BOOST_LOG_TRIVIAL(info) << logStream.str();

    auto fpga = LRB::Board(AxiStreamDmaAddresses(ctrl_baddr, ctrl_asize, mm2s_baddr, s2mm_baddr, saxi_asize));

    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm now_tm = *std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d_%H-%M-%S");
    std::string name = FILENAME_DATACOLLECTION + '_' + oss.str();

    Recording r(name, BYTES_PER_SAMPLE, SAMPLES_PER_FILE);

    // prep audio buffer
    constexpr std::size_t BUFFER_LENGTH = SPS_DATACOLLECTION * BUFFER_SAVE_SECONDS_DATACOLLECTION;
    std::size_t currentChannel = 0;
    AudioFile<int32_t>::AudioBuffer audioBuffer;
    audioBuffer.resize(TOTAL_CHANNELS_DATACOLLECTION);
    std::ranges::for_each(audioBuffer, [&BUFFER_LENGTH](auto &b)
                          { b.reserve(BUFFER_LENGTH); });

    // prep audio file
    AudioFile<int32_t> audioFile;
    initialiseAudioFile(audioFile, audioBuffer, r);

    BOOST_LOG_TRIVIAL(info) << r.to_str();
    std::signal(SIGINT, handler);

    uint64_t inputIdx = 0;
    uint64_t inputFileNumSamples = inputFile.getNumSamplesPerChannel();
    BOOST_LOG_TRIVIAL(info) << "inputFileNumSamples " << inputFileNumSamples;
    inputFileNumSamples = (inputFileNumSamples % 2 == 0) ? inputFileNumSamples : inputFileNumSamples - 1;
    BOOST_LOG_TRIVIAL(info) << "inputFileNumSamples updated to " << inputFileNumSamples;
    fpga.dma.sendData(inputFile.samples[0], inputIdx);
    inputIdx += 2;
    while (inputIdx < inputFileNumSamples && !recordingStopSignal)
    {
        if (fpga.dma.status == Status::ERROR)
        {
            BOOST_LOG_TRIVIAL(fatal) << "FPGA LRB not available!";
            recordingStopSignal = 1;
        }

        if (r.recordedSamples >= SAMPLES_PER_FILE)
        {
            BOOST_LOG_TRIVIAL(info) << "save audiofile " << r.filename();
            audioFile.setAudioBuffer(audioBuffer);
            audioFile.save(r.filename(), AudioFileFormat::Wave);

            r.file++;
            r.recordedSamples = 0;
            std::ranges::for_each(audioBuffer, [&BUFFER_LENGTH](auto &b)
                                  { b.clear();
                                    b.reserve(BUFFER_LENGTH); });
            initialiseAudioFile(audioFile, audioBuffer, r);
        }
        else if (r.recordedSamples == audioBuffer.begin()->capacity())
        {
            // save wip
            audioFile.setAudioBuffer(audioBuffer);
            BOOST_LOG_TRIVIAL(info) << "save audiofile " << r.filename();
            audioFile.save(r.filename(), AudioFileFormat::Wave);

            // increase buffer size
            size_t newBufferSize = audioBuffer[0].capacity() + (SPS_DATACOLLECTION * BUFFER_SAVE_SECONDS_DATACOLLECTION);
            BOOST_LOG_TRIVIAL(debug)
                << "increase audio file buffer size from "
                << audioBuffer[0].capacity()
                << " to "
                << newBufferSize;
            std::ranges::for_each(audioBuffer, [newBufferSize](auto &b)
                                  { b.reserve(newBufferSize); });
        }

        // record sample packet
        int bytesRecorded = fpga.dma.fillBuffer();

        //volatile int audioBufferLength = audioBuffer[0].size();
        auto sampleIt = fpga.dma.buffer.begin();
        while (sampleIt != fpga.dma.buffer.end())
        {
            // BOOST_LOG_TRIVIAL(info) << "got sample " << *sampleIt;
            audioBuffer[currentChannel].push_back(*sampleIt);
            sampleIt++;
            r.recordedSamples++;
            currentChannel = (currentChannel == TOTAL_CHANNELS_DATACOLLECTION - 1) ? 0 : currentChannel + 1;
        }
        fpga.dma.buffer.clear();

        // transfer audio
        // BOOST_LOG_TRIVIAL(info) << "sent samples " << inputFile.samples[0][inputIdx] << " " << inputFile.samples[0][inputIdx + 1];
        fpga.dma.sendData(inputFile.samples[0], inputIdx);
        inputIdx += 2;
    }
    // save file
    if (r.recordedSamples != 0)
    {
        BOOST_LOG_TRIVIAL(info)
            << "save audiofile " << r.filename();
        audioFile.save(r.filename(), AudioFileFormat::Wave);
    }

    BOOST_LOG_TRIVIAL(info) << "data collection completed!";

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
