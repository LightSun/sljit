
#include <stdlib.h>
#include <stdio.h>

struct Person{
    char name[16];
    int age;
};

struct Student{
    struct Person p;
    int arr;
    int* b;
};

void g_test1(){
    struct Student* st = malloc(sizeof (struct Student));
    st->p.age = 10086;
    struct Person* sp = (struct Person*)st;
    printf("g_test1 >> age = %d\n", sp->age);
}
