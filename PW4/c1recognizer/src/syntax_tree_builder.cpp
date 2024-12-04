
#include <c1recognizer/syntax_tree_builder.h>
#include <memory>
#include <iomanip>

using namespace c1_recognizer;
using namespace c1_recognizer::syntax_tree;

// You should use std::any_cast instead of dynamic_cast or static_cast
template<typename T>
bool is(std::any operand) {
    return std::any_cast<T>(&operand) != nullptr;
}

// Return pointer to the value of std::any object
template<typename T>
auto as(std::any operand) {
    return *(std::any_cast<T>(&operand));
}

syntax_tree_builder::syntax_tree_builder(error_reporter &_err) : err(_err) {}

antlrcpp::Any syntax_tree_builder::visitCompilationUnit(C1Parser::CompilationUnitContext *ctx)
{
    auto result = new assembly;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    auto decls = ctx->decl();
    for (auto &decl : decls) {
        auto var_defs = as<ptr_list<var_def_stmt_syntax>>(visit(decl));
        for (auto var_def : var_defs)
            result->global_defs.push_back(var_def);
    }
    auto funcdefs = ctx->funcdef();
    for (auto &funcdef : funcdefs) {
        ptr<func_def_syntax> func;
        func.reset(as<func_def_syntax *>(visit(funcdef)));
        result->global_defs.push_back(func);
    }
    return static_cast<assembly *>(result);
}

antlrcpp::Any syntax_tree_builder::visitDecl(C1Parser::DeclContext *ctx)
{
    if (ctx->constdecl())
        return visit(ctx->constdecl());
    else if (ctx->vardecl())
        return visit(ctx->vardecl());
    return nullptr; // Should never reach here
}

antlrcpp::Any syntax_tree_builder::visitConstdecl(C1Parser::ConstdeclContext *ctx)
{
    ptr_list<var_def_stmt_syntax> result;
    bool isInt = ctx->btype()->Int() != nullptr;
    auto defs = ctx->constdef();
    for (auto &def : defs) {
        ptr<var_def_stmt_syntax> var_def;
        var_def.reset(as<var_def_stmt_syntax *>(visit(def)));
        var_def->is_constant = true;
        var_def->is_int = isInt;
        result.push_back(var_def);
    }
    return static_cast<ptr_list<var_def_stmt_syntax>>(result);
}

antlrcpp::Any syntax_tree_builder::visitConstdef(C1Parser::ConstdefContext *ctx)
{
    auto result = new var_def_stmt_syntax;
    auto expressions = ctx->exp();
    auto comma_num = ctx->Comma().size();
    auto exp_num = expressions.size();
    auto identifier = ctx->Identifier();

    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->is_constant = true;
    result->is_int = true; // Default to int, will be changed at visitConstdecl if it's float
    result->name = std::string(identifier->getSymbol()->getText());
    result->initializers.clear();

    if (ctx->LeftBracket() == nullptr && exp_num == 1) {
        // Identifier Assign exp
        result->array_length = nullptr;
        ptr<expr_syntax> temp;
        temp.reset(as<expr_syntax *>(visit(expressions[0])));
        result->initializers.push_back(temp);
    } else if(exp_num - 2 == comma_num) {
        // Identifier LeftBracket exp RightBracket Assign LeftBrace exp (Comma exp)* RightBrace
        result->array_length.reset(as<expr_syntax *>(visit(expressions[0])));
        for (int i = 1; i < exp_num; i++) {
            ptr<expr_syntax> temp;
            temp.reset(as<expr_syntax *>(visit(expressions[i])));
            result->initializers.push_back(temp);
        }
    } else {
        // Identifier LeftBracket RightBracket Assign LeftBrace exp (Comma exp)* RightBrace
        auto num = new literal_syntax;
        num->line = ctx->getStart()->getLine();
        num->pos = ctx->LeftBracket()->getSymbol()->getCharPositionInLine() + 1;
        num->intConst = exp_num;
        result->array_length.reset(num);
        for (int i = 0; i < exp_num; i++) {
            ptr<expr_syntax> temp;
            temp.reset(as<expr_syntax *>(visit(expressions[i])));
            result->initializers.push_back(temp);
        }
    }

    return static_cast<var_def_stmt_syntax *>(result);
}

antlrcpp::Any syntax_tree_builder::visitVardecl(C1Parser::VardeclContext *ctx)
{
    ptr_list<var_def_stmt_syntax> result;
    bool isInt = ctx->btype()->Int() != nullptr;
    auto defs = ctx->vardef();
    for (auto &def : defs) {
        ptr<var_def_stmt_syntax> var_def;
        var_def.reset(as<var_def_stmt_syntax *>(visit(def)));
        var_def->is_constant = false;
        var_def->is_int = isInt;
        result.push_back(var_def);
    }
    return static_cast<ptr_list<var_def_stmt_syntax>>(result);
}

