cmake_minimum_required(VERSION 3.1...3.28)
project(Optrode)

set(CMAKE_CXX_STANDARD 17)

set(dir "optrode")
set(lib_dir "lib_optrode")

# BOOST
find_package(Boost 1.65.1 REQUIRED COMPONENTS log log_setup thread filesystem system regex)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(demo ${dir}/main.cpp ${lib_dir}/SerialPort.cpp ${lib_dir}/axi_dma.c)
    target_link_libraries(demo ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Boost not found")
endif()
