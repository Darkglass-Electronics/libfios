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

if(MSVC)
  include(FetchContent)
  FetchContent_Declare(pthreads4w
    GIT_REPOSITORY https://git.code.sf.net/p/pthreads4w/code
    GIT_TAG f12b445b336ee0117b43fca1d4b9f22c9af82c36
  )
  FetchContent_MakeAvailable(pthreads4w)

  add_library(pthreads4w STATIC)
  add_library(fios::pthreads4w ALIAS pthreads4w)

  target_sources(pthreads4w PRIVATE ${pthreads4w_SOURCE_DIR}/pthread.c)
  target_compile_definitions(pthreads4w
    PRIVATE
      HAVE_CONFIG_H
      HAVE_STDINT_H=1
      _POSIX_C_SOURCE=200112L
    PUBLIC
      INCLUDE_NP
      PTW32_DLLPORT
      __PTW32_STATIC_LIB
  )

  target_include_directories(pthreads4w
    PUBLIC
      ${pthreads4w_SOURCE_DIR}
  )
  set(FIOS_PTHREADS fios::pthreads4w)
else()
  set(FIOS_PTHREADS ${CMAKE_THREAD_LIBS_INIT})
endif()

# building interface library
add_library(libfios-interface INTERFACE)
add_library(libfios::interface ALIAS libfios-interface)

target_include_directories(libfios-interface
  INTERFACE
    src
)

target_link_libraries(libfios-interface
  INTERFACE
    ${FIOS_PTHREADS}
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
      USC_SHARED_LIBRARY
  )

  target_include_directories(libfios
    PRIVATE
      src
  )

  target_link_libraries(libfios
    PRIVATE
      ${FIOS_PTHREADS}
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
