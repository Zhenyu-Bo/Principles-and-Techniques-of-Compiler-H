#ifndef SYSYF_COMSUBEXPRELI_H
#define SYSYF_COMSUBEXPRELI_H

#include "BasicBlock.h"
#include "Pass.h"
#include <map>
#include <memory>
#include <set>
#include "Instruction.h"
#include "internal_types.h"

namespace SysYF {
namespace IR {

struct cmp_expr{
    bool operator()(WeakPtr<Instruction> a, WeakPtr<Instruction> b) const {
        auto lhs = a.lock();
        auto rhs = b.lock();
        if (!lhs || !rhs) {
            return lhs < rhs;
        }
        // 按照指令类型比较
        auto opcodeL = lhs->get_instr_type();
        auto opcodeR = rhs->get_instr_type();
        if (opcodeL < opcodeR) return true;
        if (opcodeL > opcodeR) return false;

        // 指令类型相同，比较操作数数量
        int numOpsL = lhs->get_num_operand();
        int numOpsR = rhs->get_num_operand();
        if (numOpsL < numOpsR) return true;
        if (numOpsL > numOpsR) return false;

        // 比较各操作数指针地址
        for (int i = 0; i < numOpsL; i++) {
            auto opL = lhs->get_operand(i).get();
            auto opR = rhs->get_operand(i).get();
            if (opL < opR) return true;
            if (opL > opR) return false;
        }
        return false;
    }
};

/*****************************CommonSubExprElimination**************************************/
/***************************This class is based on SSA form*********************************/
class ComSubExprEli : public Pass {
public:
    explicit ComSubExprEli(WeakPtr<Module> m):Pass(m){}
    const std::string get_name() const override {return name;}
    void execute() override;
    void compute_local_gen(Ptr<Function> f);
    void compute_global_in_out(Ptr<Function> f);
    void compute_global_common_expr(Ptr<Function> f);
    /**
     * @brief init bb in/out/gen map with empty set
     * 
     * @param f 
     */
    void initial_map(Ptr<Function> f);
    static bool is_valid_expr(Ptr<Instruction> inst);
private:
    const std::string name = "ComSubExprEli";
    std::set<Ptr<Instruction>,cmp_expr> availableExprs;
    WeakPtrMap<BasicBlock, std::set<Ptr<Instruction>, cmp_expr>> bb_in;
    WeakPtrMap<BasicBlock, std::set<Ptr<Instruction>, cmp_expr>> bb_out;
    WeakPtrMap<BasicBlock, std::set<Ptr<Instruction>, cmp_expr>> bb_gen;
    bool rerun = false;
};

}
}

#endif // SYSYF_COMSUBEXPRELI_H
