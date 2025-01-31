#ifndef CHECK_H
#define CHECK_H

#include "BasicBlock.h"
#include "Function.h"
#include "Instruction.h"
#include "Module.h"
#include "Pass.h"

namespace SysYF {
namespace IR {

class Check: public Pass {
private:
    const std::string name = "Check";
public:
    explicit Check(WeakPtr<Module> m): Pass(m) {}
    ~Check() {}
    void execute() final;
    const std::string get_name() const override {return name;}
private:
    void checkFunction(Ptr<Function> func);
    void checkBasicBlock(Ptr<BasicBlock> bb);
    void checkInstruction(Ptr<Instruction> inst);
};

}
}
#endif