antlrcpp::Any syntax_tree_builder::visitVardef(C1Parser::VardefContext *ctx)
{
    auto result = new var_def_stmt_syntax;
    auto expressions = ctx->exp();
    auto comma_num = ctx->Comma().size();
    auto exp_num = expressions.size();
    auto identifier = ctx->Identifier();

    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->is_constant = false;
    result->is_int = true; // Default to int, will be changed at visitVardecl if it's float
    result->name = std::string(identifier->getSymbol()->getText());
    result->initializers.clear();

    if (!ctx->LeftBracket()) {
        // Identifier | Identifier Assign exp
        result->array_length = nullptr;
        if (exp_num == 1) {
            // Identifier Assign exp
            ptr<expr_syntax> temp;
            temp.reset(as<expr_syntax *>(visit(expressions[0])));
            result->initializers.push_back(temp);
        }
        // Identifier, no need to do anything
    } else if (!ctx->LeftBrace()) {
        // Identifier LeftBracket exp RightBracket
        result->array_length.reset(as<expr_syntax *>(visit(expressions[0])));
    } else if (exp_num - 2 == comma_num) {
        // Identifier LeftBracket exp RightBracket Assign LeftBrace exp (Comma exp)* RightBrace
        result->array_length.reset(as<expr_syntax *>(visit(expressions[0])));
        for (int i = 1; i < exp_num; i++) {
            ptr<expr_syntax> temp;
            temp.reset(as<expr_syntax *>(visit(expressions[i])));
            result->initializers.push_back(temp);
        }
    } else {
        // Identifier LeftBracket RightBracket Assign LeftBrace exp (Comma exp)* RightBrace
        auto num = new literal_syntax;
        num->line = ctx->getStart()->getLine();
        num->pos = ctx->LeftBracket()->getSymbol()->getCharPositionInLine() + 1;
        num->intConst = exp_num;
        result->array_length.reset(num);
        for (int i = 0; i < exp_num; i++) {
            ptr<expr_syntax> temp;
            temp.reset(as<expr_syntax *>(visit(expressions[i])));
            result->initializers.push_back(temp);
        }
    }

    return static_cast<var_def_stmt_syntax *>(result);
}

antlrcpp::Any syntax_tree_builder::visitFuncdef(C1Parser::FuncdefContext *ctx)
{
    auto result = new func_def_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->name = std::string(ctx->Identifier()->getSymbol()->getText());
    result->body.reset(as<block_syntax *>(visit(ctx->block())));
    return static_cast<func_def_syntax *>(result);
}

antlrcpp::Any syntax_tree_builder::visitBlock(C1Parser::BlockContext *ctx)
{
    auto result = new block_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    auto blockItems = ctx->blockItem();
    for (auto &item : blockItems) {
        if (item->decl() != nullptr) {
            auto stmts = as<ptr_list<var_def_stmt_syntax>>(visit(item->decl()));
            for (auto &stmt : stmts)
                result->body.push_back(stmt);
        } else if (item->stmt() != nullptr) {
            ptr<stmt_syntax> stmt;
            stmt.reset(as<stmt_syntax *>(visit(item->stmt())));
            result->body.push_back(stmt);
        }
    }
    return static_cast<block_syntax *>(result);
}

