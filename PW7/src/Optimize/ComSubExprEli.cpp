#include "Pass.h"
#include "ComSubExprEli.h"
#include <set>
#include <algorithm>

namespace SysYF {
namespace IR {

// void ComSubExprEli::execute() {
// }

bool ComSubExprEli::is_valid_expr(Ptr<Instruction> inst) {
    return !(
        inst->is_void() // ret, br, store, void call
        || inst->is_phi()
        || inst->is_alloca()
        || inst->is_load()
        || inst->is_call()
        || inst->is_cmp()
        || inst->is_fcmp()
    );
}

void ComSubExprEli::execute() {
    auto mod = module.lock();
    if (!mod) return;
    // 对每个函数进行公共子表达式删除
    for (auto &f : mod->get_functions()) {
        if (f->is_declaration()) continue;
        do {
            rerun = false;
            initial_map(f);
            compute_local_gen(f);
            compute_global_in_out(f);
            compute_global_common_expr(f);
        } while (rerun);
    }
}

void ComSubExprEli::initial_map(Ptr<Function> f) {
    for (auto bb : f->get_basic_blocks()) {
        bb_in[bb].clear();
        bb_out[bb].clear();
        bb_gen[bb].clear();
    }
}

void ComSubExprEli::compute_local_gen(Ptr<Function> f) {
    // 在每个基本块中找到所有“有效”表达式，存入gen集合
    for (auto bb : f->get_basic_blocks()) {
        auto &genSet = bb_gen[bb];
        genSet.clear();
        for (auto &inst : bb->get_instructions()) {
            if (is_valid_expr(inst)) {
                genSet.insert(inst);
            }
        }
    }
}

void ComSubExprEli::compute_global_in_out(Ptr<Function> f) {
    // 迭代计算可用表达式的in/out集合
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto bb : f->get_basic_blocks()) {
            // in[B] = 所有前驱块out集合的交集
            auto &inSet = bb_in[bb];
            std::set<Ptr<Instruction>, cmp_expr> newIn;
            bool firstPred = true;
            for (auto pred : bb->get_pre_basic_blocks()) {
                auto &outSet = bb_out[pred];
                if (firstPred) {
                    newIn = outSet;
                    firstPred = false;
                } else {
                    std::set<Ptr<Instruction>, cmp_expr> temp;
                    std::set_intersection(
                        newIn.begin(), newIn.end(),
                        outSet.begin(), outSet.end(),
                        std::inserter(temp, temp.begin()),
                        cmp_expr()
                    );
                    newIn = temp;
                }
            }
            if (newIn != inSet) {
                inSet = newIn;
                changed = true;
            }

            // out[B] = in[B] ∪ gen[B]
            auto &outSet = bb_out[bb];
            auto &genSet = bb_gen[bb];
            std::set<Ptr<Instruction>, cmp_expr> newOut = inSet;
            newOut.insert(genSet.begin(), genSet.end());
            if (newOut != outSet) {
                outSet = newOut;
                changed = true;
            }
        }
    }
}

void ComSubExprEli::compute_global_common_expr(Ptr<Function> f) {
    // 根据in集合对全局范围内的指令进行公共子表达式删除
    for (auto bb : f->get_basic_blocks()) {
        auto curSet = bb_in[bb];
        auto &instList = bb->get_instructions();
        for (auto it = instList.begin(); it != instList.end();) {
            auto inst = *it;
            if (!is_valid_expr(inst)) {
                ++it;
                continue;
            }
            // 如果已经有相同表达式，则替换所有使用并删除该指令
            auto found = curSet.find(inst);
            if (found != curSet.end()) {
                inst->replace_all_use_with(std::static_pointer_cast<Value>(*found));
                it = instList.erase(it);
                rerun = true;
            } else {
                // 否则将其加入集合
                curSet.insert(inst);
                ++it;
            }
        }
    }
}

}
}
