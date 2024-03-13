
#include <stdio.h>

extern void test_align_struct();
extern void test_inst();

int main(int argc, const char* argv[]){

    setbuf(stdout, NULL);
    test_align_struct();
    test_inst();
    return 0;
}
