//
// Created by 曹顺 on 2019/5/31.
//
#ifndef PARSE_E_FILE_TRANSLATOR_H
#define PARSE_E_FILE_TRANSLATOR_H

#include <ast.h>
#include <visitor.h>
#include "ECodeParser.h"

class Translator : Visitor {
private:
    ESub *current{};
    ECode *code{};
public:
    explicit Translator(ECode *code) : code(code) {}
    void enter_sub(ESub *enter) {
        current = enter;
    };
    EConst *find_const(Key key) {
        if (key.type != KeyType_Constant) {
            return nullptr;
        }
        for (int i = 0; i < code->constantNumber; ++i) {
            if (code->constants[i].key.value == key.value) {
                return &code->constants[i];
            }
        }
        return nullptr;
    }
    ESub *find_sub(Key key) {
        for (int i = 0; i < code->subNumber; ++i) {
            if (code->subs[i].key.value == key.value) {
                return &code->subs[i];
            }
        }
        return nullptr;
    }
    EStruct *find_struct(Key key) {
        for (int i = 0; i < code->structNumber; ++i) {
            if (code->structs[i].key.value == key.value) {
                return &code->structs[i];
            }
        }
        return nullptr;
    }

    /**
     * 查找局部变量或者参数
     * @param key
     * @return
     */
    EVar *find_local(Key key) {
        for (int i = 0; i < current->localNumber; ++i) {
            if (current->locals[i].key.value == key.value) {
                return &current->locals[i];
            }
        }
        for (int i = 0; i < current->paramNumber; ++i) {
            if (current->params[i].key.value == key.value) {
                return &current->params[i];
            }
        }
        return nullptr;
    }

    /**
     * 查找程序集变量
     * @param key
     * @return
     */
    EVar *find_program_var(Key key) {
        EModule *module = current->module;
        for (int i = 0; i < module->varNumber; ++i) {
            if (module->vars[i].key.value == key.value) {
                return &module->vars[i];
            }
        }
        return nullptr;
    }
    PLIB_CONST_INFO find_lib_const(int lib_index, int member_index) {
        if (code->libraryNumber >= lib_index) {
            return nullptr;
        }
        return &code->libraries[lib_index].info->m_pLibConst[member_index];
    }

private:
    void visit(ASTProgram *node) override {
        for (auto &stmt : node->stmts) {
            if (stmt != nullptr) {
                stmt->accept(this);
            }
        }
    }
};


#endif //PARSE_E_FILE_TRANSLATOR_H
