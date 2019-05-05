//
// Created by 曹顺 on 2019/5/5.
//

#include "test.h"
#include "ast.h"

void DumpVisitor::visit(ASTProgram *node) {
    for (auto &stmt : node->stmts) {
        stmt->accept(this);
    }
}

void DumpVisitor::visit(ASTList *node) {
    for (auto &arg : node->args) {
        arg->accept(this);
    }
}

void DumpVisitor::visit(ASTFunCall *node) {
    cout << "funcall : " << node->key.index<< endl;
}

void DumpVisitor::visit(ASTIfStmt *node) {
}

void DumpVisitor::visit(ASTLiteral *node) {
}

void DumpVisitor::visit(ASTConstant *node) {
}

void DumpVisitor::visit(ASTLibConstant *node) {
}

void DumpVisitor::visit(ASTAddress *node) {
}

void DumpVisitor::visit(ASTSubscript *node) {
}

void DumpVisitor::visit(ASTEnumConstant *node) {
}

void DumpVisitor::visit(ASTStructMember *node) {
}

void DumpVisitor::visit(ASTVariable *node) {
}

void DumpVisitor::visit(ASTDot *node) {
}
