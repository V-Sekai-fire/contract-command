# The interactor contract.
#
# `weft::interactor` links nothing, and that is the test. If a socket library or a QUIC
# implementation ever appears below, the boundary this repository exists to hold has gone, and
# the linker is a better place to find that out than a review is.
cmake_minimum_required(VERSION 3.22)
project(weft_interactor C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(weft_interactor STATIC src/cbor.c)
add_library(weft::interactor ALIAS weft_interactor)
target_include_directories(weft_interactor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(weft_interactor PRIVATE -Wall -Wextra)
