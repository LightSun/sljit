
#include "h7/Classer.h"
#include "h7/h7_ctx_impl.h"
#include "h7/common/c_common.h"
#include "h7/common/common.h"

using namespace h7;

static void test_tail_align();
static void test_head_align();
static void test_align_no_head_tail();
static void test_1();
static void test_aligned_1_middle();
static void test_aligned_1_head();
static void test_aligned_1_tail();

void test_align_struct(){

    test_tail_align();
    test_head_align();
    test_align_no_head_tail();
    test_1();
    test_aligned_1_middle();
    test_aligned_1_head();
    test_aligned_1_tail();
}

void test_1(){
    auto type2 = TypeInfo::makeSimple(kType_uint16);

    ListTypeInfo ls = {type2};
    List<String> fns = {"a1"};

    ClassInfo info;
    alignStructSize(ls, fns, &info);
    H7_ASSERT(info.structSize == 8);
    H7_ASSERT(info.getFieldOffset("a1")== 0);
}

void test_tail_align(){
    auto type1 = TypeInfo::makeSimple(kType_uint8);
    auto type2 = TypeInfo::makeSimple(kType_uint16);
    auto type3 = TypeInfo::makeSimple(kType_uint32);

    auto type4 = TypeInfo::makeSimple(kType_uint64);

    auto type5 = TypeInfo::makeSimple(kType_float);

    auto type6 = TypeInfo::makeSimple(kType_double);
    ListTypeInfo ls = {type1, type2, type3, type4, type5, type6};
    List<String> fns = {"a1", "a2", "a3", "a4", "a5", "a6"};

    ClassInfo info;
    alignStructSize(ls, fns, &info);
    H7_ASSERT(info.structSize == 32);
    H7_ASSERT(info.getFieldOffset("a1") == 0);
    H7_ASSERT(info.getFieldOffset("a2") == 1);
    H7_ASSERT(info.getFieldOffset("a3") == 3);
    H7_ASSERT(info.getFieldOffset("a4") == 8);
    H7_ASSERT(info.getFieldOffset("a5") == 16);
    H7_ASSERT(info.getFieldOffset("a6") == 24);
}

void test_head_align(){
    auto type1 = TypeInfo::makeSimple(kType_double);

    auto type2 = TypeInfo::makeSimple(kType_uint8);

    auto type3 = TypeInfo::makeSimple(kType_uint64);

    auto type4 = TypeInfo::makeSimple(kType_uint16);
    auto type5 = TypeInfo::makeSimple(kType_uint32);

    auto type6 = TypeInfo::makeSimple(kType_float);

    ListTypeInfo ls = {type1, type2, type3, type4, type5, type6};
    List<String> fns = {"a1", "a2", "a3", "a4", "a5", "a6"};

    ClassInfo info;
    alignStructSize(ls, fns, &info);
    H7_ASSERT(info.structSize == 40);
    H7_ASSERT(info.getFieldOffset("a1") == 0);
    H7_ASSERT(info.getFieldOffset("a2") == 8);
    H7_ASSERT(info.getFieldOffset("a3") == 16);
    H7_ASSERT(info.getFieldOffset("a4") == 24);
    H7_ASSERT(info.getFieldOffset("a5") == 26);
    H7_ASSERT(info.getFieldOffset("a6") == 32);
}

void test_align_no_head_tail(){

    auto type1 = TypeInfo::makeSimple(kType_uint8);

    auto type2 = TypeInfo::makeSimple(kType_double);

    auto type3 = TypeInfo::makeSimple(kType_uint32);

    auto type4 = TypeInfo::makeSimple(kType_uint64);

    auto type5 = TypeInfo::makeSimple(kType_uint16);
    auto type6 = TypeInfo::makeSimple(kType_float);

    ListTypeInfo ls = {type1, type2, type3, type4, type5, type6};
    List<String> fns = {"a1", "a2", "a3", "a4", "a5", "a6"};

    ClassInfo info;
    alignStructSize(ls, fns, &info);
    H7_ASSERT(info.structSize == 40);
    H7_ASSERT(info.getFieldOffset("a1") == 0);
    H7_ASSERT(info.getFieldOffset("a2") == 8);
    H7_ASSERT(info.getFieldOffset("a3") == 16);
    H7_ASSERT(info.getFieldOffset("a4") == 24);
    H7_ASSERT(info.getFieldOffset("a5") == 32);
    H7_ASSERT(info.getFieldOffset("a6") == 34);
}

void test_aligned_1_middle(){

    auto type1 = TypeInfo::makeSimple(kType_uint8);

    auto type2 = TypeInfo::makeSimple(kType_double);

    auto type3 = TypeInfo::makeSimple(kType_uint32);


    ListTypeInfo ls = {type1, type2, type3};
    List<String> fns = {"a1", "a2", "a3"};

    ClassInfo info;
    alignStructSize(ls, fns, &info);
    H7_ASSERT(info.structSize == 24);
    H7_ASSERT(info.getFieldOffset("a1") == 0);
    H7_ASSERT(info.getFieldOffset("a2") == 8);
    H7_ASSERT(info.getFieldOffset("a3") == 16);
}

void test_aligned_1_head(){

    auto type1 = TypeInfo::makeSimple(kType_double);

    auto type2 = TypeInfo::makeSimple(kType_uint8);
    auto type3 = TypeInfo::makeSimple(kType_uint32);


    ListTypeInfo ls = {type1, type2, type3};
    List<String> fns = {"a1", "a2", "a3"};

    ClassInfo info;
    alignStructSize(ls, fns, &info);
    H7_ASSERT(info.structSize == 16);
    H7_ASSERT(info.getFieldOffset("a1") == 0);
    H7_ASSERT(info.getFieldOffset("a2") == 8);
    H7_ASSERT(info.getFieldOffset("a3") == 9);
}

void test_aligned_1_tail(){

    auto type1 = TypeInfo::makeSimple(kType_uint8);
    auto type2 = TypeInfo::makeSimple(kType_uint32);
    auto type3 = TypeInfo::makeSimple(kType_double);

    ListTypeInfo ls = {type1, type2, type3};
    List<String> fns = {"a1", "a2", "a3"};

    ClassInfo info;
    alignStructSize(ls, fns, &info);
    H7_ASSERT(info.structSize == 16);
    H7_ASSERT(info.getFieldOffset("a1") == 0);
    H7_ASSERT(info.getFieldOffset("a2") == 1);
    H7_ASSERT(info.getFieldOffset("a3") == 8);
}
