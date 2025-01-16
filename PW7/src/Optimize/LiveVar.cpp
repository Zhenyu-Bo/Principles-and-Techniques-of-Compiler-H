#include "LiveVar.h"
#include "BasicBlock.h"
#include "Value.h"
#include "internal_types.h"
#include <fstream>
#include <cassert>

#include <algorithm>
#include <memory>

namespace SysYF
{
namespace IR
{

// Process a phi instruction from src to dst
void LiveVar::process_phi(WeakPtrSet<Value> &values, Ptr<BasicBlock> dst, Ptr<BasicBlock> src) {
    for (auto inst : dst->get_instructions()) {
        if (!inst->is_phi()) {
            continue;
        }
        for (int i = 0; i < (int)inst->get_num_operand(); i += 2) {
            auto block = dynamic_pointer_cast<BasicBlock>(inst->get_operand(i + 1));
            if (!src || block == src) {
                auto operand = inst->get_operand(i);
                if (is_local_var(operand)) {
                    // The operand is a value
                    values.insert(operand);
                }
            }
        }
    }
}

void LiveVar::execute() {
    //  请不要修改该代码。在被评测时不要在中间的代码中重新调用set_print_name
    module.lock()->set_print_name();

    for (auto func : this->module.lock()->get_functions()) {
        if (func->get_basic_blocks().empty()) {
            continue;
        } else {
            func_ = func;
            
            bool modified = true;
            while (modified) {
                modified = false;
                for (auto bb : func->get_basic_blocks()) {
                    // Record previous OUT
                    auto &out = bb->get_live_out();
                    WeakPtrSet<Value> prev_out;
                    prev_out.swap(out);
                    // Calculate OUT
                    for (auto succ : bb->get_succ_basic_blocks()) {
                        out.insert(succ.lock()->get_live_in().begin(), succ.lock()->get_live_in().end());
                        process_phi(out, succ.lock(), bb);
                    }
                    // Calculate Def & Use
                    WeakPtrSet<Value> def, use;
                    for (auto inst : bb->get_instructions()) {
                        if (inst->is_phi()) {
                            // Phi inst is somewhat complicated
                            // Process in function process_phi
                            def.insert(inst);
                            continue;
                        }
                        auto operands = inst->get_operands();
                        for (auto operand : operands) {
                            if (is_local_var(operand.lock())) {
                                // The operand is a local value
                                if (!def.count(operand)) {
                                    use.insert(operand);
                                }
                            }
                        }
                        if (!use.count(inst)) {
                            // A newly defined value
                            def.insert(inst);
                        }
                    }
                    // Record previous IN
                    auto &in = bb->get_live_in();
                    WeakPtrSet<Value> prev_in;
                    prev_in.swap(in);
                    // Calculate IN
                    in = out;
                    for (auto def_v : def) in.erase(def_v);
                    in.insert(use.begin(), use.end());
                    // Check if modified
                    // Since prev_in must be a subset of in, and prev_out must be a subset of out
                    // Checking size is enough
                    if (prev_in.size() != in.size() || prev_out.size() != out.size()) {
                        modified = true;
                    }
                }
            }
            // Post-process phi for IN
            // Since values in phi was not added into IN
            // These values should be added to IN here
            for (auto bb : func->get_basic_blocks()) {
                process_phi(bb->get_live_in(), bb);
            }
        }
    }

    // 请不要修改该代码，在被评测时不要删除该代码
    dump();
    //
    return ;
}

void LiveVar::dump() {
    std::fstream f;
    f.open(lvdump, std::ios::out);
    for (auto &func: module.lock()->get_functions()) {
        for (auto &bb: func->get_basic_blocks()) {
            f << bb->get_name() << std::endl;
            auto &in = bb->get_live_in();
            auto &out = bb->get_live_out();
            auto sorted_in = sort_by_name(in);
            auto sorted_out = sort_by_name(out);
            f << "in:\n";
            for (auto in_v: sorted_in) {
                if(in_v.lock()->get_name() != "")
                {
                    f << in_v.lock()->get_name() << " ";
                }
            }
            f << "\n";
            f << "out:\n";
            for (auto out_v: sorted_out) {
                if(out_v.lock()->get_name() != ""){
                    f << out_v.lock()->get_name() << " ";
                }
            }
            f << "\n";
        }
    }
    f.close();
}


bool ValueCmp(WeakPtr<Value> a, WeakPtr<Value> b) {
    return a.lock()->get_name() < b.lock()->get_name();
}

WeakPtrVec<Value> sort_by_name(WeakPtrSet<Value> &val_set) {
    WeakPtrVec<Value> result;
    result.assign(val_set.begin(), val_set.end());
    std::sort(result.begin(), result.end(), ValueCmp);
    return result;
}

}
}
