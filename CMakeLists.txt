cmake_minimum_required(VERSION 3.5)

project(sljit LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#-Wall -Wextra -Wconversion -Wsign-compare -Werror
set(TEST_INTERNAL_FLAGS "-Wall -Wextra -Wconversion -Wsign-compare")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC ${TEST_INTERNAL_FLAGS}")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC -std=c11 ${TEST_INTERNAL_FLAGS}")

include_directories(sljit_src/)
include_directories(test_src/)

add_compile_definitions(SLJIT_HAVE_CONFIG_PRE=1)
add_compile_definitions(USE_MALLOC_WRAPPERS)
add_compile_definitions(USE_C11_ATOMICS)

#file(GLOB_RECURSE SRC_CORE sljit_src/*.h sljit_src/*.c)
SET(SRC_CORE
    sljit_src/sljitConfig.h
    sljit_src/sljitConfigInternal.h
    sljit_src/sljitLir.h
    sljit_src/sljitLir.c
    test_src/sljitConfigPre.h
    test_src/sljitConfigPost.h
    )

add_library(sljit_core ${SRC_CORE})
target_link_libraries(sljit_core m pthread)

## jit-middle
file(GLOB_RECURSE SRC_VM jit-middle/*.h
    jit-middle/*.c
    jit-middle/*.cpp
    )

include_directories(jit-middle/)

## subs
add_subdirectory(test_programing/)

## test
SET(SRC_TEST
    #${SRC_VM}
    test_src/sljitTest.c
    test_src/sljitMain.cpp
    #test_src/test_jit_middle.c
    test_src/test_base1.c
    test_src/test_branch.c
    test_src/test_loop.c
    test_src/test_struct.c
    test_src/test_struct2.cpp
    test_src/test1.c
    test_src/test_sljit_st.c
    test_src/test_atomic.c
    test_src/test_shift.cpp
    test_src/test_h.h
    )
add_executable(sljit_test ${SRC_TEST})
target_link_libraries(sljit_test sljit_core)


