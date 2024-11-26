cmake_minimum_required(VERSION 3.15)
project(libfios)

set(CMAKE_POLICY_DEFAULT_CMP0025 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(Threads REQUIRED)

# building interface library
add_library(libfios-interface INTERFACE)
add_library(libfios::interface ALIAS libfios-interface)

target_include_directories(libfios-interface
  INTERFACE
    src
)

target_link_libraries(libfios-interface
  INTERFACE
    ${CMAKE_THREAD_LIBS_INIT}
)

target_sources(libfios-interface
  INTERFACE
    src/libfios-file.c
    src/libfios-serial.c
)

# building standalone tools (if we are building this project alone)
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)

  # building shared library
  add_library(libfios SHARED)

  target_compile_definitions(libfios
    PRIVATE
      FIOS_SHARED_LIBRARY
  )

  target_include_directories(libfios
    PRIVATE
      src
  )

  target_link_libraries(libfios
    PRIVATE
      ${CMAKE_THREAD_LIBS_INIT}
  )

  target_sources(libfios
    PRIVATE
      src/libfios-file.c
      src/libfios-serial.c
  )

  # building utility executable
  add_executable(fios-file)

  target_include_directories(fios-file
    PRIVATE
      src
  )

  target_link_libraries(fios-file
    PRIVATE
      libfios-interface
  )

  target_sources(fios-file
    PRIVATE
      src/fios-file.c
  )

endif()

#######################################################################################################################
