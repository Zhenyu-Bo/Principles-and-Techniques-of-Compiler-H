#include "IRBuilder.h"
#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "GlobalVariable.h"
#include "Instruction.h"
#include "Module.h"
#include "SyntaxTree.h"
#include "Type.h"
#include "Value.h"
#include <memory>
#include <utility>
#include <vector>
#include <map>
#include <random>
#include <chrono>

namespace SysYF
{
namespace IR
{
#define CONST_INT(num) ConstantInt::create(num, module)
#define CONST_FLOAT(num) ConstantFloat::create(num, module)

// types
Ptr<Type> VOID_T;
Ptr<Type> INT1_T;
Ptr<Type> INT32_T;
Ptr<Type> FLOAT_T;
Ptr<Type> INT32PTR_T;
Ptr<Type> FLOATPTR_T;

// IRBuilder's private members:
// Ptr<Module> module;
// Ptr<IRStmtBuilder> builder;
// Ptr<Function> cur_func; // function analyzed currently
// Ptr<Value> visitee_val; // the Value provided by the visitee

// Global variable
Ptr<Type> cur_type = nullptr; // type of variable analyzed currently
std::vector<Ptr<Constant>> init_val; // initial value of variable analyzed currently
std::map<Ptr<Value>, Ptr<Value>> const_local_values;
bool need_addr = false; // whether need variable address to transfer value, for array is true
std::vector<Ptr<BasicBlock>> bbs;  // basic blocks of current function

// set relative variables of function analyzed currently a global variable for the convenience of access in different functions
std::vector<SyntaxTree::FuncParam> params; // parameters of current function
std::vector<Ptr<Type>> params_types; // types of parameters of current function
Ptr<Value> ret_addr; // return address of current function
Ptr<BasicBlock> retBB; // return basic block of current function
bool has_entered_scope = false; // whether a new function is analyzed currently

typedef struct CondBBs {
    Ptr<BasicBlock> trueBB;
    Ptr<BasicBlock> falseBB;
} condBBs;
std::vector<condBBs> totalCondBBs; // every member of totalConBBs is the bbs of the total condition expression
std::vector<condBBs> andCondBBs; // every member of totalConBBs is the bbs of the member of and condition expression
std::vector<condBBs> orCondBBs; // every member of totalConBBs is the bbs of the member of and condition expression
std::vector<condBBs> whileBBs; // store condBB and falseBB of while statement

// A random generator used to generate UUID
std::mt19937 random_generator(std::chrono::system_clock::now().time_since_epoch().count());

// Author: Changshu Yan
// A UUID generator
std::string generate_uuid(char split_char = '-') {
    std::uniform_int_distribution<int> distribution(0, 15);
    std::string uuid = "uuid_";
    for (int i = 0; i < 32; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            uuid += split_char;
        } else if (i == 14) {
            uuid += "4";
        } else if (i == 19) {
            uuid += std::to_string((distribution(random_generator) & 0x3) | 0x8);
        } else {
            uuid += std::to_string(distribution(random_generator));
        }
    }
    return uuid;
}

void IRBuilder::visit(SyntaxTree::Assembly &node) {
    VOID_T = Type::get_void_type(module);
    INT1_T = Type::get_int1_type(module);
    INT32_T = Type::get_int32_type(module);
    FLOAT_T = Type::get_float_type(module);
    INT32PTR_T = Type::get_int32_ptr_type(module);
    FLOATPTR_T = Type::get_float_ptr_type(module);
    for (const auto &def : node.global_defs) {
        def->accept(*this);
    }
}

// TODO: You need to fill them.
// NOTE: The following codes are just examples, you can modify them as you like.

// Author: Zhenyu Bo
// InitVal: Exp | '{'[InitVal {',' InitVal}]'}'
void IRBuilder::visit(SyntaxTree::InitVal &node) {
    if (node.isExp) {
        // InitVal: Exp, using Ptr<Expr> expr
        node.expr->accept(*this);
        // after accept method, this->visitee_val should be constant
        // check if the type of variable and init_val are the same
        auto const_int_val = this->visitee_val->as<ConstantInt>();
        auto const_float_val = this->visitee_val->as<ConstantFloat>();
        if (cur_type->is_integer_type() && const_float_val) {
            this->visitee_val = CONST_INT(int(const_float_val->get_value()));
        } else if (cur_type->is_float_type() && const_int_val) {
            this->visitee_val = CONST_FLOAT(float(const_int_val->get_value()));
        }
        auto const_val = this->visitee_val->as<Constant>();
        init_val.push_back(const_val);
    } else {
        // InitVal: '{'[InitVal {',' InitVal}]'}', using PtrVec<InitVal> elementList
        for (auto &element : node.elementList) {
            element->accept(*this);
        }
    }
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::FuncDef &node) {
    has_entered_scope = true; // a new function is analyzed currently
    // get return type
    Ptr<Type> ret_type;
    if (node.ret_type == SyntaxTree::Type::INT) {
        ret_type = INT32_T;
    } else if (node.ret_type == SyntaxTree::Type::BOOL) {
        ret_type = INT32_T; // this is a temporary solution, whether it is correct needs to be confirmed
    } else if (node.ret_type == SyntaxTree::Type::FLOAT) {
        ret_type = FLOAT_T;
    } else {
        ret_type = VOID_T;
    }
    // get paras types
    params_types.clear();
    params.clear();
    node.param_list->accept(*this);
    // create function
    auto fn = Function::create(FunctionType::create(ret_type, params_types, this->module), node.name, this->module);
    // auto fn = Function::create(FunctionType::create(INT32_T, {}, this->module), "main", this->module);
    // push fn to scope
    this->scope.push(node.name, fn);
    this->cur_func = fn;
    // create entry block and set insert point
    auto entry = BasicBlock::create(this->module, "entry" + generate_uuid('_'), fn);
    builder->set_insert_point(entry);
    bbs.push_back(entry);
    scope.enter();
    // allocate space for return value
    if (ret_type != VOID_T) {
        ret_addr = builder->create_alloca(ret_type);
    }
    // create ret BB
    retBB = BasicBlock::create(this->module, "ret" + generate_uuid('_'), fn);
    // get and store parameters
    std::vector<Ptr<Value>> args;
    for (auto arg = fn->arg_begin(); arg != fn->arg_end(); arg++) {
        args.push_back(*arg);
    }
    for (std::vector<SyntaxTree::FuncParam>::size_type i = 0; i < params.size(); i++) {
        auto var = builder->create_alloca(params_types[i]);
        builder->create_store(args[i], var);
        scope.push(params[i].name, var);
    }
    // visit body
    node.body->accept(*this);
    // if last BB does not return, create a br instruction to jump to retBB
    if (!bbs.back()->get_terminator()) {
        // bbs.back() == builder->get_insert_block()
        auto cur_ret_type = this->cur_func->get_return_type();
        if (cur_ret_type->is_void_type()) {
            builder->create_void_ret();
        } else {
            if (cur_ret_type->is_integer_type()) {
                builder->create_store(CONST_INT(0), ret_addr);
            } else if (cur_ret_type->is_float_type()) {
                builder->create_store(CONST_FLOAT(0), ret_addr);
            }
            builder->create_br(retBB);
        }
    }

    // scope.exit(); // exit scope in visit(BlockStmt)
    bbs.pop_back();

    // set insert point to retBB
    builder->set_insert_point(retBB);
    // return
    if (fn->get_return_type() != VOID_T) {
        auto ret_val = builder->create_load(ret_addr);
        builder->create_ret(ret_val);
    } else {
        builder->create_void_ret();
    }
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::FuncFParamList &node) {
    for (auto &param : node.params) {
        param->accept(*this);
    }
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::FuncParam &node) {
    params.push_back(node);
    if (node.array_index.empty()) {
        if (node.param_type == SyntaxTree::Type::INT) {
            params_types.push_back(INT32_T);
        } else if (node.param_type == SyntaxTree::Type::BOOL) {
            params_types.push_back(INT32_T);
        } else if (node.param_type == SyntaxTree::Type::FLOAT) {
            params_types.push_back(FLOAT_T);
        }
    } else {
        if (node.param_type == SyntaxTree::Type::INT) {
            params_types.push_back(INT32PTR_T);
        } else if (node.param_type == SyntaxTree::Type::BOOL) {
            params_types.push_back(INT32PTR_T);
        } else if (node.param_type == SyntaxTree::Type::FLOAT) {
            params_types.push_back(FLOATPTR_T);
        }
    }
}

// Author: Changshu Yan
// A variable definition, including global variable and local variable.
void IRBuilder::visit(SyntaxTree::VarDef &node) {
    // Fetch the type of the variable
    if (node.btype == SyntaxTree::Type::INT || node.btype == SyntaxTree::Type::BOOL) {
        cur_type = INT32_T;
    } else {
        cur_type = FLOAT_T;
    }
    Ptr<BasicBlock> entry_block, cur_block;
    if (!scope.in_global()) {
        // If it is not in the global scope
        // Fetch the function entry and the scope of current block
        entry_block = cur_func->get_entry_block();
        cur_block = bbs.back();
    }

    if (node.is_constant) {
        // Is a constant variable
        if (node.array_length.empty()) {
            // Is not an array variable
            Ptr<Value> var;
            // Calculate the initializer first
            node.initializers->accept(*this);
            if (cur_type == INT32_T) {
                // Is an integer variable
                // If the initializer is a constant float
                // A type cast is needed
                auto const_float = this->visitee_val->as<ConstantFloat>();
                if (const_float) {
                    this->visitee_val = CONST_INT(int(const_float->get_value()));
                }
                // Initialize the variable
                auto const_int = this->visitee_val->as<ConstantInt>();
                auto initializer = const_int->get_value();
                var = ConstantInt::create(initializer, module);
            } else {
                // Is a float variable
                // If the initializer is a constant integer
                // A type cast is needed
                auto const_int = this->visitee_val->as<ConstantInt>();
                if (const_int) {
                    this->visitee_val = CONST_FLOAT(float(const_int->get_value()));
                }
                // Initialize the variable
                auto const_float = this->visitee_val->as<ConstantFloat>();
                auto initializer = const_float->get_value();
                var = ConstantFloat::create(initializer, module);
            }
            scope.push(node.name, var);
        } else {
            // Is an array variable
            // Fetch the length of the array
            node.array_length[0]->accept(*this);
            auto length = this->visitee_val->as<ConstantInt>()->get_value();
            // Initialize the array
            auto array_type = ArrayType::get(cur_type, length);
            init_val.clear();
            node.initializers->accept(*this);
            if (scope.in_global()) {
                // If it is in the global scope
                // Create a global array
                int initialized_size = init_val.size();
                while (initialized_size < length) {
                    // Append empty values to the array
                    // Until the array is fully initialized
                    if (cur_type == INT32_T) {
                        init_val.push_back(CONST_INT(0));
                    } else {
                        init_val.push_back(CONST_FLOAT(0.0));
                    }
                    initialized_size++;
                }
                auto initializer = ConstantArray::create(array_type, init_val, module);
                auto global_var = GlobalVariable::create(node.name, module, array_type, true, initializer);
                scope.push(node.name, global_var);
            } else {
                // If it is not in the global scope
                // Create a local variable
                // The space of the local variable should be allocated in the entry block
                // If the entry block is terminated
                // The value should be inserted before the terminator
                auto entry_block_terminator = entry_block->get_terminator();
                if (entry_block_terminator) {
                    // If the entry block is terminated
                    // Remove the terminator first
                    entry_block->get_instructions().pop_back();
                }
                // Allocate the space for the array
                auto var = builder->create_alloca(array_type);
                // The create_alloca defaults to the current block
                // It should be moved to the entry block
                cur_block->get_instructions().pop_back();
                entry_block->add_instruction(std::dynamic_pointer_cast<Instruction>(var));
                std::dynamic_pointer_cast<Instruction>(var)->set_parent(entry_block);
                // Re-place the terminator if it is removed before
                if (entry_block_terminator) {
                    entry_block->add_instruction(entry_block_terminator);
                }
                // Initialize the array
                int id = 0;
                for (auto initializer : init_val) {
                    builder->create_store(initializer, builder->create_gep(var, {CONST_INT(0), CONST_INT(id)}));
                    id++;
                }
                // Fill the remaining elements with 0
                while (id < length) {
                    if (cur_type == INT32_T) {
                        builder->create_store(CONST_INT(0), builder->create_gep(var, {CONST_INT(0), CONST_INT(id)}));
                    } else {
                        builder->create_store(CONST_FLOAT(0.), builder->create_gep(var, {CONST_INT(0), CONST_INT(id)}));
                    }
                    id++;
                }
                scope.push(node.name, var);
                // Add to a global const array
                int initialized_size = init_val.size();
                while (initialized_size < length) {
                    // Append empty values to the array
                    // Until the array is fully initialized
                    if (cur_type == INT32_T) {
                        init_val.push_back(CONST_INT(0));
                    } else {
                        init_val.push_back(CONST_FLOAT(0.0));
                    }
                    initialized_size++;
                }
                // Create a global array
                // And store the local array to the global array
                // The name of the global array should be unique
                auto initializer = ConstantArray::create(array_type, init_val, module);
                std::string global_name = generate_uuid() + '@' + cur_func->get_name() + '.' + node.name;
                auto global_var = GlobalVariable::create(global_name, module, array_type, true, initializer);
                scope.push(global_name, global_var);
                const_local_values[var] = global_var;
            }
        }
    } else {
        // Is a mutable variable
        if (node.array_length.empty()) {
            // Is not an array variable
            if (scope.in_global()) {
                // If it is in the global scope
                if (node.is_inited) {
                    node.initializers->accept(*this);
                    Ptr<Constant> initializer;
                    if (cur_type == INT32_T) {
                        // Is an integer variable
                        // If the initializer is a constant float
                        // A type cast is needed
                        auto const_float = this->visitee_val->as<ConstantFloat>();
                        if (const_float) {
                            this->visitee_val = CONST_INT(int(const_float->get_value()));
                        }
                        // Initialize the variable
                        initializer = this->visitee_val->as<ConstantInt>();
                    } else {
                        // Is a float variable
                        // If the initializer is a constant integer
                        // A type cast is needed
                        auto const_int = this->visitee_val->as<ConstantInt>();
                        if (const_int) {
                            this->visitee_val = CONST_FLOAT(float(const_int->get_value()));
                        }
                        // Initialize the variable
                        initializer = this->visitee_val->as<ConstantFloat>();
                    }
                    // Create a global variable
                    auto global_var = GlobalVariable::create(node.name, module, cur_type, false, initializer);
                    scope.push(node.name, global_var);
                } else {
                    // Initialize the variable with 0
                    auto initializer = ConstantZero::create(cur_type, module);
                    // Create a global variable
                    auto global_var = GlobalVariable::create(node.name, module, cur_type, false, initializer);
                    scope.push(node.name, global_var);
                }
            } else {
                // If it is not in the global scope
                // Create a local variable
                // The space of the local variable should be allocated in the entry block
                // If the entry block is terminated
                // The value should be inserted before the terminator
                auto entry_block_terminator = entry_block->get_terminator();
                if (entry_block_terminator) {
                    // If the entry block is terminated
                    // Remove the terminator first
                    entry_block->get_instructions().pop_back();
                }
                // Allocate the space for the variable
                auto var = builder->create_alloca(cur_type);
                // The create_alloca defaults to the current block
                // It should be moved to the entry block
                cur_block->get_instructions().pop_back();
                auto inst = var->as<Instruction>();
                entry_block->add_instruction(inst);
                inst->set_parent(entry_block);
                // Re-place the terminator if it is removed before
                if (entry_block_terminator) {
                    entry_block->add_instruction(entry_block_terminator);
                }
                // Initialize the variable
                if (node.is_inited) {
                    node.initializers->accept(*this);
                    auto var_pointer_type = var->get_type()->get_pointer_element_type();
                    if (var_pointer_type->is_integer_type()) {
                        // If the variable is an integer variable
                        // and the initializer is a float type
                        // A type cast is needed
                        if (this->visitee_val->get_type()->is_float_type()) {
                            this->visitee_val = builder->create_fptosi(this->visitee_val, INT32_T);
                        }
                    } else if (var_pointer_type->is_float_type()) {
                        // If the variable is a float variable
                        // and the initializer is an integer type
                        // A type cast is needed
                        if (this->visitee_val->get_type()->is_integer_type()) {
                            this->visitee_val = builder->create_sitofp(this->visitee_val, FLOAT_T);
                        }
                    }
                    builder->create_store(this->visitee_val, var);
                }
                scope.push(node.name, var);
            }
        } else {
            // Is an array variable
            // Fetch the length of the array
            node.array_length[0]->accept(*this);
            auto const_int = this->visitee_val->as<ConstantInt>();
            auto length = const_int->get_value();
            if (scope.in_global()) {
                // If it is in the global scope
                if (node.is_inited) {
                    // If the array is initialized
                    // Initialize the array
                    auto array_type = ArrayType::get(cur_type, length);
                    init_val.clear();
                    node.initializers->accept(*this);
                    // Create a global array
                    int initialized_size = init_val.size();
                    while (initialized_size < length) {
                        // Append empty values to the array
                        // Until the array is fully initialized
                        if (cur_type == INT32_T) {
                            init_val.push_back(CONST_INT(0));
                        } else {
                            init_val.push_back(CONST_FLOAT(0.));
                        }
                        initialized_size++;
                    }
                    auto initializer = ConstantArray::create(array_type, init_val, module);
                    auto global_var = GlobalVariable::create(node.name, module, array_type, false, initializer);
                    scope.push(node.name, global_var);
                } else {
                    // If the array is not initialized
                    // Initialize the array with 0
                    auto array_type = ArrayType::get(cur_type, length);
                    auto initializer = ConstantZero::create(array_type, module);
                    auto global_var = GlobalVariable::create(node.name, module, array_type, false, initializer);
                    scope.push(node.name, global_var);
                }
            } else {
                // If it is not in the global scope
                // Create a local variable
                // The space of the local variable should be allocated in the entry block
                // If the entry block is terminated
                // The value should be inserted before the terminator
                auto entry_block_terminator = entry_block->get_terminator();
                if (entry_block_terminator) {
                    // If the entry block is terminated
                    // Remove the terminator first
                    entry_block->get_instructions().pop_back();
                }
                // Allocate the space for the array
                auto array_type = ArrayType::get(cur_type, length);
                auto var = builder->create_alloca(array_type);
                // The create_alloca defaults to the current block
                // It should be moved to the entry block
                cur_block->get_instructions().pop_back();
                auto inst = var->as<Instruction>();
                entry_block->add_instruction(inst);
                inst->set_parent(entry_block);
                // Re-place the terminator if it is removed before
                if (entry_block_terminator) {
                    entry_block->add_instruction(entry_block_terminator);
                }
                // Initialize the array
                if (node.is_inited) {
                    init_val.clear();
                    node.initializers->accept(*this);
                    int id = 0;
                    for (auto initializer : init_val) {
                        auto ptr = builder->create_gep(var, {CONST_INT(0), CONST_INT(id)});
                        builder->create_store(initializer, ptr);
                        id++;
                    }
                    // Fill the remaining elements with 0
                    while (id < length) {
                        auto ptr = builder->create_gep(var, {CONST_INT(0), CONST_INT(id)});
                        if (cur_type == INT32_T) {
                            builder->create_store(CONST_INT(0), ptr);
                        } else {
                            builder->create_store(CONST_FLOAT(0.0), ptr);
                        }
                        id++;
                    }
                }
                scope.push(node.name, var);
            }
        }
    }
}

// Author: Zhenyu Bo (if part) and Changshu Yan (else part)
// Expression like `ident` or `ident[exp]`.
void IRBuilder::visit(SyntaxTree::LVal &node) {
    // get variable
    auto lval = scope.find(node.name, false)->as<Value>();
    bool ret_is_addr = need_addr;
    need_addr = false;
    if (node.array_index.empty()) {
        // ident
        // auto var = scope.find(node.name, false)->as<Value>();
        if (ret_is_addr) {
            // this->visitee_val needs to be set as an address
            auto lval_type = lval->get_type()->get_pointer_element_type();
            if (lval_type->is_pointer_type()) {
                // lval is a pointer to pointer
                this->visitee_val = builder->create_load(lval);
            } else if (lval_type->is_array_type()) {
                // lval is a pointer to an array
                this->visitee_val = builder->create_gep(lval, {CONST_INT(0), CONST_INT(0)});
            } else {
                // lval is a pointer to a simple variable
                this->visitee_val = lval;
            }
        } else {
            // we need to load the value
            // if the lval is constant, we can load the value directly
            auto const_int = lval->as<ConstantInt>();
            auto const_float = lval->as<ConstantFloat>();
            if (const_int) {
                this->visitee_val = const_int;
            } else if (const_float) {
                this->visitee_val = const_float;
            } else {
                this->visitee_val = builder->create_load(lval);
            }
            // this->visitee_val = builder->create_load(lval);
        }
    } else {
        // ident[exp]
        // get array index, array has been limited to one dimension in this experiment
        node.array_index[0]->accept(*this);
        auto idx = this->visitee_val;
        auto const_idx = std::dynamic_pointer_cast<ConstantInt>(idx);
        // Is const index?
        if (const_idx) {
            // Is global array?
            auto global_cast = std::dynamic_pointer_cast<GlobalVariable>(lval);
            if (global_cast) {
                // If the array is const and the index is const
                // Then the value is const and can be directly loaded
                if (global_cast->is_const()) {
                    auto init_arr = std::dynamic_pointer_cast<ConstantArray>(global_cast->get_init());
                    auto idx_val = const_idx->get_value();
                    this->visitee_val = init_arr->get_element_value(idx_val);
                    return;
                }
                // Otherwise the index is const but the array is not const
                // The value cannot be loaded directly
            }
            // Is local array?
            // If is local const array, it can be converted to global const array
            auto local_cast = const_local_values[lval];
            if (local_cast) {
                global_cast = local_cast->as<GlobalVariable>();
                if (global_cast) {
                    // If the array is const and the index is const
                    // Then the value is const and can be directly loaded
                    if (global_cast->is_const()) {
                        auto init_arr = global_cast->get_init()->as<ConstantArray>();
                        auto idx_val = const_idx->get_value();
                        this->visitee_val = init_arr->get_element_value(idx_val);
                        return;
                    }
                    // Otherwise the index is const but the array is not const
                    // The value cannot be loaded directly
                }
            }
        }
        // If is not const index, the value is not const
        Ptr<Value> loaded_value;
        auto lval_pointer_type = lval->get_type()->get_pointer_element_type();
        if (lval_pointer_type->is_pointer_type()) {
            // The array is actually a pointer aka. ptr[exp]
            // Create gep by calculating pointer offset
            auto ptr_load = builder->create_load(lval);
            loaded_value = builder->create_gep(ptr_load, {idx});
        } else {
            // Otherwise the array is a real array aka. arr[exp]
            // Create gep using array method
            loaded_value = builder->create_gep(lval, {CONST_INT(0), idx});
        }
        if (ret_is_addr) {
            // If the lvalue should be used as an addr
            // which will be modified in outer scope
            this->visitee_val = loaded_value;
            // need_addr = false;
        } else {
            // If the lvalue is only used as a value
            this->visitee_val = builder->create_load(loaded_value);
        }
    }
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::AssignStmt &node) {
    node.value->accept(*this);
    auto val = this->visitee_val;
    need_addr = true; // need variable address to store value
    node.target->accept(*this);
    auto var_addr = this->visitee_val;
    // check if the type of res and type of variable match
    auto res_type = val->get_type();
    auto var_type = var_addr->get_type()->get_pointer_element_type();
    if (var_type->is_float_type() && res_type->is_integer_type()) {
        // assignment is 'float = int'
        if (res_type == INT1_T) {
            val = builder->create_zext(val, INT32_T);
        }
        val = builder->create_sitofp(val, FLOAT_T);
    } else if (var_type->is_integer_type() && res_type->is_float_type()) {
        // assignment is 'int = float'
        val = builder->create_fptosi(val, INT32_T);
    }
    // store
    builder->create_store(val, var_addr);
    // update visitee_val
    this->visitee_val = val;
}

void IRBuilder::visit(SyntaxTree::Literal &node) {
    switch (node.literal_type)
    {
    case SyntaxTree::Type::INT: {
        this->visitee_val = CONST_INT(node.int_const);
        break;
    }
    case SyntaxTree::Type::FLOAT: {
        this->visitee_val = CONST_FLOAT(node.float_const);
        break;
    }
    default:
        throw UnreachableException();
        break;
    }
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::ReturnStmt &node) {
    if (node.ret == nullptr) {
        builder->create_void_ret();
        return;
    }
    node.ret->accept(*this);
    auto cur_ret_type = this->cur_func->get_return_type();
    auto ret_val_type = this->visitee_val->get_type();
    if (cur_ret_type->is_integer_type() && ret_val_type->is_float_type()) {
        this->visitee_val = builder->create_fptosi(this->visitee_val, INT32_T);
        // if (cur_ret_type == INT1_T) {
            
        // }
    } else if (cur_ret_type->is_float_type() && ret_val_type->is_integer_type()) {
        if (ret_val_type == INT1_T) {
            this->visitee_val = builder->create_zext(this->visitee_val, INT32_T);
        }
        this->visitee_val = builder->create_sitofp(this->visitee_val, FLOAT_T);
    }
    builder->create_store(this->visitee_val, ret_addr);
    builder->create_br(retBB);
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::BlockStmt &node) {
    if (has_entered_scope) {
        // if the function analyzed currently has entered scope, we should not enter scope again
        // and we should set has_entered_scope to false, so that the block statement in the function can enter scope
        has_entered_scope = false;
    } else {
        scope.enter();
    }
    // scope.enter();
    for (auto stmt: node.body) {
        stmt->accept(*this);
        if (builder->get_insert_block()->get_terminator()) {
            // if return statement occurs, no need to visit the rest of statements
            break;
        }
    }
    scope.exit();
}

void IRBuilder::visit(SyntaxTree::EmptyStmt &node) {}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::ExprStmt &node) {
    node.exp->accept(*this);
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::UnaryCondExpr &node) {
    if (node.op == SyntaxTree::UnaryCondOp::NOT) {
        node.rhs->accept(*this);
        auto cond_expr_type = this->visitee_val->get_type();
        if (cond_expr_type->is_integer_type()) {
            this->visitee_val = builder->create_icmp_eq(this->visitee_val, CONST_INT(0));
        } else if (cond_expr_type->is_float_type()) {
            this->visitee_val = builder->create_fcmp_eq(this->visitee_val, CONST_FLOAT(0));
        }
        // now visitee_val is boolean type, that is INT1_T, we should zero extend it
        this->visitee_val = builder->create_zext(this->visitee_val, INT32_T);
    }
}

