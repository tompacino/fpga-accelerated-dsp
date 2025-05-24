cmake_minimum_required(VERSION 3.1...3.28)
set(CMAKE_CXX_STANDARD 20)
project(Optrode)

# Boost Library
find_package(Boost 1.65.1 REQUIRED COMPONENTS log log_setup thread filesystem system regex)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found")
endif()
include_directories(${Boost_INCLUDE_DIRS})

# The project requires CMake Package Manager, which is used to fetch the following required packages:
#     Fixed Point Math Library
#     Compile Time Math
#     GNU Scientific Library
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/cmake/CPM.cmake")
   message(STATUS "CPM.cmake not found, downloading...")
   file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake")
   file(DOWNLOAD
           "https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake"
           "${CMAKE_SOURCE_DIR}/cmake/CPM.cmake"
   )
endif()
include(cmake/CPM.cmake)

# Cache for CPM
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/.cpmcache")
   file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/.cpmcache")
endif()
set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpmcache")

message(STATUS "CPM fetching required packages...")
CPMAddPackage(
        NAME cnl
        GITHUB_REPOSITORY johnmcfarlane/cnl
        GIT_TAG        v1.1.2
)
CPMAddPackage(
  NAME fixed_math
  GITHUB_REPOSITORY arturbac/fixed_math
  GIT_TAG        v1.0.0
)
CPMAddPackage(
  NAME gcem
  GITHUB_REPOSITORY kthohr/gcem
  GIT_TAG        v1.18.0
)
CPMAddPackage(
  NAME gsl
  GITHUB_REPOSITORY ampl/gsl
  GIT_TAG        20211111
  OPTIONS "BUILDLIBS statistics" "NO_AMPL_BINDINGS on" "GSL_DISABLE_TESTS on" "DOCUMENTATION off"
)

message(STATUS "CPM adding required packages...")
find_package(cnl REQUIRED)
find_package(fixed_math REQUIRED)
find_package(gcem REQUIRED)
find_package(gsl REQUIRED)

# The project also requires header only library AudioFile.h
set(AUDIOFILE_DIR "${CMAKE_BINARY_DIR}/_deps/AudioFile")
if(NOT EXISTS "${AUDIOFILE_DIR}/AudioFile.h")
   message(STATUS "AudioFile.h not found, cloning the AudioFile repository...")
   execute_process(
           COMMAND git clone https://github.com/adamstark/AudioFile.git "${AUDIOFILE_DIR}"
           WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
           RESULT_VARIABLE result
   )
   if(NOT result EQUAL "0")
      message(FATAL_ERROR "Failed to clone the AudioFile repository. Please manually add AudioFile.h")
   endif()
endif()

# src
add_subdirectory(optrode)
add_subdirectory(filters)

# software filter demo
add_executable(lmsDemo lmsDemo.cpp)
target_include_directories(lmsDemo PRIVATE "${AUDIOFILE_DIR}")
target_link_libraries(lmsDemo PUBLIC filters gcem Cnl gsl)

# soc-fpga-dsp-platorm
add_executable(record main.cpp)
target_include_directories(record PRIVATE "${AUDIOFILE_DIR}")
target_link_libraries(record PUBLIC optrode filters)

# data-collection
add_executable(datacollection datacollection.cpp)
target_include_directories(datacollection PRIVATE "${AUDIOFILE_DIR}")
target_link_libraries(datacollection PUBLIC optrode filters)

# dma-test
add_executable(dmatest damtest/dmatest.c)
target_link_libraries(dmatest PUBLIC)

# axidma test
add_executable(test dmatest/test.c)
target_link_libraries(test PUBLIC)
