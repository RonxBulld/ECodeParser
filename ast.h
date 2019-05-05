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

struct ASTNode {
    ASTNode() = default;
    virtual void accept(Visitor *visitor) {
        visitor->visit(this);
    };
};
using ASTNodePtr = std::shared_ptr<ASTNode>;

enum KeyType : char {
    Type_None = 0,
    Type_Sub = 4,
    Type_Global = 5,
    Type_Program = 9,
    Type_DllFunc = 10,
    Type_ProgramVar = 21,
    Type_Control = 22,
    Type_Constant = 24,
    Type_WindowProgram = 25,
    Type_LocalVariable = 37,
    Type_ImageRes = 40,
    Type_DataStructureMember = 53,
    Type_SoundRes = 56,
    Type_DataStruct = 65,
    Type_DllFunParam = 69,
    Type_Module = 73,
    Type_WindowRes = 82,
};

struct Key {
    short index;
    char code; // 不知道这个是干啥的
    KeyType type;
    inline int hash() {
        return index | (code >> 16) | (type >> 24);
    }
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
    explicit Value(FixedData value) : type(3), val_string(value) {}
    explicit Value(long long value) : type(4), val_time(value) {}
    explicit Value(double value) : type(5), val_double(value) {}

};

AST_NODE(List) {
    AST_DECL();
    std::vector<ASTNodePtr> args;
    inline void AddArg(ASTNodePtr &&node) {
        args.push_back(node);
    }
};

AST_NODE(FunCall) {
    Key key;
    short lib;
    short unkown;
    FixedData object;
    FixedData comment;
    ASTListPtr args;
    AST_DECL();
};

AST_NODE(Program) {
    AST_DECL();
    std::vector<ASTNodePtr> stmts;
    inline void AddStmt(ASTNodePtr node) {
        stmts.push_back(node);
    }
};

AST_NODE(IfStmt) {
    AST_DECL();
    ASTNodePtr condition;
    ASTListPtr then_block;
    ASTListPtr else_block;
};

AST_NODE(Literal) {
    AST_DECL();
    Value value;
    ASTLiteral(const Value &value) : value(value) {}
};

AST_NODE(Constant) {
    AST_DECL();
    int key;
    ASTConstant(const int &key) : key(key) {}
};

AST_NODE(LibConstant) {
    AST_DECL();
    int code;
    ASTLibConstant(int code) : code(code) {}
};

AST_NODE(Address) {
    AST_DECL();
    int key;
    ASTAddress(const int &key) : key(key) {}
};

AST_NODE(Subscript) {
    AST_DECL();
    ASTNodePtr value;
    ASTSubscript(const ASTNodePtr &value) : value(value) {}
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
    int key;
    ASTVariable(int key) : key(key) {}
};

AST_NODE(Dot) {
    AST_DECL();
    ASTNodePtr var;
    ASTNodePtr field;
    ASTDot(const ASTNodePtr &var, const ASTNodePtr &field) : var(var), field(field) {}
};

AST_NODE(Judge) {
    AST_DECL();
    std::vector<ASTNodePtr> conditions;
    std::vector<ASTNodePtr>  blocks;
    ASTNodePtr default_block;
};

AST_NODE(Loop) {
    AST_DECL();
    ASTFunCallPtr head;
    ASTListPtr block;
    ASTFunCallPtr tail;
};


#endif //PARSE_E_FILE_AST_H