void createCondBr(Ptr<IRStmtBuilder> builder, Ptr<Value> val, Ptr<BasicBlock> trueBB, Ptr<BasicBlock> falseBB, Ptr<Module> module) {
    // val may be also a compare instruction. In this case we don't need to create a compare inst
    auto val_type = val->get_type();
    auto cmp_int = val->as<CmpInst>();
    auto cmp_float = val->as<FCmpInst>();
    if (val_type->is_integer_type() || cmp_int) {
        // try to convert val to Ptr<CmpInst>
        // auto cmp = std::dynamic_pointer_cast<CmpInst>(val);
        // auto cmp = val->as<CmpInst>();
        if (!cmp_int) {
            // if conversion fails, we should construct CmpInst manually
            cmp_int = builder->create_icmp_ne(val, CONST_INT(0));
        }
        builder->create_cond_br(cmp_int, trueBB, falseBB);
    } else if (val_type->is_float_type() || cmp_float) {
        // auto cmp = std::dynamic_pointer_cast<FCmpInst>(val);
        // auto cmp = val->as<FCmpInst>();
        if (!cmp_float) {
            cmp_float = builder->create_fcmp_ne(val, CONST_FLOAT(0));
        }
        builder->create_cond_br(cmp_float, trueBB, falseBB);
    }
}

