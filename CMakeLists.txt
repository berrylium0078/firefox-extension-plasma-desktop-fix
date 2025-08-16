# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.20)
project(firefox-extension-plasma-desktop-fix)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)

execute_process(
  COMMAND git describe --tags --always --dirty
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE PROJECT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Project version: ${PROJECT_VERSION}")

include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(src/kwin)
add_subdirectory(src/native)
add_subdirectory(src/browser)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
