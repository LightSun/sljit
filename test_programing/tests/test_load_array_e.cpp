#include "h7/Instruction.h"
#include "h7/DataStack.h"
#include "h7/Classer.h"
#include "h7/h7_ctx_impl.h"

#include "test_common.h"

using namespace h7;

static void test_array_e();

//load array element
void test_load_array_e(){
    test_array_e();
}

void test_array_e(){
    String arr = "0123456789";
    void* dataPtr = (void*)arr.data();

    Function func(3);
    //p.ag1=v, ...p.ag5=v
    //print(ag1~ag5)
    auto reg_obj_arr = func.allocLocalN(3);
    {   //load obj
        auto sent_obj = Sentence::New();
        sent_obj->makeLoadObjectDS(0, reg_obj_arr);
        func.addEasyStatment(sent_obj);
    }

    //
    auto codeDesc = CodeDesc::New();
    String msg = func.compile(codeDesc.get());
    H7_ASSERT(msg.empty());
    //
    DataStack ds(3);
    ds.setAt(0, dataPtr);
    ds.setAt(1, 10);
    ds.setAt(2, 100);

    msg = codeDesc->run(&ds);
    H7_ASSERT(msg.empty());
}