// Author: Zhenyu Bo
// BinaryCondOp: LT, LTE, GT, GTE, EQ, NEQ, LAND, LOR
void IRBuilder::visit(SyntaxTree::BinaryCondExpr &node) {
    if (node.op == SyntaxTree::BinaryCondOp::LAND) {
        auto andTrueBB = BasicBlock::create(module, "andTrueBB" + generate_uuid('_'), this->cur_func);
        // and cond's falseBB is the same with the total cond's falseBB without considering short circuit calculation
        auto andFalseBB = totalCondBBs.back().falseBB;
        andCondBBs.push_back({andTrueBB, andFalseBB});
        node.lhs->accept(*this);
        andCondBBs.pop_back();
        createCondBr(builder, this->visitee_val, andTrueBB, andFalseBB, module);
        // if lhs is true, continue to process rhs
        builder->set_insert_point(andTrueBB);
        node.rhs->accept(*this);
    } else if (node.op == SyntaxTree::BinaryCondOp::LOR) {
        // or cond's trueBB is the with total cond's trueBB
        auto orTrueBB = totalCondBBs.back().trueBB;
        auto orFalseBB = BasicBlock::create(module, "orFalseBB" + generate_uuid('_'), this->cur_func);
        orCondBBs.push_back({orTrueBB, orFalseBB});
        node.lhs->accept(*this);
        orCondBBs.pop_back();
        createCondBr(builder, this->visitee_val, orTrueBB, orFalseBB, module);
        // if lhs is false, continue to process rhs
        builder->set_insert_point(orFalseBB);
        node.rhs->accept(*this);
    } else {
        node.lhs->accept(*this);
        auto lval = this->visitee_val;
        node.rhs->accept(*this);
        auto rval = this->visitee_val;
        auto lval_type = lval->get_type();
        auto rval_type = rval->get_type();
        if (lval_type == INT1_T) {
            lval = builder->create_zext(lval, INT32_T);
        }
        if (rval_type == INT1_T) {
            rval = builder->create_zext(rval, INT32_T);
        }
        if (lval_type->is_integer_type() && rval_type->is_integer_type()) {
            switch (node.op) {
            case SysYF::SyntaxTree::BinaryCondOp::LT:
                this->visitee_val = builder->create_icmp_lt(lval, rval); break;
            case SysYF::SyntaxTree::BinaryCondOp::LTE:
                this->visitee_val = builder->create_icmp_le(lval, rval); break;
            case SysYF::SyntaxTree::BinaryCondOp::GT:
                this->visitee_val = builder->create_icmp_gt(lval, rval); break;
            case SysYF::SyntaxTree::BinaryCondOp::GTE:
                this->visitee_val = builder->create_icmp_ge(lval, rval); break;
            case SysYF::SyntaxTree::BinaryCondOp::EQ:
                this->visitee_val = builder->create_icmp_eq(lval, rval); break;
            case SysYF::SyntaxTree::BinaryCondOp::NEQ:
                this->visitee_val = builder->create_icmp_ne(lval, rval); break;
            default: break;
            }
        } else {
            if (lval_type->is_integer_type()) {
                lval = builder->create_sitofp(lval, FLOAT_T);
            }
            if (rval_type->is_integer_type()) {
                rval = builder->create_sitofp(rval, FLOAT_T);
            }
            switch (node.op) {
            case SysYF::SyntaxTree::BinaryCondOp::LT:
                this->visitee_val = builder->create_fcmp_lt(lval, rval); break;
            case SysYF::SyntaxTree::BinaryCondOp::LTE:
                this->visitee_val = builder->create_fcmp_le(lval, rval); break;
            case SysYF::SyntaxTree::BinaryCondOp::GT:
                this->visitee_val = builder->create_fcmp_gt(lval, rval); break;
            case SysYF::SyntaxTree::BinaryCondOp::GTE:
                this->visitee_val = builder->create_fcmp_ge(lval, rval); break;
            case SysYF::SyntaxTree::BinaryCondOp::EQ:
                this->visitee_val = builder->create_fcmp_eq(lval, rval); break;
            case SysYF::SyntaxTree::BinaryCondOp::NEQ:
                this->visitee_val = builder->create_fcmp_ne(lval, rval); break;
            default: break;
            }
        }
        // now visitee_val is boolean type, that is INT1_T, we should zero extend it
        this->visitee_val = builder->create_zext(this->visitee_val, INT32_T);
    }
}

