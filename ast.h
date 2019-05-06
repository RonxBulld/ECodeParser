#include <utility>

#include <utility>

#include <utility>

//
// Created by 曹顺 on 2019/5/5.
//

#ifndef PARSE_E_FILE_AST_H
#define PARSE_E_FILE_AST_H

#include <vector>
#include <memory>
#include "visitor.h"
#include "FileBuffer.h"

#define AST_DECL() void accept(Visitor *visitor) override {visitor->enter(this);visitor->visit(this);visitor->leave(this);}
#define AST_NODE(ast) struct AST##ast;using AST##ast##Ptr = std::shared_ptr<AST##ast>;struct AST##ast : public ASTNode

enum KeyType : char {
    KeyType_None = 0,
    KeyType_Sub = 4,
    KeyType_Global = 5,
    KeyType_Program = 9,
    KeyType_DllFunc = 10,
    KeyType_ProgramVar = 21,
    KeyType_Control = 22,
    KeyType_Constant = 24,
    KeyType_WindowProgram = 25,
    KeyType_LocalOrParam = 37,
    KeyType_ImageRes = 40,
    KeyType_DataStructureMember = 53,
    KeyType_SoundRes = 56,
    KeyType_DataStruct = 65,
    KeyType_DllFunParam = 69,
    KeyType_Module = 73,
    KeyType_WindowRes = 82,
};

struct Key {
    union {
        struct {
            short index;
            char code; // 不知道这个是干啥的
            KeyType type;
        };
        int value;
        char part[4];
    };

    Key(int value) : value(value) {}
    Key() : value(0) {}

};

struct Value {
    int type{0};
    union {
        int val_int{};
        double val_double;
        bool val_bool;
        long long val_time;
        FixedData val_string;
    };
    explicit Value() : type(0), val_int(0) {}
    explicit Value(int value) : type(1), val_int(0) {}
    explicit Value(bool value) : type(2), val_bool(value) {}
    explicit Value(FixedData value) : type(3), val_string(value) {
        val_string.length--;
    }
    explicit Value(long long value) : type(4), val_time(value) {}
    explicit Value(double value) : type(5), val_double(value) {}

};

struct ASTNode {
    ASTNode() = default;
    virtual void accept(Visitor *visitor) {
        visitor->visit(this);
    };
};
using ASTNodePtr = std::shared_ptr<ASTNode>;

AST_NODE(Args) {
    AST_DECL();
    std::vector<ASTNodePtr> args;
    inline void AddArg(const ASTNodePtr &node) {
        args.push_back(node);
    }
};

AST_NODE(Block) {
    AST_DECL();
    std::vector<ASTNodePtr> element;
    inline void AddStmt(const ASTNodePtr &node) {
        element.push_back(node);
    }
};

AST_NODE(FunCall) {
    Key key;
    short lib;
    short unknown;
    FixedData object;
    FixedData comment;
    ASTArgsPtr args;
    AST_DECL();
};

AST_NODE(Program) {
    AST_DECL();
    std::vector<ASTNodePtr> stmts;
    inline void AddStmt(const ASTNodePtr &node) {
        stmts.push_back(node);
    }
};

AST_NODE(IfStmt) {
    AST_DECL();
    ASTNodePtr condition;
    ASTBlockPtr then_block;
    ASTBlockPtr else_block;
};

AST_NODE(Literal) {
    AST_DECL();
    Value value;

    explicit ASTLiteral(const Value &value) : value(value) {}
};

AST_NODE(Constant) {
    AST_DECL();
    Key key;

    explicit ASTConstant(const int &key) : key(key) {}
};

AST_NODE(LibConstant) {
    AST_DECL();
    short index;
    short member;

    explicit ASTLibConstant(int code) : index((short) (code & 0xff - 1)), member((short) ((code >> 16) - 1)) {}
};

AST_NODE(Address) {
    AST_DECL();
    Key key;

    explicit ASTAddress(const int &key) : key(key) {}
};

AST_NODE(Subscript) {
    AST_DECL();
    ASTNodePtr value;

    explicit ASTSubscript(ASTNodePtr value) : value(std::move(value)) {}
};

AST_NODE(EnumConstant) {
    AST_DECL();
    int key;
    int index;
    ASTEnumConstant(int key, int index) : key(key), index(index) {}
};

AST_NODE(StructMember) {
    AST_DECL();
    int member;
    int key;
    ASTStructMember(int member, int key) : member(member), key(key) {}
};

AST_NODE(Variable) {
    AST_DECL();
    Key key;

    explicit ASTVariable(Key key) : key(key) {}
};

AST_NODE(Dot) {
    AST_DECL();
    ASTNodePtr var;
    ASTNodePtr field;
    ASTDot(ASTNodePtr var, ASTNodePtr field) : var(std::move(var)), field(std::move(field)) {}
};

AST_NODE(Judge) {
    AST_DECL();
    std::vector<ASTNodePtr> conditions;
    std::vector<ASTNodePtr>  blocks;
    ASTBlockPtr default_block;
};

AST_NODE(Loop) {
    AST_DECL();
    ASTFunCallPtr head;
    ASTBlockPtr block;
    ASTFunCallPtr tail;
};


#endif //PARSE_E_FILE_AST_H
