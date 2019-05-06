//
// Created by ??? on 2019/5/3.
//

#ifndef PARSE_E_FILE_ECODEPARSER_H
#define PARSE_E_FILE_ECODEPARSER_H
#define assert(condition, str) \
    do { \
    if(!(condition)) \
        std::cout << str;\
    } while(0);
#include <vector>
#include <memory>
#include <map>
#include "FileBuffer.h"
#include "tools.h"
#include "ast.h"
#include "lib2.h"
#include "test.h"
#define NOT_REACHED() (void(0))
#define make_ptr(p, ...) std::make_shared<p>(__VA_ARGS__)
using namespace std;

ostream &operator<<(ostream &os, Key &key);
ostream &operator<<(ostream &os, FixedData &data);

struct BasicInfo {
    int type{0};
    FixedData name;
    FixedData description;
    FixedData author;
    FixedData code;
    FixedData address;
    FixedData telephone;
    FixedData fox;
    FixedData email;
    FixedData host;
    FixedData copyright;
    int version[2]{0, 0};
    int create[2]{0, 0};
};

struct Window {
    Key key;
    Key belong;
    FixedData name;
    FixedData comment;
    int left{};
    int top{};
    int width{};
    int height{};
    FixedData cursor;
    FixedData mark;
    int visible{};
    int bidden{};
    int border{};
    int bgSize{};
    int bgColor{};
    int maxBtn{};
    int minBtn{};
    int ctrlBtn{};
    int position{};
    int movable{};
    int musicTimes{};
    int enterFocus{};
    int escClose{};
    int f1Help{};
    int helpMark{};
    int showInTaskbar{};
    int mov{}; // 随意移动
    int shape{};
    int alwaysTop{};
    int alwaysActive{};
    FixedData className;
    FixedData title;
    FixedData helpFileName;
    int number{0};
    Window() = default;
};

struct Constant;

struct LibConstant {
    int lib; // lib index
    int index; // constant index
};


struct Constant {
    Key key;
    int property{};
    FixedData name;
    FixedData comment;

    union {
        FixedData data;
        Value value;
    };

};

struct Variable {
    Key key;
    short type{};
    short property{};
    FixedData name;
    FixedData comment;
    int *dimension{};
    int count{};
};

struct Library {
    FixedData path;
    HMODULE hModule{};
    PLIB_INFO info{};
};

struct Module {
    Key key{};
    Key base{};
    FixedData name;
    FixedData comment;
    int property{};
    Key *include{};
    int number{};

    Variable *vars{};
    int varNumber{};
};

struct Sub {
    Key key{};

    int property{};
    int type{};
    FixedData name;
    FixedData comment;
    FixedData code[6];

    Variable *params{};
    int paramNumber{};

    Variable *locals{};
    int localNumber{};

    ASTProgramPtr ast;
};

struct DataStruct {
    Key key{};
    int property{};
    FixedData name;
    FixedData comment;
    Variable *members{};
    int memberNumber{};

};

struct DllFunc {
    Key key{};
    int property{};
    int type{};
    FixedData name;
    FixedData comment;
    FixedData lib;
    FixedData func;
    Variable *params{};
    int paramNumber{};

};

struct ECode {
    uint8_t *code{};
    // 源码信息
    BasicInfo info;
    //窗口
    Window *windows{};
    int windowNumber{};
    // 常量
    Constant *constants{};
    int constantNumber{};
    // 支持库
    Library *libraries{};
    int libraryNumber{};
    // 程序集/类
    Module *modules{};
    int moduleNumber{};
    // 所有子程序
    Sub *subs{};
    int subNumber{};
    // 全局变量
    Variable *globals{};
    int globalNumber{};
    // 自定义数据类型
    DataStruct *structs{};
    int structNumber{};
    // dll
    DllFunc *dlls{};
    int dllNumber{};

    void free() {
        ::free(code);
        for (int i = 0; i < libraryNumber; ++i) {
            FreeLibrary(libraries[i].hModule);
        }
        delete[] windows;
        delete[] constants;
        delete[] libraries;
        delete[] modules;
        delete[] subs;
        delete[] globals;
        delete[] structs;
        delete[] dlls;
    }
};

static char seg_start[4] = {0x19, 0x73, 0x11, 0x15};

