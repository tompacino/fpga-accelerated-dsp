add_library(filters lms/lms.h leakyIntegrator.h)
set_target_properties(filters PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(filters PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lms)
