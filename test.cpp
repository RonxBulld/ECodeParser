//
// Created by 曹顺 on 2019/5/5.
//

#include "test.h"
#include "ast.h"
#include "ECodeParser.h"

DumpVisitor::DumpVisitor(ECode *code) : code(code) {}

void DumpVisitor::visit(ASTProgram *node) {
    for (auto &stmt : node->stmts) {
        if (stmt != nullptr) {
            stmt->accept(this);
            cout << endl;
        }
    }
}

void DumpVisitor::visit(ASTList *node) {
    for (auto &arg : node->args) {
        if (arg != nullptr) {
            arg->accept(this);
        }
    }
}

void DumpVisitor::visit(ASTFunCall *node) {
    if (node->key.value == 0) {
        // 空行
        return;
    }
    if (node->lib >= 0) {
        cout << code->libraries[node->lib].info->m_pBeginCmdInfo[node->key.value].m_szName;
    } else {

        for (int i = 0; i < code->subNumber; ++i) {
            if (code->subs[i].key.value == node->key.value) {
                cout << code->subs[i].name;
            }
        }
    }
    cout << node->args->args.size();

    cout << "(";
    for (auto &arg : node->args->args) {
        if (arg != nullptr) {
            arg->accept(this);
            if (arg != node->args->args.back()) {
                cout << ", ";
            }

        }
    }
    cout << ")";

}

void DumpVisitor::visit(ASTIfStmt *node) {
    cout << "if (";
    node->condition->accept(this);
    cout << " )" << endl;
    for (auto &arg : node->then_block->args) {
        if (arg != nullptr) {
            cout << "    ";
            arg->accept(this);
        }
    }
}

void DumpVisitor::visit(ASTLiteral *node) {
    switch (node->value.type) {
        case 0:
            cout << "null";
            break;
        case 1:
            cout << node->value.val_int;
            break;
        case 2:
            cout << node->value.val_bool;
            break;
        case 3:
            cout << "\"" << node->value.val_string << "\"";
            break;
        case 4:
            cout << node->value.val_time;
            break;
        case 5:
            cout << node->value.val_double;
            break;
        default:
            break;
    }
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
    if (node->key.type == KeyType_LocalOrParam) {
        for (int i = 0; i < current->localNumber; ++i) {
            if (current->locals[i].key.value == node->key.value) {
                cout << current->locals[i].name;
            }
        }
        for (int i = 0; i < current->paramNumber; ++i) {
            if (current->params[i].key.value == node->key.value) {
                cout << current->params[i].name;
            }
        }

    }

}

void DumpVisitor::visit(ASTDot *node) {
}

