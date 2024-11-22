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
add_library(libusc-interface INTERFACE)
add_library(libusc::interface ALIAS libusc-interface)

target_include_directories(libusc-interface
  INTERFACE
    src
)

target_sources(libusc-interface
  INTERFACE
    src/libusc.c
)

# building standalone tools (if we are building this project alone)
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)

  # building shared library
  add_library(libusc SHARED)

  target_compile_definitions(libusc
    PRIVATE
      USC_SHARED_LIBRARY
  )

  target_include_directories(libusc
    PRIVATE
      src
  )

  target_sources(libusc
    PRIVATE
      src/libusc.c
  )

  # building utility executable
  add_executable(usc-file)

  target_include_directories(usc-file
    PRIVATE
      src
  )

  target_link_libraries(usc-file
    PRIVATE
      libusc-interface
  )

  target_sources(usc-file
    PRIVATE
      src/usc-file.c
  )

endif()

#######################################################################################################################
