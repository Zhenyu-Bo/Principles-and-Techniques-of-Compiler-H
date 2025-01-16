#include "Check.h"
#include "Module.h"

namespace SysYF {
namespace IR {

void Check::execute() {
    auto m = module.lock();
    if(!m) return;
    for(auto &func : m->get_functions()){
        if(func->is_declaration()) continue;
        checkFunction(func);
    }
}

void Check::checkFunction(Ptr<Function> func){
    for(auto &bb : func->get_basic_blocks()){
        checkBasicBlock(bb);
    }
}

void Check::checkBasicBlock(Ptr<BasicBlock> bb){
    auto &insts = bb->get_instructions();
    if(!insts.empty()){
        auto term = insts.back();
        if(!term->isTerminator()){
            std::cerr << "[Check Error] BasicBlock without terminator.\n";
        }
    }
    for(auto &inst : insts){
        checkInstruction(inst);
    }
}

void Check::checkInstruction(Ptr<Instruction> inst){
    // 检查：Instruction 是否嵌入 BasicBlock
    auto bb = inst->get_parent();
    if (!bb) {
        std::cerr << "[Check Error] Instruction not embedded in BasicBlock.\n";
        return;
    }

    // 检查：所有操作数是否都已定义（示例：是否为 nullptr）
    int numOps = inst->get_num_operand();
    for (int i = 0; i < numOps; i++) {
        auto op = inst->get_operand(i);
        if (!op) {
            std::cerr << "[Check Error] Instruction operand not defined.\n";
        }
    }

    // 如果是分支指令，检查分支数与 BasicBlock 的后继是否匹配
    if (inst->is_br()) {
        auto &succ_blocks = bb->get_succ_basic_blocks();
        // 如果为无条件分支，则后继数量应为 1；如果为条件分支，则应为 2
        if ((inst->get_num_operand() == 1 && succ_blocks.size() != 1) ||
            (inst->get_num_operand() == 3 && succ_blocks.size() != 2)) {
            std::cerr << "[Check Error] Branch operand count does not match blocks.\n";
        }
    }

    // 如果是返回指令，简单检查返回值类型与当前函数声明是否相符
    if (inst->is_ret()) {
        auto retVal = inst->get_num_operand() > 0 ? inst->get_operand(0) : nullptr;
        auto func = bb->get_parent();
        if (func) {
            auto expectedTy = func->get_return_type();
            if (!retVal && !expectedTy->is_void_type()) {
                std::cerr << "[Check Error] Return instruction expects a value, but none found.\n";
            } else if (retVal && expectedTy->is_void_type()) {
                std::cerr << "[Check Error] Function declared void but returning a value.\n";
            }
        }
    }

    // 如果是函数调用，简单检查被调用目标是否真的是函数，及其返回类型
    if (inst->is_call()) {
        auto callee = inst->get_operand(0);
        if (auto calleeFunc = dynamic_cast<Function *>(callee.get())) {
            // 检查调用指令返回类型是否与函数定义匹配
            if (calleeFunc->get_return_type()->get_type_id() != inst->get_type()->get_type_id()) {
                std::cerr << "[Check Error] Call's return type and function's return type mismatch.\n";
            }
        } else {
            std::cerr << "[Check Error] Call operand[0] is not a valid Function.\n";
        }
    }
}

}
}

