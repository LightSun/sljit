

add_executable(stack_base
    tests/test_stack_based.cpp
    )

add_executable(reg_base
    tests/test_register_based.cpp
    )
##
file(GLOB_RECURSE SRC_H7 h7/*.h h7/*.c h7/*.cpp)

add_library(h7
    ${SRC_H7}
    )

# h7 test

add_executable(h7_test_all
    tests/main.cpp
    tests/test_align_struct.cpp
    tests/test_inst.cpp
    tests/test_load_object_f.cpp
    )

target_link_libraries(h7_test_all PUBLIC
    h7 sljit_core
    )