antlrcpp::Any syntax_tree_builder::visitStmt(C1Parser::StmtContext *ctx)
{
    if (ctx->lval() && ctx->exp()) {
        // lval Assign exp SemiColon
        auto stmt = new assign_stmt_syntax;
        stmt->line = ctx->getStart()->getLine();
        stmt->pos = ctx->getStart()->getCharPositionInLine();
        stmt->target.reset(as<lval_syntax *>(visit(ctx->lval())));
        stmt->value.reset(as<expr_syntax *>(visit(ctx->exp())));
        return static_cast<stmt_syntax *>(stmt);
    }
    if (ctx->Identifier()) {
        // Identifier LeftParen (exp (Comma exp)*)? RightParen SemiColon
        auto stmt = new func_call_stmt_syntax;
        stmt->line = ctx->getStart()->getLine();
        stmt->pos = ctx->getStart()->getCharPositionInLine();
        stmt->name = std::string(ctx->Identifier()->getSymbol()->getText());
        return static_cast<stmt_syntax *>(stmt);
    }
    if (ctx->block()) {
        // block
        return static_cast<stmt_syntax *>(as<block_syntax *>(visit(ctx->block())));
    }
    if (ctx->If() && ctx->cond() && ctx->stmt().size() >= 1) {
        // If LeftParen cond RightParen stmt (Else stmt)?
        auto stmt = new if_stmt_syntax;
        stmt->line = ctx->getStart()->getLine();
        stmt->pos = ctx->getStart()->getCharPositionInLine();
        stmt->pred.reset(as<cond_syntax *>(visit(ctx->cond())));
        auto statements = ctx->stmt();
        stmt->then_body.reset(as<stmt_syntax *>(visit(statements[0])));
        if (ctx->Else() && statements.size() >= 2)
            stmt->else_body.reset(as<stmt_syntax *>(visit(statements[1])));
        else
            stmt->else_body = nullptr;
        return static_cast<stmt_syntax *>(stmt);
    }
    if (ctx->While() && ctx->cond() && ctx->stmt().size() >= 1) {
        // While LeftParen cond RightParen stmt
        auto stmt = new while_stmt_syntax;
        stmt->line = ctx->getStart()->getLine();
        stmt->pos = ctx->getStart()->getCharPositionInLine();
        stmt->pred.reset(as<cond_syntax *>(visit(ctx->cond())));
        stmt->body.reset(as<stmt_syntax *>(visit(ctx->stmt(0))));
        return static_cast<stmt_syntax *>(stmt);
    }
    return nullptr; // Should never reach here
}

antlrcpp::Any syntax_tree_builder::visitLval(C1Parser::LvalContext *ctx)
{
    auto lval = new lval_syntax;
    lval->line = ctx->getStart()->getLine();
    lval->pos = ctx->getStart()->getCharPositionInLine();
    lval->name = std::string(ctx->Identifier()->getSymbol()->getText());
    if (ctx->exp()) {
        // Identifier LeftBracket exp RightBracket
        lval->array_index.reset(as<expr_syntax *>(visit(ctx->exp())));
    } else {
        // Identifier
        lval->array_index = nullptr;
    }
    return static_cast<lval_syntax *>(lval);
}

antlrcpp::Any syntax_tree_builder::visitCond(C1Parser::CondContext *ctx)
{
    auto result = new cond_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    auto expressions = ctx->exp();
    result->lhs.reset(as<expr_syntax *>(visit(expressions[0])));
    auto relop = ctx->relop();
    if (relop->Equal())
        result->op = relop::equal;
    if (relop->NonEqual())
        result->op = relop::non_equal;
    if (relop->Less())
        result->op = relop::less;
    if (relop->LessEqual())
        result->op = relop::less_equal;
    if (relop->Greater())
        result->op = relop::greater;
    if (relop->GreaterEqual())
        result->op = relop::greater_equal;
    result->rhs.reset(as<expr_syntax *>(visit(expressions[1])));
    return static_cast<cond_syntax *>(result);
}

// Returns antlrcpp::Any, which is std::any.
// You should be sure you use the same type for packing and depacking the `Any` object.
// This function always returns an `Any` object containing a `expr_syntax *`.
antlrcpp::Any syntax_tree_builder::visitExp(C1Parser::ExpContext *ctx)
{
    // Get all sub-contexts of type `exp`.
    auto expressions = ctx->exp();
    // Two sub-expressions presented: this indicates it's a expression of binary operator, aka `binop`.
    if (expressions.size() == 2)
    {
        auto result = new binop_expr_syntax;
        // Set line and pos.
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        // visit(some context) is equivalent to calling corresponding visit method; dispatching is done automatically
        // by ANTLR4 runtime. For this case, it's equivalent to visitExp(expressions[0]).
        // Use reset to set a new pointer to a std::shared_ptr object. DO NOT use assignment; it won't work.
        // Use `as<Type>(std::any)' to get value from antlrcpp::Any object; notice that this Type must match the type used in
        // constructing the Any object, which is constructed from (usually pointer to some derived class of
        // syntax_node, in this case) returning value of the visit call.
        result->lhs.reset(as<expr_syntax *>(visit(expressions[0])));
        // Check if each token exists.
        // Returnd value of the calling will be nullptr (aka NULL in C) if it isn't there; otherwise non-null pointer.
        if (ctx->Plus())
            result->op = binop::plus;
        if (ctx->Minus())
            result->op = binop::minus;
        if (ctx->Multiply())
            result->op = binop::multiply;
        if (ctx->Divide())
            result->op = binop::divide;
        if (ctx->Modulo())
            result->op = binop::modulo;
        result->rhs.reset(as<expr_syntax *>(visit(expressions[1])));
        return static_cast<expr_syntax *>(result);
    }
    // Otherwise, if `+` or `-` presented, it'll be a `unaryop_expr_syntax`.
    if (ctx->Plus() || ctx->Minus())
    {
        auto result = new unaryop_expr_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        if (ctx->Plus())
            result->op = unaryop::plus;
        if (ctx->Minus())
            result->op = unaryop::minus;
        result->rhs.reset(as<expr_syntax *>(visit(expressions[0])));
        return static_cast<expr_syntax *>(result);
    }
    // In the case that `(` exists as a child, this is an expression like `'(' expressions[0] ')'`.
    if (ctx->LeftParen())
        return visit(expressions[0]); // Any already holds expr_syntax* here, no need for dispatch and re-patch with casting.
    // If `number` exists as a child, we can say it's a literal integer expression.
    if (auto number = ctx->number())
        return visit(number);
    // If `lval` exists as a child, it's a left-value expression.
    if (auto lval = ctx->lval()) {
        return static_cast<expr_syntax *>(as<lval_syntax*>(visit(lval)));
    }
    return nullptr; // Should never reach here
}

