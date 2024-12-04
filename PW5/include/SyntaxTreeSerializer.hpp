
#ifndef _SYSYF_TEST_SYNTAX_TREE_SERIALIZER_
#define _SYSYF_TEST_SYNTAX_TREE_SERIALIZER_

#include <SyntaxTree.h>
#include <sstream>

namespace SyntaxTree
{

  std::map<Type, std::string> typeToStr = {
      {Type::INT, "int"},
      {Type::VOID, "void"},
      {Type::FLOAT, "float"}};

  std::map<BinaryCondOp, std::string> bincondOpToStr = {
      {BinaryCondOp::EQ, "=="},
      {BinaryCondOp::NEQ, "!="},
      {BinaryCondOp::GT, ">"},
      {BinaryCondOp::GTE, ">="},
      {BinaryCondOp::LT, "<"},
      {BinaryCondOp::LTE, "<="},
      {BinaryCondOp::LAND, "&&"},
      {BinaryCondOp::LOR, "||"}};

  std::map<BinOp, std::string> binOpToStr = {
      {BinOp::PLUS, "+"},
      {BinOp::MINUS, "-"},
      {BinOp::MULTIPLY, "*"},
      {BinOp::DIVIDE, "/"},
      {BinOp::MODULO, "%"}};

  std::map<UnaryCondOp, std::string> unarycondOpToStr = {
      {UnaryCondOp::NOT, "!"}};

  std::map<UnaryOp, std::string> unaryOpToStr = {
      {UnaryOp::PLUS, "+"},
      {UnaryOp::MINUS, "-"}};

  std::string locationToString(const yy::location &loc)
  {
    std::stringstream ss;
    ss << loc << ": ";
    ss << "line " << loc.begin.line << ", column " << loc.begin.column;
    return ss.str();
  }

  template <typename Writer>
  class SyntaxTreeSerializer : public Visitor
  {
  public:
    SyntaxTreeSerializer(Writer &w) : writer(w) {}

    void serialize(Node *tree)
    {
      tree->accept(*this);
    }

    virtual void visit(Assembly &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("Assembly");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("global defs");
      writer.StartArray();
      for (auto def : node.global_defs)
        def->accept(*this);
      writer.EndArray();
      writer.EndObject();
    }

