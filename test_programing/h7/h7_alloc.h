#pragma once

#include <stdlib.h>

#define H7_NEW(s) malloc(s)
#define H7_NEW_TYPE(T) (T*)malloc(sizeof(T))
#define H7_DELETE(ptr) free(ptr)
