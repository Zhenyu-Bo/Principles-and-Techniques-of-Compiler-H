#ifndef SYSYF_LIVEVAR_H
#define SYSYF_LIVEVAR_H

#include "Pass.h"
#include "Module.h"

namespace SysYF{
namespace IR{

class LiveVar : public Pass
{
public:
    LiveVar(WeakPtr<Module> m) : Pass(m) {}
    void execute() final;
    void process_phi(WeakPtrSet<Value> &values, Ptr<BasicBlock> dst, Ptr<BasicBlock> src = nullptr);
    const std::string get_name() const override {return name;}
    void dump();

    static bool is_local_var(Ptr<Value> val) {
        return !dynamic_pointer_cast<GlobalVariable>(val) && !dynamic_pointer_cast<Constant>(val)
            && (val->get_type()->is_pointer_type() || val->get_type()->is_array_type() || val->get_type()->is_integer_type() || val->get_type()->is_float_type());
    }
private:
    Ptr<Function> func_;
    const std::string name = "LiveVar";
};

bool ValueCmp(WeakPtr<Value> a, WeakPtr<Value> b);
WeakPtrVec<Value> sort_by_name(WeakPtrSet<Value> &val_set);
const std::string lvdump = "live_var.out";

}
}

#endif  // SYSYF_LIVEVAR_H
