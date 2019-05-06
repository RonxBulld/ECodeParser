//
// Created by 曹顺 on 2019/5/5.
//

#ifndef PARSE_E_FILE_TEST_H
#define PARSE_E_FILE_TEST_H

#include "visitor.h"
#include "ast.h"
#include <iostream>
#include <map>
struct ECode;
struct Sub;
using namespace std;
struct DumpVisitor : Visitor {
    // 当前子程序
    Sub *current;
    ECode *code;
    explicit DumpVisitor(ECode *code);

    void visit(ASTProgram *node) override;
    void visit(ASTList *node) override;
    void visit(ASTFunCall *node) override;
    void visit(ASTIfStmt *node) override;
    void visit(ASTLiteral *node) override;
    void visit(ASTConstant *node) override;
    void visit(ASTLibConstant *node) override;
    void visit(ASTAddress *node) override;
    void visit(ASTSubscript *node) override;
    void visit(ASTEnumConstant *node) override;
    void visit(ASTStructMember *node) override;
    void visit(ASTVariable *node) override;
    void visit(ASTDot *node) override;

};


#endif //PARSE_E_FILE_TEST_H
