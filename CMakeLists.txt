cmake_minimum_required(VERSION 3.20)

project(Chaitin LANGUAGES C CXX)

find_package(LLVM REQUIRED CONFIG)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

add_library(chaitin MODULE
    RegAllocChaitin.cpp
    RegAllocBase.h
    AllocationOrder.h AllocationOrder.cpp
    RegAllocChaitinRegisters.h RegAllocChaitinRegisters.cpp
    RegAllocChaitinGraph.h RegAllocChaitinGraph.cpp
    RegAllocChaitinSolvers.h RegAllocChaitinSolvers.cpp
)

target_compile_features(chaitin PRIVATE cxx_std_17)
target_compile_options(chaitin PRIVATE -Wall -Wextra -pedantic -fno-rtti)
target_compile_definitions(chaitin PUBLIC ${LLVM_DEFINITIONS_LIST})
target_include_directories(chaitin PUBLIC ${LLVM_INCLUDE_DIRS})

include(GNUInstallDirs)
install(TARGETS chaitin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
