#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRStmtBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG  // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;  // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::create(num, module)

#define CONST_FP(num) \
    ConstantFloat::create(num, module) // 得到常数值的表示,方便后面多次用到

using namespace SysYF::IR;

int main() {
    auto module = Module::create("if code");
    auto builder = IRStmtBuilder::create(nullptr, module);
    SysYF::Ptr<Type> Int32Type = Type::get_int32_type(module);

    // 全局变量a
    auto zero_initializer = ConstantZero::create(Int32Type, module);
    auto a = GlobalVariable::create("a", module, Int32Type, false, zero_initializer);

    // main函数
    auto mainFun = Function::create(FunctionType::create(Int32Type, {}, module),
                                    "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
    auto retAlloc = builder->create_alloca(Int32Type); // 在内存中分配返回值位置
    // a = 10
    builder->create_store(CONST_INT(10), a);

    auto retBB = BasicBlock::create(module, "ret", mainFun); // ret分支
    auto trueBB = BasicBlock::create(module, "trueBB_if", mainFun); // true分支
    auto falseBB = BasicBlock::create(module, "falseBB_if", mainFun); // false分支

    auto aLoad = builder->create_load(a);
    auto icmp = builder->create_icmp_gt(aLoad, CONST_INT(0));
    builder->create_cond_br(icmp, trueBB, falseBB); // 条件BR

    builder->set_insert_point(trueBB);
    builder->create_store(aLoad, retAlloc);
    builder->create_br(retBB);

    builder->set_insert_point(falseBB);
    builder->create_store(CONST_INT(0), retAlloc);
    builder->create_br(retBB);

    builder->set_insert_point(retBB);
    auto retLoad = builder->create_load(retAlloc);
    builder->create_ret(retLoad);

    std::cout << module->print();
    return 0;
}