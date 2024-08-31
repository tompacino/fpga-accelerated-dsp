cmake_minimum_required(VERSION 3.1...3.28)
set(CMAKE_CXX_STANDARD 20)

add_library(external SerialPort.cpp AxiDMA.cpp AxiDmaBuffer.cpp AxiDmaDescriptors.cpp)
target_include_directories(external PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
