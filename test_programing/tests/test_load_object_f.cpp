#include "h7/Instruction.h"
#include "h7/DataStack.h"
#include "h7/Classer.h"
#include "h7/h7_ctx_impl.h"

#include "test_common.h"

using namespace h7;

static void test_for_h7(ObjectPtr op);
static void test_for_direct_sljit(ObjectPtr op);
static void test_align();

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

    test_for_direct_sljit(pObj);
    test_for_h7(pObj);
    //
    pObj->unref();
    printf(" test_load_object_f >> end...\n");
}

void test_align(){
    int a = 47;
    auto old = a;
    a = calc_align_up_bit(a, 8);
    printf(" test_align >> calc_align_up_num-8(%d) = %d...\n", old, a);
}

#define LS_R SLJIT_MEM1(SLJIT_SP)
#define LS_RW(i) (sizeof(void*) * i)
void test_for_direct_sljit(ObjectPtr pObj){
     printf(" test_for_direct_sljit >> start...\n");
    ObjectDelegate od(pObj);
    void* ag1 = od.getFieldAddress("ag1");
    *(char*)ag1 = 123;
    //
    char* addr = (char*)od.getDataAddress();
    auto offset = od.getFieldOffset("ag1");
    H7_ASSERT(offset == 0);
    H7_ASSERT(od.getFieldOffset("ag2") == sizeof(char));
    //
    SljitFunc func;
    func.ds.setAt(0, pObj);
    func.enter();
    //---------
    sljit_emit_op1(func.C, SLJIT_MOV, SLJIT_R1, 0, SLJIT_IMM, (ULong)&addr[0]);
    sljit_emit_op1(func.C, SLJIT_MOV, SLJIT_R2, 0, SLJIT_IMM, offset);

    sljit_emit_mem(func.C, SLJIT_MOV_S8 | SLJIT_MEM_UNALIGNED, SLJIT_R0,
                   SLJIT_MEM2(SLJIT_R1, SLJIT_R2), 0); //byte shift

    sljit_emit_op1(func.C, SLJIT_MOV_S8, LS_R, LS_RW(0), SLJIT_R0, 0);
    sljit_emit_op1(func.C, SLJIT_MOV_S8, LS_R, LS_RW(1), LS_R, LS_RW(0));
    sljit_emit_op1(func.C, SLJIT_MOV_S8, SLJIT_R0, 0, LS_R, LS_RW(1));

    sljit_emit_icall(func.C, SLJIT_CALL, SLJIT_ARGS1(VOID, 32),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(printInt));
    //---------
    func.exit();
    func.run();
}

void test_for_h7(ObjectPtr pObj){
    printf(" test_for_h7 >> start...\n");
    ObjectDelegate od(pObj);
    void* ag1 = od.getFieldAddress("ag1");
    *(char*)ag1 = 123;
    //
    Function func(3);
    //p.ag1=v, ...p.ag5=v
    //print(ag1~ag5)
    auto reg_obj_arr = func.allocLocalN(3);
    {   //load obj
        auto sent_obj = Sentence::New();
        sent_obj->makeLoadObjectDS(0, reg_obj_arr);
        func.addEasyStatment(sent_obj);
    }
    {
        int ls_f = func.allocLocal();
        int ls_f2 = func.allocLocal();
        //load field
        auto sent = Sentence::New();
        sent->makeLoadObjectField(reg_obj_arr, kType_int8, ls_f, 0);
        func.addEasyStatment(sent);
        //cast
        sent = Sentence::New();
        sent->makeTypeCast2LS(kType_int8, ls_f, kType_int32, ls_f2);
        func.addEasyStatment(sent);
        //print
        func.addEasyStatment(addStat_printLSInt(kType_int32, ls_f2));
        //assign
        sent = Sentence::New();
        sent->makeAssignByIMM(kType_int32, ls_f2, kType_int32, "99");
        func.addEasyStatment(sent);
        //cast: ls_f2->ls_f
        sent = Sentence::New();
        sent->makeTypeCast2LS(kType_int32, ls_f2, kType_int8, ls_f);
        func.addEasyStatment(sent);
        //store
        sent = Sentence::New();
        sent->makeStoreObjectField(reg_obj_arr, kType_int8, ls_f, 0);
        func.addEasyStatment(sent);
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
    auto finalVal = *(char*)ag1;
    printf(" test_for_h7 >> finalVal: ag1 = %d\n", finalVal);
}