static char sec_start[2] = {0x19, 0x73};

class ECodeParser {
public:
    FileBuffer &_buffer;
    ECode code{};
    int _check{0};
    char *_eLibPath;

    explicit ECodeParser(FileBuffer &buf) : _buffer(buf), _eLibPath(GetLibPath()) {
        code.code = buf.code;
    }

    ~ECodeParser() {
        delete[]_eLibPath;

    }

    bool Check(const char *check, size_t length);;

    bool Cmp(const char *check, size_t length);

    void SetElibPath(char *path);

    void Parse();

    ECode &GetECode() {
        return code;
    }

    bool CheckSegment(int num);

    /**
     * 跳过段校检
     */
    void SkipSegment();

    Key ParseKey();

    void ParseCodeSegement();

    void ParseLibrary();

    void ParseModule();

    void ParseInfoSegement(int arg);

    void ParseWindow();

    void ParseResourceSegement();

    void ParseConstant();

    Variable *ParseVariable(int &num);

    void ParseSub();

    void ParseDataStruct();

    void ParseDll();

    void ParseAST();

    Value ParseValue(FileBuffer &buf, uint8_t type) {
        switch (type) {
            case 22:
                // 空值
                return Value();
            case 23:
            {
                // double
                FixedData data = buf.Read(8);
                return Value(*((double *) data.data));
            }
            case 59:
                return Value(buf.ReadInt());
                // int
            case 24:
                // 逻辑
                return Value(buf.ReadShort() != 0);
            case 25:
            {
                // 日期时间
                FixedData data = buf.Read(8);
                return Value(*((long long *) data.data));
            }
            case 26:
                // 文本
                return Value(buf.ReadFixedData());
            default:
                return Value();
        }
    }

    ASTProgramPtr ParseSubCode(FileBuffer &buf) {
        ASTProgramPtr ast = make_ptr(ASTProgram);
        while (buf.Good()) {
            ast->AddStmt(ParseLineNode(buf, buf.ReadByte()));
        }
        return ast;
    };

    ASTNodePtr ParseLineNode(FileBuffer &buf, uint8_t type) {
        switch (type) {
            case 1:
                return ParseLineNode(buf, buf.ReadByte());
            case 106:
                // 行
                return ParseFunCall(buf);
            case 107:
                // 如果
                return ParseIf(buf);
            case 108:
                // 如果真
                return ParseIfTrue(buf);
            case 109:
                // 判断
                return ParseJudge(buf);
            case 112:
                // 循环
                return ParseLoop(buf);
            default:
                break;
        }
        NOT_REACHED();
        return nullptr;
    }

    ASTIfStmtPtr ParseIf(FileBuffer &buf) {
        ASTIfStmtPtr ifstmt = make_ptr(ASTIfStmt);
        ASTFunCallPtr ptr = ParseFunCall(buf);
        if (!ptr->args->args.empty()) {
            ifstmt->condition = ptr->args->args[0];
        }
        ifstmt->then_block = make_ptr(ASTList);
        uint8_t next;
        do {
            next = buf.ReadByte();
            if (next == 80 || next == 81)
                break;
            ifstmt->then_block->AddArg(ParseLineNode(buf, next));
        } while (buf.Good());
        if (next == 80) {
            ifstmt->else_block = make_ptr(ASTList);
            do {
                next = buf.ReadByte();
                if (next == 81)
                    break;
                ifstmt->else_block->AddArg(ParseLineNode(buf, next));
            } while (buf.Good());

        }
        buf.Check(114);
        return ifstmt;
    }

    ASTIfStmtPtr ParseIfTrue(FileBuffer &buf) {
        ASTIfStmtPtr ifstmt = make_ptr(ASTIfStmt);
        ASTFunCallPtr ptr = ParseFunCall(buf);
        ifstmt->condition = ptr;

/*
        if (!ptr->args->args.empty()) {
            ifstmt->condition = ptr->args->args[0];
        }
*/
        ifstmt->then_block = make_ptr(ASTList);
        uint8_t next;
        do {
            next = buf.ReadByte();
            if (next == 82)
                break;
            ifstmt->then_block->AddArg(ParseLineNode(buf, next));
        } while (buf.Good());
        buf.Check(115);
        return ifstmt;
    }

