cmake_minimum_required(VERSION 3.15)
project(libusc)

set(CMAKE_POLICY_DEFAULT_CMP0025 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

# building interface library
add_library(libusc INTERFACE)

target_include_directories(libusc
  INTERFACE
    src
)

target_sources(libusc
  INTERFACE
    src/libusc.c
)

# building standalone tools (if we are building this project alone)
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)

  add_executable(usc-file)

  target_include_directories(usc-file
    PRIVATE
      src
  )

  target_link_libraries(usc-file
    PRIVATE
      libusc
  )

  target_sources(usc-file
    PRIVATE
      src/libusc.c
      src/usc-file.c
  )

else()

endif()

#######################################################################################################################
