cmake_minimum_required(VERSION 3.1...3.28)
project(Optrode)

set(CMAKE_CXX_STANDARD 17)

# Require Cmake Package Manager
include(cmake/CPM.cmake)

# Fixed point
CPMAddPackage(
  NAME fixed_math
  GITHUB_REPOSITORY arturbac/fixed_math
  GIT_TAG        v1.0.0
)
find_package(fixed_math REQUIRED)

# Compile Time Math
CPMAddPackage(
  NAME gcem
  GITHUB_REPOSITORY kthohr/gcem
  GIT_TAG        v1.18.0
)
find_package(gcem REQUIRED)

# GNU Scientific Library
CPMAddPackage(
  NAME gsl
  GITHUB_REPOSITORY ampl/gsl
  GIT_TAG        20211111
  OPTIONS "BUILDLIBS statistics" "NO_AMPL_BINDINGS on" "GSL_DISABLE_TESTS on" "DOCUMENTATION off"
)
find_package(gcem REQUIRED)

# Libs
add_subdirectory(optrode)
add_subdirectory(filters)
add_subdirectory(AudioFile)
add_subdirectory(cnl)

# Output Information
if(CMAKE_SYSTEM MATCHES Windows)
   message(STATUS "Target system is Windows")
endif()

if(CMAKE_HOST_SYSTEM MATCHES Linux)
   message(STATUS "Build host runs Linux")
endif()

add_executable(demo main.cpp)
target_link_libraries(demo PUBLIC ${Boost_LIBRARIES} optrode filters)

add_executable(lmsDemo lmsDemo.cpp)
target_link_libraries(lmsDemo PUBLIC ${Boost_LIBRARIES} gcem filters AudioFile Cnl gsl)