    ASTJudgePtr ParseJudge(FileBuffer &buf) {
        ASTJudgePtr ast = make_ptr(ASTJudge);
        uint8_t next;
        ASTListPtr block;
        do {
            next = buf.ReadByte();
            if (next == 110) {
                ASTFunCallPtr ptr = ParseFunCall(buf);
                if (!ptr->args->args.empty()) {
                    ast->conditions.push_back(ptr->args->args[0]);
                }
                block = make_ptr(ASTList);
            } else if (next == 111) {
                // 进入默认分支
                ast->default_block = make_ptr(ASTList);
                block = ast->default_block;
            } else if (next == 83) {
                // 分支结束
                ast->blocks.push_back(block);
            } else if (next == 84) {
                // 判断结束
                break;
            } else {
                if (block == nullptr)
                    return ast;
                block->args.push_back(ParseLineNode(buf, next));
            }
        } while (buf.Good());

        buf.Check(116);
        return ast;
    }

    ASTLoopPtr ParseLoop(FileBuffer &buf) {
        ASTLoopPtr ast = make_ptr(ASTLoop);
        ast->head = ParseFunCall(buf);
        ast->block = make_ptr(ASTList);
        uint8_t next;
        do {
            next = buf.ReadByte();
            if (next == 85)
                break;
            ast->block->args.push_back(ParseLineNode(buf, next));
        } while (buf.Good());
        if (buf.Check(85)) {
            ast->tail = ParseFunCall(buf);
        }
        buf.Check(113);
        return nullptr;
    }

    ASTFunCallPtr ParseFunCall(FileBuffer &buf) {
        ASTFunCallPtr ptr = make_ptr(ASTFunCall);
        ptr->key.index = buf.ReadShort();
        ptr->key.code = buf.ReadByte();
        ptr->key.type = (KeyType) buf.ReadByte();
        ptr->lib = buf.ReadShort();
        ptr->unkown = buf.ReadShort();
        ptr->object = buf.ReadFixedData();
        ptr->comment = buf.ReadFixedData();
        ptr->args = ParseArgs(buf);
        return ptr;
    }

    ASTNodePtr ParseNode(FileBuffer &buf, uint8_t type) {
        switch (type) {
            case 6:
                // 不知道是啥
                return ParseNode(buf, buf.ReadByte());
            case 27:
                // 自定义常量
                return make_ptr(ASTConstant, buf.ReadInt());
            case 28:
                // 支持库常量
                return make_ptr(ASTLibConstant, buf.ReadInt());
            case 29:
                // 变量
                return ParseNode(buf, buf.ReadByte());
            case 30:
                // 子程序指针
                return make_ptr(ASTAddress, buf.ReadInt());
            case 31:
                // 左大括号

                break;
            case 32:
                // 右大括号

                break;
            case 33:
                // 函数
                return ParseFunCall(buf);
            case 35:
                // 枚举常量
                return make_ptr(ASTEnumConstant, buf.ReadInt(), buf.ReadInt());
            case 56:
                // 对象开始
            {
                int mark = buf.ReadInt();
                if (mark == 0x500FFFE)
                    buf.ReadByte();

                ASTNodePtr ast = make_ptr(ASTVariable, mark);
                uint8_t next;
                while ((next = buf.ReadByte()) != 55) {
                    ast = make_ptr(ASTDot, ast, ParseNode(buf, next));
                }
                return ast;
            }
            case 57:
                // 数据成员
                return make_ptr(ASTStructMember, buf.ReadInt(), buf.ReadInt());
            case 58:
                // 数组下标
                return make_ptr(ASTSubscript, ParseNode(buf, buf.ReadByte()));
            default:
                if ((type >= 22 && type <= 26) || type == 59) {
                    return make_ptr(ASTLiteral, ParseValue(buf, type));
                }
                break;
        }
        return nullptr;
    }

    ASTListPtr ParseArgs(FileBuffer &buf) {
        ASTListPtr ast = make_ptr(ASTList);
        buf.Check(54);
        uint8_t type;
        while ((type = buf.ReadByte()) != 1) {
            ast->AddArg(ParseNode(buf, type));
        }
        return ast;
    }


private:


};


#endif //PARSE_E_FILE_ECODEPARSER_H
