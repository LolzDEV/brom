cmake_minimum_required(VERSION 3.10)

project(brom)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(llvm_libs support core irreader x86asmparser x86codegen x86desc x86disassembler x86info)

file(GLOB SOURCES src/*.cpp)

add_executable(brom ${SOURCES})

target_link_libraries(brom ${llvm_libs})
