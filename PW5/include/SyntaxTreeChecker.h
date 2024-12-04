#ifndef _C1_SYNTAX_TREE_CHECKER_H_
#define _C1_SYNTAX_TREE_CHECKER_H_

#include <cassert>
#include <map>
#include "ErrorReporter.h"
#include "SyntaxTree.h"

class SyntaxTreeChecker : public SyntaxTree::Visitor {
   public:
    SyntaxTreeChecker(ErrorReporter& e) : err(e) {}
    virtual void visit(SyntaxTree::Assembly& node) override;
    virtual void visit(SyntaxTree::FuncDef& node) override;
    virtual void visit(SyntaxTree::BinaryExpr& node) override;
    virtual void visit(SyntaxTree::UnaryExpr& node) override;
    virtual void visit(SyntaxTree::LVal& node) override;
    virtual void visit(SyntaxTree::Literal& node) override;
    virtual void visit(SyntaxTree::ReturnStmt& node) override;
    virtual void visit(SyntaxTree::VarDef& node) override;
    virtual void visit(SyntaxTree::AssignStmt& node) override;
    virtual void visit(SyntaxTree::FuncCallStmt& node) override;
    virtual void visit(SyntaxTree::BlockStmt& node) override;
    virtual void visit(SyntaxTree::EmptyStmt& node) override;
    virtual void visit(SyntaxTree::ExprStmt& node) override;
    virtual void visit(SyntaxTree::FuncParam& node) override;
    virtual void visit(SyntaxTree::FuncFParamList& node) override;
    virtual void visit(SyntaxTree::BinaryCondExpr& node) override;
    virtual void visit(SyntaxTree::UnaryCondExpr& node) override;
    virtual void visit(SyntaxTree::IfStmt& node) override;
    virtual void visit(SyntaxTree::WhileStmt& node) override;
    virtual void visit(SyntaxTree::BreakStmt& node) override;
    virtual void visit(SyntaxTree::ContinueStmt& node) override;
    virtual void visit(SyntaxTree::InitVal& node) override;

   private:
    ErrorReporter& err;
    bool Expr_int;
    SyntaxTree::Type type;
};

enum class ErrorType {
    Accepted = 0,
    Modulo,
    VarUnknown,
    VarDuplicated,
    FuncUnknown,
    FuncDuplicated,
    FuncParams
};

struct NodeTypes {
    SyntaxTree::Type val_type;
    std::vector<SyntaxTree::Type> params_type; // empty for VarDef
};

class SymbolTable {
private:
    std::map<std::string, NodeTypes> symbol_table; // symbol table
public:
    SymbolTable() {};
    ~SymbolTable() {};

    NodeTypes lookup(std::string name) {
        if (symbol_table.find(name) == symbol_table.end()) {
            // not found
            return NodeTypes();
        }
        return symbol_table[name];
    }

    void add_type(std::string name, NodeTypes prop) {
        if (symbol_table.find(name) != symbol_table.end()) {
            // duplicated
            return;
        }
        symbol_table[name] = prop;
    }

    bool exist(std::string name) {
        return symbol_table.find(name) != symbol_table.end();
    }
};

#endif  // _C1_SYNTAX_TREE_CHECKER_H_