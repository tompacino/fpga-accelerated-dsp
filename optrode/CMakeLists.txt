cmake_minimum_required(VERSION 3.1...3.28)
set(CMAKE_CXX_STANDARD 20)

# Boost Library
find_package(Boost 1.65.1 REQUIRED COMPONENTS log log_setup thread filesystem system regex)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found")
endif()
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(external)
add_library(optrode external board.cpp AxiStreamDma.cpp datawriter.cpp ui.cpp)
target_include_directories(optrode PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/external)
target_link_libraries(optrode PUBLIC ${Boost_LIBRARIES} external)

