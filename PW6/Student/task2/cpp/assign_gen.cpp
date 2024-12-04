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
    auto module = Module::create("assign code");  // module name是什么无关紧要
    auto builder = IRStmtBuilder::create(nullptr, module);
    SysYF::Ptr<Type> Int32Type = Type::get_int32_type(module);
    SysYF::Ptr<Type> FloatType = Type::get_float_type(module);

    // main函数
    auto mainFun = Function::create(FunctionType::create(Int32Type, {}, module),
                                    "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);

    // 变量b赋值1.8
    auto bAlloca = builder->create_alloca(FloatType);
    builder->create_store(CONST_FP(1.8), bAlloca);

    // 数组a, a[0] = 2
    SysYF::Ptr<Type> arrayType_a = ArrayType::get(Int32Type, 2);
    auto aAlloca = builder->create_alloca(arrayType_a);
    auto a0Gep = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(0)});
    builder->create_store(CONST_INT(2), a0Gep);

    // a[1] = a[0] * b
    auto a0Load = builder->create_load(a0Gep);
    auto a0Float = builder->create_sitofp(a0Load, FloatType);  // 将a[0]转化为float
    auto bLoad = builder->create_load(bAlloca);
    auto mul = builder->create_fmul(a0Float, bLoad);
    auto mulInt = builder->create_fptosi(mul, Int32Type); // 将计算结果转化为int
    auto a1Gep = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(1)});
    builder->create_store(mulInt, a1Gep);

    // 返回a[1]
    auto a1Load = builder->create_load(a1Gep);
    builder->create_ret(a1Load);

    std::cout << module->print();
    return 0;
}