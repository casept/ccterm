# Earlier versions are not tested or supported
cmake_minimum_required(VERSION 3.19)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Fix stuff that's broken by default
include(ForbidInSource)
include(SetBuildType)

project(
  ccterm
  VERSION 0.1
  DESCRIPTION "casept's crappy terminal emulator"
  LANGUAGES CXX)
add_executable(ccterm "src/main.cpp" "src/render.cpp" "src/pty.cpp")
# We want ISO C++20
target_compile_features(ccterm PRIVATE cxx_std_20)
target_precompile_headers(
  ccterm
  PRIVATE
  # STL
  <chrono>
  <optional>
  <thread>
  <stdexcept>
  <string>
  <string_view>
  # 3rd-party
  <fmt/core.h>
  <fmt/format.h>
  <fmt/ranges.h>
  <SDL_ttf.h>
  <SDL_render.h>)

# 3rd-party deps
find_package(fmt 7.1.3 REQUIRED QUIET)
find_package(SDL2 2.0.14 REQUIRED QUIET)
find_package(SDL2_ttf 2.0.14 REQUIRED QUIET)

if(CMAKE_BUILD_TYPE EQUAL "Debug")
  list(APPEND CCTERM_SHARED_COMPILE_OPTS -O0 -g)
endif()

option(CCTERM_ENABLE_SANITIZERS "Enable address and UB sanitizers." FALSE)
if(CCTERM_ENABLE_SANITIZERS)
  list(APPEND CCTERM_SHARED_COMPILE_OPTS -fsanitize=address
       -fsanitize=undefined -fno-omit-frame-pointer)
  list(APPEND CCTERM_DEBUG_LD_OPTS -fno-omit-frame-pointer -fsanitize=address
       -fsanitize=undefined)
endif()

option(CCTERM_FORCE_COLORED_OUTPUT
       "Always produce ANSI-colored output (GNU/Clang only)." FALSE)
if(${ccterm_FORCE_COLORED_OUTPUT})
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    list(APPEND SHARED_COMPILE_OPTS -fdiagnostics-color=always)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(APPEND SHARED_COMPILE_OPTS -fcolor-diagnostics)
  endif()
endif()

option(CCTERM_ENABLE_WARNINGS
       "Enable strict compiler warnings (GNU/Clang only)." FALSE)
if(${ccterm_ENABLE_WARNINGS})
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    list(APPEND SHARED_COMPILE_OPTS -Wall -Wextra -Wpedantic)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(
      APPEND
      CCTERM_SHARED_COMPILE_OPTS
      -Wall
      -Wextra
      -Wpedantic
      -Weverything
      -Wno-c++98-compat-pedantic
      -Wno-c++98-compat
      -Wno-padded
      -Wno-covered-switch-default
      -Wno-newline-eof
      -Wno-missing-prototypes
      -Wno-weak-vtables
      -Wno-exit-time-destructors
      -Wno-unreachable-code-break)
  endif()
endif()

target_compile_options(ccterm PRIVATE ${CCTERM_SHARED_COMPILE_OPTS})
target_link_libraries(ccterm PRIVATE fmt::fmt SDL2::SDL2 SDL2::SDL2_ttf)
target_link_options(ccterm PRIVATE ${CCTERM_DEBUG_LD_OPTS})

# Hack to force cmake to add system (libstdc++) header path to
# compile_commands.json. This also adds a lot of junk, but as long as it doesn't
# slow clangd too much I'll deal.
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

install(
  TARGETS ccterm
  CONFIGURATIONS Release
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
