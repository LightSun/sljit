#pragma once


#include <stdio.h>
#include <stdlib.h>

#include <functional>
#include <iostream>

#ifndef H7_ASSERT
#define H7_ASSERT(condition)                                                   \
    do                                                                      \
    {                                                                       \
        if (!(condition))                                                   \
        {                                                                   \
            std::cout << "Assertion failure: " << __FILE__ << "::" << __FUNCTION__  \
                                     << __LINE__ \
                                     << " >> " << #condition << std::endl;  \
            __THROW_ERR("");                                                     \
        }                                                                   \
    } while (0)
#endif

#ifndef H7_ASSERT_EQ
#define H7_ASSERT_EQ(a, b)                                                   \
    do                                                                      \
    {                                                                       \
        if (a != b)                                                   \
        {                                                                   \
            std::cout << "Assertion failure: " << __FILE__ << "::" << __FUNCTION__  \
                                     << __LINE__ \
                                     << " >> " << a << "==" << b << std::endl;  \
            __THROW_ERR("");                                                     \
        }                                                                   \
    } while (0)
#endif

#ifndef H7_ASSERT_IF
#define H7_ASSERT_IF(pre, condition)                                       \
    do                                                                      \
    {                                                                       \
        if (pre && !(condition))                                            \
        {                                                                   \
            std::cout << "Assertion failure: " << __FILE__ << "::" << __FUNCTION__  \
                                     << __LINE__ \
                                     << " >> " << #condition << std::endl;  \
            __THROW_ERR("");                                                             \
        }                                                                   \
    } while (0)
#endif

#ifndef H7_ASSERT_X
#define H7_ASSERT_X(condition, msg)                                                   \
    do                                                                      \
    {                                                                       \
        if (!(condition))                                                   \
        {                                                                   \
            std::cout << "Assertion failure: " << __FILE__ << "::" << __FUNCTION__  \
                                     << __LINE__ \
                                     << "\n" << (msg) << " >> " << #condition << std::endl;  \
            __THROW_ERR("");                                                               \
        }                                                                   \
    } while (0)
#endif

static inline void __THROW_ERR(const std::string& s){
    abort();
}
