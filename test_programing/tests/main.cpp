
#include <stdio.h>

extern void test_align_struct();
extern void test_inst();
extern void test_load_object_f();

int main(int argc, const char* argv[]){

    setbuf(stdout, NULL);
    test_align_struct();
    test_inst();
    test_load_object_f();
    return 0;
}
