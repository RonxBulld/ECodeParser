//
// Created by 曹顺 on 2019/5/5.
//

#ifndef PARSE_E_FILE_VISITOR_H
#define PARSE_E_FILE_VISITOR_H

struct ASTNode;
struct ASTFunCall;
struct ASTProgram;
struct ASTList;
struct ASTIfStmt;
struct ASTLiteral;
struct ASTConstant;
struct ASTLibConstant;
struct ASTAddress;
struct ASTSubscript;
struct ASTEnumConstant;
struct ASTStructMember;
struct ASTVariable;
struct ASTDot;
struct ASTJudge;

struct Visitor {
    virtual void enter(ASTNode *node) {}
    virtual void leave(ASTNode *node) {}
    virtual void visit(ASTNode *node) {};
    virtual void visit(ASTFunCall *node) {};
    virtual void visit(ASTProgram *node) {};
    virtual void visit(ASTList *node) {};
    virtual void visit(ASTIfStmt *node) {};
    virtual void visit(ASTLiteral *node) {};
    virtual void visit(ASTConstant *node) {};
    virtual void visit(ASTLibConstant *node) {};
    virtual void visit(ASTAddress *node) {};
    virtual void visit(ASTSubscript *node) {};
    virtual void visit(ASTEnumConstant *node) {};
    virtual void visit(ASTStructMember *node) {};
    virtual void visit(ASTVariable *node) {};
    virtual void visit(ASTDot *node) {};
    virtual void visit(ASTJudge *node) {};

};


#endif //PARSE_E_FILE_VISITOR_H
