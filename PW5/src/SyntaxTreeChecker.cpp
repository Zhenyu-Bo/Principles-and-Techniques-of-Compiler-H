#include "SyntaxTreeChecker.h"
#include <vector>

using namespace SyntaxTree;

std::vector<SymbolTable> symbol_tables;
int current_pos = 0; // indicate current scope

void SyntaxTreeChecker::visit(Assembly& node) {
    symbol_tables.push_back(SymbolTable());
    for (auto def : node.global_defs) {
        def->accept(*this);
    }
}

void SyntaxTreeChecker::visit(FuncDef& node) {
    if (symbol_tables[current_pos].exist(node.name)) {
        err.error(node.loc, "Function duplicated.");
        exit(int(ErrorType::FuncDuplicated));
    }
    NodeTypes temp;
    temp.val_type = node.ret_type;
    temp.params_type.clear();
    for (auto param : node.param_list->params) {
        temp.params_type.push_back(param->param_type);
    }
    symbol_tables[current_pos].add_type(node.name, temp);
    node.param_list->accept(*this);
    node.body->accept(*this);
}

void SyntaxTreeChecker::visit(BinaryExpr& node) {
    node.lhs->accept(*this);
    bool lhs_int = this->Expr_int;
    node.rhs->accept(*this);
    bool rhs_int = this->Expr_int;
    if (node.op == SyntaxTree::BinOp::MODULO) {
        if (!lhs_int || !rhs_int) {
            err.error(node.loc, "Operands of modulo should be integers.");
            exit(int(ErrorType::Modulo));
        }
    }
    this->Expr_int = lhs_int & rhs_int;
    this->type = this->Expr_int ? Type::INT : Type::FLOAT;
}

void SyntaxTreeChecker::visit(UnaryExpr& node) {
    node.rhs->accept(*this);
}

void SyntaxTreeChecker::visit(LVal& node) {
    int pos = current_pos;
    while (pos >= 0 && !symbol_tables[pos].exist(node.name)) {
        pos--;
    }
    if (pos < 0) {
        err.error(node.loc, "Variable unknown.");
        exit(int(ErrorType::VarUnknown));
    }
    
    this->type = symbol_tables[pos].lookup(node.name).val_type;
    this->Expr_int = (this->type == Type::INT);

    for (auto index : node.array_index) {
        index->accept(*this);
    }
}

void SyntaxTreeChecker::visit(Literal& node) {
    this->Expr_int = (node.literal_type == SyntaxTree::Type::INT);
    this->type = node.literal_type;
}

void SyntaxTreeChecker::visit(ReturnStmt& node) {
    if (node.ret != nullptr) {
        node.ret->accept(*this);
    }
}

void SyntaxTreeChecker::visit(VarDef& node) {
    if (symbol_tables[current_pos].exist(node.name)) {
        err.error(node.loc, "Variable duplicated.");
        exit(int(ErrorType::VarDuplicated));
    }

    if (node.is_inited) {
        node.initializers->accept(*this);
    }

    for (auto length : node.array_length) {
        length->accept(*this);
        // if (!this->Expr_int) {
        //     err.error(node.loc, "Array length should be an integer.");
        //     exit(int(ErrorType::ArrayLength));
        // }
    }

    NodeTypes temp;
    temp.val_type = node.btype;
    temp.params_type.clear();
    symbol_tables[current_pos].add_type(node.name, temp);
}

void SyntaxTreeChecker::visit(AssignStmt& node) {
    node.target->accept(*this);
    node.value->accept(*this);
}

void SyntaxTreeChecker::visit(FuncCallStmt& node) {
    // check if the function exists
    int pos = current_pos;
    while (pos >= 0 && !symbol_tables[pos].exist(node.name)) {
        pos--;
    }
    if (pos < 0) {
        err.error(node.loc, "Function unknown.");
        exit(int(ErrorType::FuncUnknown));
    }

    // check if the parameters match
    NodeTypes temp = symbol_tables[pos].lookup(node.name);
    if (node.params.size() != temp.params_type.size()) {
        err.error(node.loc, "Function parameters error.");
        exit(int(ErrorType::FuncParams));
    }
    
    int i = 0;
    for (auto param : node.params) {
        param->accept(*this);
        if (this->type != temp.params_type[i]) {
            err.error(node.loc, "Function parameters error.");
            exit(int(ErrorType::FuncParams));
        }
        i++;
    }

    this->Expr_int = (temp.val_type == Type::INT);
    this->type = temp.val_type;
}

void SyntaxTreeChecker::visit(BlockStmt& node) {
    current_pos++;
    symbol_tables.push_back(SymbolTable());
    for (auto stmt : node.body) {
        stmt->accept(*this);
    }
    symbol_tables.pop_back();
    current_pos--;
}
void SyntaxTreeChecker::visit(EmptyStmt& node) {}

void SyntaxTreeChecker::visit(SyntaxTree::ExprStmt& node) {
    node.exp->accept(*this);
}

void SyntaxTreeChecker::visit(SyntaxTree::FuncParam& node) {
    if (symbol_tables[current_pos].exist(node.name)) {
        err.error(node.loc, "Variable duplicated.");
        exit(int(ErrorType::VarDuplicated));
    }
    NodeTypes temp;
    temp.val_type = node.param_type;
    temp.params_type.clear();
    symbol_tables[current_pos].add_type(node.name, temp);
    for (auto index : node.array_index) {
        index->accept(*this);
    }
}

void SyntaxTreeChecker::visit(SyntaxTree::FuncFParamList& node) {
    current_pos++;
    if (node.params.size()) {
        symbol_tables.push_back(SymbolTable());
        for (auto param : node.params) {
            param->accept(*this);
        }
    }
    current_pos--;
}
void SyntaxTreeChecker::visit(SyntaxTree::BinaryCondExpr& node) {
    node.lhs->accept(*this);
    node.rhs->accept(*this);
    this->Expr_int = true;
    this->type = Type::BOOL;
}

void SyntaxTreeChecker::visit(SyntaxTree::UnaryCondExpr& node) {
    node.rhs->accept(*this);
    this->Expr_int = true;
    this->type = Type::BOOL;
}

void SyntaxTreeChecker::visit(SyntaxTree::IfStmt& node) {
    node.if_statement->accept(*this);
    node.cond_exp->accept(*this);
    if (node.else_statement != nullptr) {
        node.else_statement->accept(*this);
    }
}

void SyntaxTreeChecker::visit(SyntaxTree::WhileStmt& node) {
    node.cond_exp->accept(*this);
    node.statement->accept(*this);
}

void SyntaxTreeChecker::visit(SyntaxTree::BreakStmt& node) {}

void SyntaxTreeChecker::visit(SyntaxTree::ContinueStmt& node) {}

void SyntaxTreeChecker::visit(SyntaxTree::InitVal& node) {
    if (node.isExp) {
        node.expr->accept(*this);
    } 
    else {
        for (auto element : node.elementList) {
            element->accept(*this);
        }
    }
}
