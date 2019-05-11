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

void DumpVisitor::visit(ASTArgs *node) {
    cout << "(";
    for (auto &arg : node->args) {
        if (arg != nullptr) {
            arg->accept(this);
            if (arg != node->args.back()) {
                cout << ", ";
            }
        }
    }
    cout << ")";
}

void DumpVisitor::visit(ASTBlock *node) {
    indent += 4;
    for (auto &arg : node->element) {
        if (arg != nullptr) {
            print_indent();
            arg->accept(this);
            cout << endl;
        }
    }
    indent -= 4;
}

void DumpVisitor::visit(ASTFunCall *node) {
    if (node->key.value == 0) {
        // 空行
        return;
    }
    if (node->lib >= 0) {
        cout << code->libraries[node->lib].info->m_pBeginCmdInfo[node->key.value].m_szName;
        node->args->accept(this);
        return;
    }
    if (node->key.type == KeyType_Sub) {
        for (int i = 0; i < code->subNumber; ++i) {
            if (code->subs[i].key.value == node->key.value) {
                cout << code->subs[i].name;
            }
        }
    } else if (node->key.type == KeyType_DllFunc) {
        for (int i = 0; i < code->dllNumber; ++i) {
            if (code->dlls[i].key.index == node->key.index) {
                cout << code->dlls[i].name;
            }
        }
    }
    node->args->accept(this);

}

void DumpVisitor::visit(ASTIfStmt *node) {
    print_indent();
    cout << "if (";
    node->condition->accept(this);
    cout << ")" << endl;
    node->then_block->accept(this);

    if (node->else_block != nullptr) {
        print_indent();
        cout << "else" << endl;
        node->else_block->accept(this);
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
    for (int i = 0; i < code->constantNumber; ++i) {
        if (code->constants[i].key.value == node->key.value) {
            cout << "#" << code->constants[i].name;
        }
    }
}

void DumpVisitor::visit(ASTLibConstant *node) {
    cout << "#" << code->libraries[node->index].info->m_pLibConst[node->member].m_szName;
}

void DumpVisitor::visit(ASTAddress *node) {
    for (int i = 0; i < code->subNumber; ++i) {
        if (code->subs[i].key.value == node->key.value) {
            cout << "&" << code->subs[i].name;
        }
    }
}

void DumpVisitor::visit(ASTSubscript *node) {
    cout << "[";
    node->value->accept(this);
    cout << "]";
}

void DumpVisitor::visit(ASTEnumConstant *node) {

}

void DumpVisitor::visit(ASTStructMember *node) {
    for (int i = 0; i < code->structNumber; ++i) {
        if (code->structs[i].key.value == node->key.value) {
            for (int j = 0; j < code->structs[i].memberNumber; ++j) {
                if (code->structs[i].members[j].key.value == node->member.value) {
                    cout << "." << code->structs[i].members[j].name;
                    break;
                }
            }
            break;
        }
    }

}

void DumpVisitor::visit(ASTVariable *node) {
    if (node->key.type == KeyType_LocalOrParam) {
        for (int i = 0; i < current->localNumber; ++i) {
            if (current->locals[i].key.value == node->key.value) {
                cout << current->locals[i].name;
                return;
            }
        }
        for (int i = 0; i < current->paramNumber; ++i) {
            if (current->params[i].key.value == node->key.value) {
                cout << current->params[i].name;
                return;
            }
        }

    } else if (node->key.type == KeyType_ProgramVar) {
        EModule *module = current->module;
        for (int i = 0; i < module->varNumber; ++i) {
            if (module->vars[i].key.value == node->key.value) {
                cout << module->vars[i].name;
                return;
            }
        }
    }

}

void DumpVisitor::visit(ASTDot *node) {
    node->var->accept(this);
    node->field->accept(this);
}

void DumpVisitor::visit(ASTJudge *node) {
    for (int i = 0; i < node->conditions.size(); ++i) {
        print_indent();
        cout << "case ";
        node->conditions[i]->accept(this);
        cout << " : " << endl;
        node->blocks[i]->accept(this);
    }
    if (node->default_block != nullptr) {
        print_indent();
        cout << "default : " << endl;
        node->default_block->accept(this);
    }
}

void DumpVisitor::visit(ASTLoop *node) {
    print_indent();
    node->head->accept(this);
    cout << endl;
    node->block->accept(this);
    if (node->tail != nullptr) {
        print_indent();
        node->tail->accept(this);
    }
}

void DumpVisitor::print_indent() {
    for (int i = 0; i < indent; ++i) {
        cout << " ";
    }
}

void DumpVisitor::visit(ASTBrace *node) {
    cout << "{" ;
    node->args->accept(this);
    cout << "}";

}

