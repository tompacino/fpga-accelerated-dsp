add_library(external SerialPort.cpp axi_dma.c)
target_include_directories(external PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
