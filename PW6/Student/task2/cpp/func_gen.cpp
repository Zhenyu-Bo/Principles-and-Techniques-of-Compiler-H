#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRStmtBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>
#include <vector>

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
    auto module = Module::create("func code");  // module name是什么无关紧要
    auto builder = IRStmtBuilder::create(nullptr, module);
    SysYF::Ptr<Type> Int32Type = Type::get_int32_type(module);

    // add函数
    // 函数参数类型的vector
    std::vector<SysYF::Ptr<Type>> Ints(2, Int32Type);
    // 通过返回值类型与参数类型列表得到函数类型
    auto addFunTy = FunctionType::create(Int32Type, Ints, module);
    // 由函数类型得到函数
    auto addFun = Function::create(addFunTy, "add", module);
    // 创建BB
    auto bb = BasicBlock::create(module, "entry", addFun);
    builder->set_insert_point(bb);
    auto retAlloca = builder->create_alloca(Int32Type);  // 在内存中分配返回值的位置
    auto aAlloca = builder->create_alloca(Int32Type);  // 在内存中分配参数a的位置
    auto bAlloca = builder->create_alloca(Int32Type);  // 在内存中分配参数b的位置
    std::vector<SysYF::Ptr<Value>> args;  // 获取add函数的形参,通过Function中的iterator
    for (auto arg = addFun->arg_begin(); arg != addFun->arg_end(); arg++) {
        args.push_back(*arg);
    }
    builder->create_store(args[0], aAlloca);  // 存储到aAlloca
    builder->create_store(args[1], bAlloca);  // 存储到bAlloca
    auto aLoad = builder->create_load(aAlloca);  // 从aAlloca加载
    auto bLoad = builder->create_load(bAlloca);  // 从bAlloca加载
    auto add = builder->create_iadd(aLoad, bLoad);  // a + b
    // auto add = builder->create_iadd(args[0], args[1]);  // a + b
    auto sub = builder->create_isub(add, CONST_INT(1));  // a + b - 1
    builder->create_store(sub, retAlloca);  // 存储到retAlloca
    auto retLoad = builder->create_load(retAlloca);  // 从retAlloca加载
    builder->create_ret(retLoad);  // 返回retLoad

    // main函数
    auto mainFun = Function::create(FunctionType::create(Int32Type, {}, module),
                                    "main", module);
    bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
    retAlloca = builder->create_alloca(Int32Type);
    auto resAlloca = builder->create_alloca(Int32Type);
    // 局部变量a, b, c
    aAlloca = builder->create_alloca(Int32Type);
    bAlloca = builder->create_alloca(Int32Type);
    auto cAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(3), aAlloca);
    builder->create_store(CONST_INT(2), bAlloca);
    builder->create_store(CONST_INT(5), cAlloca);
    // Func call
    aLoad = builder->create_load(aAlloca);
    bLoad = builder->create_load(bAlloca);
    // std::vector<SysYF::Ptr<Value>> args2;
    // args2.push_back(aLoad);
    // args2.push_back(bLoad);
    auto addCall = builder->create_call(addFun, {aLoad, bLoad});
    builder->create_store(addCall, resAlloca);
    // c + res
    auto resLoad = builder->create_load(resAlloca);
    auto cLoad = builder->create_load(cAlloca);
    auto add2 = builder->create_iadd(resLoad, cLoad);
    builder->create_store(add2, retAlloca);
    retLoad = builder->create_load(retAlloca);
    builder->create_ret(retLoad);

    std::cout << module->print();
    return 0;
}
