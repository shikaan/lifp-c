cmake_minimum_required(VERSION 3.31)
project(flat_ast_interpreter C)

set(CMAKE_C_STANDARD 23)

# Strict warning and debug/release flags from flags.mk
set(COMMON_CFLAGS "-std=c23 -Wall -Wextra -Werror -fdiagnostics-color=always -fno-common -Winit-self -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wassign-enum -Wconversion -Wno-ignored-qualifiers -Wno-aggregate-return")

if(APPLE)
    set(COMMON_CFLAGS "${COMMON_CFLAGS} -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk")
endif()

set(DEBUG_CFLAGS "-g -O0 -fsanitize=address,undefined -DDEBUG")
set(RELEASE_CFLAGS "-O2 -DNDEBUG")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS "${COMMON_CFLAGS} ${RELEASE_CFLAGS}")
else()
    set(CMAKE_C_FLAGS "${COMMON_CFLAGS} ${DEBUG_CFLAGS}")
endif()

include_directories(src)
include_directories(tests)

add_executable(flat_ast_interpreter main.c src/lexer.c src/lexer.h src/debug.h src/result.h)
add_executable(lexer_test tests/lexer.test.c src/lexer.c src/lexer.h src/debug.h src/result.h tests/test.h)

# Enable testing and add test
include(CTest)
add_test(NAME lexer_test COMMAND lexer_test)
