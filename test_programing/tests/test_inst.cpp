
#include "h7/Instruction.h"
#include "h7/DataStack.h"

using namespace h7;

static void test_add();
static void test_call();

void test_inst(){
    test_add();
}
//------------------------------
void test_call(){
    Function func(3);
    {
    auto sent = Sentence::New();
    //0 is ret. 1 and 2 is left/right.
    sent->makeDSSimple3(kType_int32, {0,1,2});
    sent->op = OpCode::ADD;
    func.addEasyStatment(sent);
    }
    //create function call of c-func
    {
        int ls_p1 = func.allocLocal();
        int ls_ret = func.allocLocal();
        auto sent = Sentence::New();
        sent->setValidFlags(1);
        sent->ip.extra = std::make_unique<OpExtraInfo>();
        sent->ip.extra->funcRet.type = kType_int32;
        sent->ip.extra->funcRet.index = ls_ret;
        sent->ip.extra->funcRet.flags = kPD_FLAG_RETURN | kPD_FLAG_LS;
    }
}

void test_add(){
    Function func(3);
    auto sent = Sentence::New();
    //0 is ret. 1 and 2 is left/right.
    sent->makeDSSimple3(kType_int32, {0,1,2});
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
