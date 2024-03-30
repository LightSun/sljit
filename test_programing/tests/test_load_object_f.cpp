#include "h7/Instruction.h"
#include "h7/DataStack.h"
#include "h7/Classer.h"
#include "h7/h7_ctx_impl.h"

#include "test_common.h"

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
    ClassScope* getClassScope(){
        return m_cls->getScope();
    }
private:
    std::unique_ptr<Classer> m_cls;
};

void test_load_object_f(){
    printf(" test_load_object_f >> start...\n");
    ObjectTester otest;
    auto pObj = otest.createPerson1();

    ObjectDelegate od(pObj);
    void* ag1 = od.getFieldAddress("ag1");
    *(char*)ag1 = 0;
    //
    Function func(3);
    //p.ag1=v, ...p.ag5=v
    //print(ag1~ag5)
    auto reg_obj_arr = func.allocLocal3();
    {   //load obj
        auto sent_obj = Sentence::New();
        sent_obj->makeLoadObjectDS(0, reg_obj_arr);
        func.addEasyStatment(sent_obj);
    }
    {
        //load field
        auto sent = Sentence::New();
        int ls_f = func.allocLocal();
        sent->makeLoadObjectField(reg_obj_arr, kType_int8, ls_f, kType_int8, 0);
        func.addEasyStatment(sent);
        func.addEasyStatment(addStat_printLSInt(kType_int32, ls_f));
    }
    //
    auto codeDesc = CodeDesc::New();
    String msg = func.compile(codeDesc.get());
    H7_ASSERT(msg.empty());
    //
    DataStack ds(3);
    ds.setAt(0, pObj);
    ds.setAt(1, 10);
    ds.setAt(2, 100);

    msg = codeDesc->run(&ds);
    H7_ASSERT(msg.empty());
    //
    pObj->unref();
    printf(" test_load_object_f >> end...\n");
}
