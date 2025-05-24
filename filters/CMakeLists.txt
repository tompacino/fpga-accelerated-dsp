cmake_minimum_required(VERSION 3.1...3.28)
set(CMAKE_CXX_STANDARD 20)

add_library(filters lms.h leakyIntegrator.h)
set_target_properties(filters PROPERTIES LINKER_LANGUAGE CXX)
