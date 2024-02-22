

add_executable(stack_base
    test_stack_based.cpp
    )

add_executable(reg_base
    test_register_based.cpp
    )
##
file(GLOB_RECURSE SRC_H7 h7/*.h h7/*.c h7/*.cpp)

add_library(h7
    ${SRC_H7}
    )
