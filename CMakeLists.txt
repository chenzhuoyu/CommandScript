cmake_minimum_required(VERSION 3.6)
project(CommandScript)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

include(ExternalProject)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckIncludeFileCXX)

add_subdirectory(thirdparty/fmt)

ExternalProject_Add(fmtlib
        URL thirdparty/fmt
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/fmtlib
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>)

ExternalProject_Get_Property(fmtlib INSTALL_DIR)
link_directories(${INSTALL_DIR}/lib)
include_directories(${INSTALL_DIR}/include)

include_directories(
        include/compiler
        include/runtime/exception
        include/runtime
        include/utils)

set(COMMAND_SCRIPT
        include/compiler/AST.h
        include/compiler/Parser.h
        include/compiler/Tokenizer.h
        include/runtime/exception/SyntaxError.h
        include/utils/NonCopyable.h
        include/utils/NonMovable.h
        include/utils/Strings.h
        src/compiler/AST.cpp
        src/compiler/Parser.cpp
        src/compiler/Tokenizer.cpp
        src/utils/Strings.cpp)

add_executable(CommandScript ${COMMAND_SCRIPT} src/main.cpp)
add_dependencies(CommandScript fmtlib)
target_link_libraries(CommandScript libfmt.a)