antlrcpp::Any syntax_tree_builder::visitNumber(C1Parser::NumberContext *ctx)
{
    auto result = new literal_syntax;
    if (auto intConst = ctx->IntConst())
    {
        result->is_int = true;
        result->line = intConst->getSymbol()->getLine();
        result->pos = intConst->getSymbol()->getCharPositionInLine();
        auto text = intConst->getSymbol()->getText();
        if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) // Hexadecimal
            result->intConst = std::stoi(text, nullptr, 16); // std::stoi will eat '0x'
        /* you need to add other situations here */
        else if (text[0] == '0') // Octal
            result->intConst = std::stoi(text, nullptr, 8); // std::stoi will eat '0'
        else // Decimal
            result->intConst = std::stoi(text);
        return static_cast<expr_syntax *>(result);
    }
    // else FloatConst
    else
    {
        result->is_int = false;
        auto floatConst = ctx->FloatConst();
        result->line = floatConst->getSymbol()->getLine();
        result->pos = floatConst->getSymbol()->getCharPositionInLine();
        // result->floatConst = std::stof(floatConst->getSymbol()->getText());
        std::string text = floatConst->getSymbol()->getText();
        std::istringstream iss(text);
        iss >> result->floatConst;
        return static_cast<expr_syntax *>(result);
    }
    return nullptr; // Should never reach here
}

ptr<syntax_tree_node> syntax_tree_builder::operator()(antlr4::tree::ParseTree *ctx)
{
    auto result = visit(ctx);
    if (is<syntax_tree_node *>(result))
        return ptr<syntax_tree_node>(as<syntax_tree_node *>(result));
    if (is<assembly *>(result))
        return ptr<syntax_tree_node>(as<assembly *>(result));
    if (is<global_def_syntax *>(result))
        return ptr<syntax_tree_node>(as<global_def_syntax *>(result));
    if (is<func_def_syntax *>(result))
        return ptr<syntax_tree_node>(as<func_def_syntax *>(result));
    if (is<cond_syntax *>(result))
        return ptr<syntax_tree_node>(as<cond_syntax *>(result));
    if (is<expr_syntax *>(result))
        return ptr<syntax_tree_node>(as<expr_syntax *>(result));
    if (is<binop_expr_syntax *>(result))
        return ptr<syntax_tree_node>(as<binop_expr_syntax *>(result));
    if (is<unaryop_expr_syntax *>(result))
        return ptr<syntax_tree_node>(as<unaryop_expr_syntax *>(result));
    if (is<lval_syntax *>(result))
        return ptr<syntax_tree_node>(as<lval_syntax *>(result));
    if (is<literal_syntax *>(result))
        return ptr<syntax_tree_node>(as<literal_syntax *>(result));
    if (is<stmt_syntax *>(result))
        return ptr<syntax_tree_node>(as<stmt_syntax *>(result));
    if (is<var_def_stmt_syntax *>(result))
        return ptr<syntax_tree_node>(as<var_def_stmt_syntax *>(result));
    if (is<assign_stmt_syntax *>(result))
        return ptr<syntax_tree_node>(as<assign_stmt_syntax *>(result));
    if (is<func_call_stmt_syntax *>(result))
        return ptr<syntax_tree_node>(as<func_call_stmt_syntax *>(result));
    if (is<block_syntax *>(result))
        return ptr<syntax_tree_node>(as<block_syntax *>(result));
    if (is<if_stmt_syntax *>(result))
        return ptr<syntax_tree_node>(as<if_stmt_syntax *>(result));
    if (is<while_stmt_syntax *>(result))
        return ptr<syntax_tree_node>(as<while_stmt_syntax *>(result));
    return nullptr; // Should never reach here
}
