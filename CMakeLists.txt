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

# Libs
add_subdirectory(optrode)
add_subdirectory(filters)
add_subdirectory(AudioFile)
add_subdirectory(cnl)

add_executable(demo main.cpp)
target_link_libraries(demo PUBLIC ${Boost_LIBRARIES} optrode filters AudioFile Cnl)

add_executable(lmsDemo lmsDemo.cpp)
target_link_libraries(lmsDemo PUBLIC ${Boost_LIBRARIES} fixed_math filters AudioFile Cnl)
