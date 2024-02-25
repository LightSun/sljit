#pragma once

#include <stdlib.h>

//later align memory
#define H7_NEW(s) malloc(s)
#define H7_DELETE(ptr) free(ptr)
#define H7_NEW_TYPE(T) (T*)malloc(sizeof(T))

#define H7_NEW_OBJ(T) new T()
#define H7_NEW_OBJ1(T, p) new T(p)
#define H7_DELETE_OBJ(p) delete p