// Author: Zhenyu Bo
// BinaryOp: PLUS | MINUS | MULTIPLY | DIVIDE | MODULO
void IRBuilder::visit(SyntaxTree::BinaryExpr &node) {
    if (!node.rhs) {
        // it is an unary expression
        node.lhs->accept(*this);
        return;
    }
    node.lhs->accept(*this);
    auto lval = this->visitee_val;
    node.rhs->accept(*this);
    auto rval = this->visitee_val;
    // if lval or rval is boolean, we should convert it to integer
    if (lval->get_type() == INT1_T) {
        lval = builder->create_zext(lval, INT32_T);
    }
    if (rval->get_type() == INT1_T) {
        rval = builder->create_zext(rval, INT32_T);
    }
    // check if the type of lval and rval match
    // if not, we should convert int to float
    // if (lval->get_type()->is_float_type() || rval->get_type()->is_float_type()) {
    //     if (lval->get_type()->is_integer_type()) {
    //         lval = builder->create_sitofp(lval, FLOAT_T);
    //     }
    //     if (rval->get_type()->is_integer_type()) {
    //         rval = builder->create_sitofp(rval, FLOAT_T);
    //     }
    // }
    // After the above conversion, the type of lval and rval should be the same
    // that is both are integer or both are float
    // check if lval and rval are constant value
    auto lval_const_int = lval->as<ConstantInt>();
    auto lval_const_float = lval->as<ConstantFloat>();
    // if lval_const_int or lval_const_float is not nullptr, lval is a constant value
    bool lval_is_const = lval_const_int || lval_const_float;
    auto rval_const_int = rval->as<ConstantInt>();
    auto rval_const_float = rval->as<ConstantFloat>();
    bool rval_is_const = rval_const_int || rval_const_float;
    // get types of lval and rval
    auto lval_type = lval->get_type();
    auto rval_type = rval->get_type();
    
    if (lval_is_const && rval_is_const) {
        // if both lval and rval are constant, then we just need to process the constant value
        if (lval_const_int && rval_const_int) {
            // both lval and rval are const int
            // get value
            auto lval_const_int_val = lval_const_int->get_value();
            auto rval_const_int_val = rval_const_int->get_value();
            // calculate
            switch (node.op) {
            case SysYF::SyntaxTree::BinOp::PLUS:
                this->visitee_val = CONST_INT(lval_const_int_val + rval_const_int_val); break;
            case SysYF::SyntaxTree::BinOp::MINUS:
                this->visitee_val = CONST_INT(lval_const_int_val - rval_const_int_val); break;
            case SysYF::SyntaxTree::BinOp::MULTIPLY:
                this->visitee_val = CONST_INT(lval_const_int_val * rval_const_int_val); break;
            case SysYF::SyntaxTree::BinOp::DIVIDE:
                this->visitee_val = CONST_INT(lval_const_int_val / rval_const_int_val); break;
            case SysYF::SyntaxTree::BinOp::MODULO:
                this->visitee_val = CONST_INT(lval_const_int_val % rval_const_int_val); break;
            }
        } else {
            // lval or rval is const float
            // get value
            float lval_const_float_val = lval_const_float != nullptr ? lval_const_float->get_value() : float(lval_const_int->get_value());
            float rval_const_float_val = rval_const_float != nullptr ? rval_const_float->get_value() : float(rval_const_int->get_value());
            switch (node.op) {
            case SysYF::SyntaxTree::BinOp::PLUS:
                this->visitee_val = CONST_FLOAT(lval_const_float_val + rval_const_float_val); break;
            case SysYF::SyntaxTree::BinOp::MINUS:
                this->visitee_val = CONST_FLOAT(lval_const_float_val - rval_const_float_val); break;
            case SysYF::SyntaxTree::BinOp::MULTIPLY:
                this->visitee_val = CONST_FLOAT(lval_const_float_val * rval_const_float_val); break;
            case SysYF::SyntaxTree::BinOp::DIVIDE:
                this->visitee_val = CONST_FLOAT(lval_const_float_val / rval_const_float_val); break;
            case SysYF::SyntaxTree::BinOp::MODULO:
                std::cerr << "invalid operands for modulo!" << std::endl; exit(-1);
                // break;
            }
        }
    } else {
        if (lval_type->is_integer_type() && rval_type->is_integer_type()) {
            switch (node.op) {
            case SysYF::SyntaxTree::BinOp::PLUS:
                this->visitee_val = builder->create_iadd(lval, rval); break;
            case SysYF::SyntaxTree::BinOp::MINUS:
                this->visitee_val = builder->create_isub(lval, rval); break;
            case SysYF::SyntaxTree::BinOp::MULTIPLY:
                this->visitee_val = builder->create_imul(lval, rval); break;
            case SysYF::SyntaxTree::BinOp::DIVIDE:
                this->visitee_val = builder->create_isdiv(lval, rval); break;
            case SysYF::SyntaxTree::BinOp::MODULO:
                this->visitee_val = builder->create_isrem(lval, rval); break;
            }
        } else {
            if (lval_type->is_integer_type()) {
                lval = builder->create_sitofp(lval, FLOAT_T);
            }
            if (rval_type->is_integer_type()) {
                rval = builder->create_sitofp(rval, FLOAT_T);
            }
            switch (node.op) {
            case SysYF::SyntaxTree::BinOp::PLUS:
                this->visitee_val = builder->create_fadd(lval, rval); break;
            case SysYF::SyntaxTree::BinOp::MINUS:
                this->visitee_val = builder->create_fsub(lval, rval); break;
            case SysYF::SyntaxTree::BinOp::MULTIPLY:
                this->visitee_val = builder->create_fmul(lval, rval); break;
            case SysYF::SyntaxTree::BinOp::DIVIDE:
                this->visitee_val = builder->create_fdiv(lval, rval); break;
            case SysYF::SyntaxTree::BinOp::MODULO:
                std::cerr << "invalid operands for modulo!" << std::endl; exit(-1);
            }
        }
    }
}

