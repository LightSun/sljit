
#include "h7/Instruction.h"
#include "h7/DataStack.h"

using namespace h7;

void test_inst(){
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
