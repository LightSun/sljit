
#include <stdio.h>

extern void test_align_struct();

int main(int argc, const char* argv[]){

    setbuf(stdout, NULL);
    test_align_struct();
    return 0;
}
