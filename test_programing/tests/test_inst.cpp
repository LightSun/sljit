
#include "h7/Instruction.h"
#include "h7/DataStack.h"

using namespace h7;

void test_inst(){
    Function func(3);
    auto sent = Sentence::New();
    sent->makeAllValid();
    sent->left.type = kType_int32;
    sent->right.type = kType_int32;
    sent->ip.type = kType_int32;
    sent->op = OpCode::ADD;
    //
    sent->ip.makeDataStack();
    sent->left.makeDataStack();
    sent->right.makeDataStack();
    //
    sent->ip.rw = 0;
    sent->left.rw = 1;
    sent->right.rw = 2;
    func.addEasyStatment(sent);
    //
    auto codeDesc = CodeDesc::New();
    String msg = func.compile(codeDesc.get());
    H7_ASSERT(msg.empty());
    //
    DataStack ds(3);
    ds.setAt(0, 0);
    ds.setAt(1, 10);
    ds.setAt(2, 100);
    msg = codeDesc->run(&ds);
    H7_ASSERT(msg.empty());
    int val = ds.getAt<int>(0);
    printf("add result: %d\n", val);
}