    virtual void visit(FuncDef &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("FuncDef");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("return type");
      writer.String(typeToStr[node.ret_type].c_str());
      writer.Key("name");
      writer.String(node.name.c_str());
      writer.Key("param list");
      node.param_list->accept(*this);
      writer.Key("body");
      node.body->accept(*this);
      writer.EndObject();
    }
    virtual void visit(BinaryExpr &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("BinaryExpr");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("lhs");
      node.lhs->accept(*this);
      writer.Key("op");
      writer.String(binOpToStr[node.op].c_str());
      writer.Key("rhs");
      node.rhs->accept(*this);
      writer.EndObject();
    }
    virtual void visit(UnaryExpr &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("UnaryExpr");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("op");
      writer.String(unaryOpToStr[node.op].c_str());
      writer.Key("rhs");
      node.rhs->accept(*this);
      writer.EndObject();
    }
    virtual void visit(LVal &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("LVal");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("name");
      writer.String(node.name.c_str());
      if (node.array_index.size() != 0)
      {
        writer.Key("array index");
        writer.StartArray();
        for (auto def : node.array_index)
          def->accept(*this);
        writer.EndArray();
      }
      writer.EndObject();
    }
    virtual void visit(Literal &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("Literal");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("value");
      if (node.literal_type == Type::INT)
      {
        writer.Int(node.int_const);
        writer.Key("is int");
        writer.String("true");
      }
      else if (node.literal_type == Type::FLOAT)
      {
        writer.Double(node.float_const);
        writer.Key("is int");
        writer.String("false");
      }
      writer.EndObject();
    }
    virtual void visit(ReturnStmt &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("ReturnStmt");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      if (node.ret.get())
      {
        writer.Key("value");
        node.ret->accept(*this);
      }
      writer.EndObject();
    }
    virtual void visit(VarDef &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("VarDef");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("name");
      writer.String(node.name.c_str());
      writer.Key("type");
      writer.String(typeToStr[node.btype].c_str());
      writer.Key("is const");
      if (node.is_constant)
      {
        writer.String("true");
      }
      else
      {
        writer.String("false");
      }
      if (node.array_length.size() != 0)
      {
        writer.Key("array length");
        writer.StartArray();
        for (auto length : node.array_length)
        {
          length->accept(*this);
        }
        writer.EndArray();
      }
      if (node.is_inited)
      {
        writer.Key("initializers");
        node.initializers->accept(*this);
      }
      writer.EndObject();
    }
    virtual void visit(AssignStmt &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("AssignStmt");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("target");
      node.target->accept(*this);
      writer.Key("value");
      node.value->accept(*this);
      writer.EndObject();
    }
    virtual void visit(FuncCallStmt &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("FuncCallStmt");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("name");
      writer.String(node.name.c_str());
      writer.Key("param");
      writer.StartArray();
      for (auto exp : node.params)
      {
        exp->accept(*this);
      }
      writer.EndArray();
      writer.EndObject();
    }
    virtual void visit(BlockStmt &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("BlockStmt");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("body");
      writer.StartArray();
      for (auto stmt : node.body)
      {
        stmt->accept(*this);
      }
      writer.EndArray();
      writer.EndObject();
    }
    virtual void visit(EmptyStmt &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("EmptyStmt");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.EndObject();
    }
    virtual void visit(ExprStmt &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("ExprStmt");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("expr");
      node.exp->accept(*this);
      writer.EndObject();
    }
    virtual void visit(FuncParam &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("FuncParam");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("name");
      writer.String(node.name.c_str());
      writer.Key("type");
      writer.String(typeToStr[node.param_type].c_str());
      if (node.array_index.size() != 0)
      {
        writer.Key("array index");
        writer.StartArray();
        for (auto exp : node.array_index)
        {
          if (exp != nullptr)
            exp->accept(*this);
        }
        writer.EndArray();
      }
      writer.EndObject();
    }
    virtual void visit(FuncFParamList &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("FuncFParamList");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      if (node.params.size() != 0)
      {
        writer.Key("params");
        writer.StartArray();
        for (auto exp : node.params)
        {
          exp->accept(*this);
        }
        writer.EndArray();
      }
      writer.EndObject();
    }
    virtual void visit(IfStmt &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("IfStmt");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("condition expr");
      node.cond_exp->accept(*this);
      writer.Key("then body");
      if (dynamic_cast<BlockStmt *>(node.if_statement.get()))
      {
        node.if_statement->accept(*this);
      }
      else
      {
        node.if_statement->accept(*this);
      }
      if (node.else_statement != nullptr)
      {
        writer.Key("else body");
        if (dynamic_cast<BlockStmt *>(node.else_statement.get()))
        {
          node.else_statement->accept(*this);
        }
        else
        {
          node.else_statement->accept(*this);
        }
      }
      writer.EndObject();
    }
    virtual void visit(WhileStmt &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("WhileStmt");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("condition expr");
      node.cond_exp->accept(*this);
      writer.Key("body");
      if (dynamic_cast<BlockStmt *>(node.statement.get()))
      {
        node.statement->accept(*this);
      }
      else
      {
        node.statement->accept(*this);
      }
      writer.EndObject();
    }
    virtual void visit(BreakStmt &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("BreakStmt");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.EndObject();
    }
    virtual void visit(ContinueStmt &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("ContinueStmt");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.EndObject();
    }
    virtual void visit(UnaryCondExpr &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("UnaryCondExpr");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("op");
      writer.String(unarycondOpToStr[node.op].c_str());
      writer.Key("rhs");
      node.rhs->accept(*this);
      writer.EndObject();
    }
    virtual void visit(BinaryCondExpr &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("BinaryCondExpr");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      writer.Key("lhs");
      node.lhs->accept(*this);
      writer.Key("op");
      writer.String(bincondOpToStr[node.op].c_str());
      writer.Key("rhs");
      node.rhs->accept(*this);
      writer.EndObject();
    }
    virtual void visit(InitVal &node) override
    {
      writer.StartObject();
      writer.Key("type");
      writer.String("InitVal");
      writer.Key("location");
      writer.String(locationToString(node.loc).c_str());
      if (node.isExp)
      {
        writer.Key("expr");
        node.expr->accept(*this);
      }
      else
      {
        writer.Key("elementList");
        writer.StartArray();
        for (auto item : node.elementList)
        {
          item->accept(*this);
        }
        writer.EndArray();
      }
      writer.EndObject();
    }

  private:
    Writer &writer;
  };
}
#endif
