# Software Submodule for Optrode FPGA+SoC Platform

This platform is a work in progress and is in an abandoned development state.

Key completed work is included in `lms.h` which is a templated header only implementation of an LMS filter.

- A test program `lmsDemo.cpp` runs an LMS filter on an input wave file.
- Tests of a DMA loopback recording functionality exist but are not in a working state due to external dependencies not included in this repository
- Third party DMA loopback test files such as `dmatest/{dmatest|test}.c` are included for testing AXI DMA interactions, and belong to their respective coyright holders

### Building the submodule

- To configure a build for the first time from one of the preset build options, use `cmake --preset [BUILD_PRESET]`, e.g. `cmake --preset dev_debug`
- To build binaries for a specific preset run `cmake --build -- preset [BUILD_PRESET]`

### Prerequisites
Prerequisites are detailed in `CMakeLists.txt`

- Boost (used for logging, utilities)
- CMake Package Manager (used for package management)
- Computational Numeric Library (used for non standard data formats like fixed point integer computation)
- AudioFile.h header only `.wav` file library
- GSL static checking
