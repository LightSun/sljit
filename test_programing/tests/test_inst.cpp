
#include "h7/Instruction.h"
#include "h7/DataStack.h"

using namespace h7;

static void test_add();
static void test_call();

static float FUNC_1(int a, int b){
    return a * 1.0f / b;
}
static void printFloat(float val){
    printf("printFVal >> v = %g\n", val);
}
static void printInt(int val){
    printf("printInt >> val = %d\n", val);
}
static Sentence::Type addStat_printDSInt(int type, int idx);
static Sentence::Type addStat_printLSInt(int type, int idx);

void test_inst(){
    test_call();
    test_add();
}
//------------------------------
void test_call(){
    printf(" test_call >> start...\n");
    Function func(3);
    // a, b, c
    // int v1 = b + c
    // float v2 = func(a, v1)
    // print(v2)
    {
        int ls_v1 = func.allocLocal();
        //
        {
            auto sent = Sentence::New();
            sent->makeSimple1LS2DS({kType_int32, kType_int32, kType_int32},
                {(ULong)ls_v1,1,2});
            sent->op = OpCode::ADD;
            func.addEasyStatment(sent);
        }
        func.addEasyStatment(addStat_printLSInt(kType_int32, ls_v1));
        int ls_v2 = func.allocLocal();
        {
            auto sent = Sentence::NewCall((void*)FUNC_1);
            sent->addFunctionParameter(ParameterInfo::make(
                        kType_int32, kPD_FLAG_DS, 0));
            sent->addFunctionParameter(ParameterInfo::make(
                        kType_int32, kPD_FLAG_LS, ls_v1));
            sent->ip.extra->funcRet.type = kType_float;
            sent->ip.extra->funcRet.index = ls_v2;
            sent->ip.extra->funcRet.flags = kPD_FLAG_RETURN | kPD_FLAG_LS;
            func.addEasyStatment(sent);
        }
        {
            auto sent = Sentence::NewCall((void*)printFloat);
            sent->ip.extra->funcParams[0] = ParameterInfo::make(
                        kType_float, kPD_FLAG_LS, ls_v2);
            //no return.
            func.addEasyStatment(sent);
        }
    }
    auto codeDesc = CodeDesc::New();
    String msg = func.compile(codeDesc.get());
    H7_ASSERT(msg.empty());
    //
    DataStack ds(3);
    ds.setAt(0, 12);
    ds.setAt(1, 20);
    ds.setAt(2, 100);

    msg = codeDesc->run(&ds);
    H7_ASSERT(msg.empty());
    //not change ds
    int val = ds.getAt<int>(0);
    int val2 = ds.getAt<int>(1);
    int val3 = ds.getAt<int>(2);
    printf("v1, v2, v3 = (%d, %d, %d)\n", val, val2, val3);
    H7_ASSERT_X(val == 12, std::to_string(val));

    printf(" test_call >> end...\n");
}

void test_add(){
    Function func(3);
    auto sent = Sentence::New();
    //0 is ret. 1 and 2 is left/right.
    sent->makeSimple3DS(kType_int32, {0,1,2});
    sent->op = OpCode::ADD;
    func.addEasyStatment(sent);
    //
    auto codeDesc = CodeDesc::New();
    String msg = func.compile(codeDesc.get());
    H7_ASSERT(msg.empty());
    //
    DataStack ds(3);
    ds.setAt(0, 1);
    ds.setAt(1, 10);
    ds.setAt(2, 100);
    msg = codeDesc->run(&ds);
    H7_ASSERT(msg.empty());
    int val = ds.getAt<int>(0);
    H7_ASSERT(val == 110);
}
Sentence::Type addStat_printDSInt(int type, int idx){
    auto sent = Sentence::NewCall((void*)printInt);
    sent->addFunctionParameter(ParameterInfo::make(
                                   type, kPD_FLAG_DS, idx));
    return sent;
}
Sentence::Type addStat_printLSInt(int type, int idx){
    auto sent = Sentence::NewCall((void*)printInt);
    sent->addFunctionParameter(ParameterInfo::make(
                type, kPD_FLAG_LS, idx));
    return sent;
}