// Author: Zhenyu Bo
// UnaryOp = PLUS | MINUS
void IRBuilder::visit(SyntaxTree::UnaryExpr &node) {
    node.rhs->accept(*this);
    // if node.op == Minus, we need to negate the value, just subtracting visitee_val from 0
    if (node.op == SyntaxTree::UnaryOp::MINUS) {
        // check if the operand is constant
        auto val_const_int = this->visitee_val->as<ConstantInt>();
        auto val_const_float = this->visitee_val->as<ConstantFloat>();
        // if the operand is constant, we just need to process the constant value
        if (val_const_int != nullptr) {
            this->visitee_val = CONST_INT(0 - val_const_int->get_value());
        } else if (val_const_float != nullptr) {
            this->visitee_val = CONST_FLOAT(0 - val_const_float->get_value());
        } else {
            if (this->visitee_val->get_type()->is_integer_type()) {
                this->visitee_val = builder->create_isub(CONST_INT(0), this->visitee_val);
            } else if (this->visitee_val->get_type()->is_float_type()) {
                this->visitee_val = builder->create_fsub(CONST_FLOAT(0), this->visitee_val);
            }
        }
    }
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::FuncCallStmt &node) {
    // get function
    auto func = scope.find(node.name, true)->as<Function>();
    // set parameters
    std::vector<Ptr<Value>> caller_params;
    for (std::vector<SyntaxTree::FuncParam>::size_type i = 0; i < node.params.size(); i++) {
        // need_addr = false; // reset need_addr
        auto param = node.params[i];
        auto param_type = func->get_function_type()->get_param_type(i);
        // if param is an int or float, we can pass the value directly, so we set need_addr to false
        // but if param is an array, we need to pass the address, then we should set need_addr to true
        need_addr = (param_type->is_integer_type() || param_type->is_float_type()) ? false : true;
        param->accept(*this);
        // check if the type of param and type of variable match
        auto val_type = this->visitee_val->get_type();
        if (param_type->is_float_type() && val_type->is_integer_type()) {
            if (val_type == INT1_T) {
                this->visitee_val = builder->create_zext(this->visitee_val, INT32_T);
            }
            auto const_int = this->visitee_val->as<ConstantInt>();
            if (const_int) {
                this->visitee_val = CONST_FLOAT(float(const_int->get_value()));
            } else {
                this->visitee_val = builder->create_sitofp(this->visitee_val, FLOAT_T);
            }
            // this->visitee_val = builder->create_sitofp(this->visitee_val, FLOAT_T);
        } else if (param_type->is_integer_type() && val_type->is_float_type()) {
            auto const_float = this->visitee_val->as<ConstantFloat>();
            if (const_float) {
                this->visitee_val = CONST_INT(int(const_float->get_value()));
            } else {
                this->visitee_val = builder->create_fptosi(this->visitee_val, INT32_T);
            }
        }
        // push param to caller_params
        caller_params.push_back(this->visitee_val);
    }
    this->visitee_val = builder->create_call(func, caller_params);
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::IfStmt &node) {
    std::string if_uuid = generate_uuid('_');
    // create BasicBlocks
    auto ifTrueBB = BasicBlock::create(module, "trueBB_if" + if_uuid, this->cur_func);
    auto ifFalseBB = BasicBlock::create(module, "falseBB_if" + if_uuid, this->cur_func);
    auto ifNextBB = BasicBlock::create(module, "nextBB_if" + if_uuid, this->cur_func);
    condBBs curCondBB = {ifTrueBB, ifFalseBB};
    if (node.else_statement == nullptr) {
        // if there is no else statement, we should set falseBB to nextBB
        curCondBB.falseBB = ifNextBB;
    }
    totalCondBBs.push_back(curCondBB);
    node.cond_exp->accept(*this);
    totalCondBBs.pop_back();
    // create cond br
    createCondBr(builder, this->visitee_val, curCondBB.trueBB, curCondBB.falseBB, module);
    bbs.pop_back();
    // set insert point to trueBB and push it to bbs
    builder->set_insert_point(curCondBB.trueBB);
    bbs.push_back(curCondBB.trueBB);
    // visit if_statement
    auto if_block = std::dynamic_pointer_cast<SyntaxTree::BlockStmt>(node.if_statement);
    if (if_block != nullptr) {
        if_block->accept(*this);
    } else {
        // if if_statement is not a block statement, we should enter and exit scope manually
        scope.enter();
        node.if_statement->accept(*this);
        scope.exit();
    }
    // if if_statement does not have a terminator, we should create a br instruction to jump to nextBB
    if (!bbs.back()->get_terminator()) {
        builder->create_br(ifNextBB);
    }
    // pop trueBB from bbs
    bbs.pop_back();
    // set insert point to falseBB and push it to bbs
    if (node.else_statement != nullptr) {
        builder->set_insert_point(curCondBB.falseBB);
        bbs.push_back(curCondBB.falseBB);
        // visit else_statement
        auto else_block = std::dynamic_pointer_cast<SyntaxTree::BlockStmt>(node.else_statement);
        if (else_block != nullptr) {
            else_block->accept(*this);
        } else {
            // if else_statement is not a block statement, we should enter and exit scope manually
            scope.enter();
            node.else_statement->accept(*this);
            scope.exit();
        }
        // if else_statement does not have a terminator, we should create a br instruction to jump to nextBB
        if (!builder->get_insert_block()->get_terminator()) {
            builder->create_br(ifNextBB);
        }
        // pop falseBB from bbs
        bbs.pop_back();
    } else {
        ifFalseBB->erase_from_parent();
    }
    // set insert point to nextBB
    builder->set_insert_point(ifNextBB);
    bbs.push_back(ifNextBB);

    if (ifNextBB->get_pre_basic_blocks().empty()) {
        builder->set_insert_point(ifTrueBB);
        ifNextBB->erase_from_parent();
    }
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::WhileStmt &node) {
    std::string while_uuid = generate_uuid('_');
    // create BasicBlocks
    auto whileCondBB = BasicBlock::create(module, "condBB_while" + while_uuid, this->cur_func);
    auto whileTrueBB = BasicBlock::create(module, "trueBB_while" + while_uuid, this->cur_func);
    auto whileFalseBB = BasicBlock::create(module, "falseBB_while" + while_uuid, this->cur_func);
    whileBBs.push_back({whileCondBB, whileFalseBB}); // condBB is used for continue statement, falseBB is used for break statement
    // if previous basic block does not have a terminator, we should create a br instruction to jump to while statement
    if (!bbs.back()->get_terminator()) {
        builder->create_br(whileCondBB);
    }
    bbs.pop_back();
    // process cond_exp
    builder->set_insert_point(whileCondBB);
    CondBBs curCondBB = {whileTrueBB, whileFalseBB};
    totalCondBBs.push_back(curCondBB);
    node.cond_exp->accept(*this);
    totalCondBBs.pop_back();
    // create cond br
    createCondBr(builder, this->visitee_val, whileTrueBB, whileFalseBB, module);
    // set insert point to trueBB and push it to bbs
    builder->set_insert_point(whileTrueBB);
    bbs.push_back(whileTrueBB);
    // visit while_statement
    auto while_block = std::dynamic_pointer_cast<SyntaxTree::BlockStmt>(node.statement);
    if (while_block != nullptr) {
        while_block->accept(*this);
    } else {
        // if while_statement is not a block statement, we should enter and exit scope manually
        scope.enter();
        node.statement->accept(*this);
        scope.exit();
    }
    // if while_statement does not have a terminator, we should create a br instruction to jump to condBB
    if (!bbs.back()->get_terminator()) {
        builder->create_br(whileCondBB);
    }
    // pop trueBB from bbs
    bbs.pop_back();
    // set insert point to falseBB and push it to bbs
    builder->set_insert_point(whileFalseBB);
    bbs.push_back(whileFalseBB);
    // pop whileBB from whileBBs
    whileBBs.pop_back();
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::BreakStmt &node) {
    // get current whileBB
    auto whileBB = whileBBs.back();
    // create a br instruction to jump to falseBB
    builder->create_br(whileBB.falseBB);
}

// Author: Zhenyu Bo
void IRBuilder::visit(SyntaxTree::ContinueStmt &node) {
    // get current whileBB
    auto whileBB = whileBBs.back();
    // create a br instruction to jump to condBB, which is named as "trueBB" in whileBB
    builder->create_br(whileBB.trueBB);
}

}
}
