#include "h7/Instruction.h"
#include "h7/DataStack.h"
#include "h7/Classer.h"
#include "h7/h7_ctx_impl.h"

using namespace h7;

class ObjectTester{
public:
    ObjectTester(){
        m_cls = std::make_unique<Classer>(ClassScope::enterGlobal());
    }
    ObjectPtr createPerson1(){
        auto lis = TypeInfo::makeListSimple(kType_int8, kType_int16, kType_int32,
                                 kType_float, kType_double);
        ListString names = {"ag1", "ag2", "ag3", "ag4", "ag5"};
        auto pCls = m_cls->defineClass("Person", lis, names);
        return m_cls->create(pCls, nullptr);
    }
private:
    std::unique_ptr<Classer> m_cls;
};

void test_load_object_f(){
    ObjectTester otest;
    auto pObj = otest.createPerson1();
    //
    printf(" test_call >> start...\n");
    Function func(3);
    //
    pObj->unref();
}
