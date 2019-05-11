//
// Created by 曹顺 on 2019/5/3.
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
#define NOT_REACHED() printf("not reached !! %s:%d\n", __FILE__, __LINE__);
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

struct EWindow {
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
    EWindow() = default;
};

struct EConst;

struct ELibConst {
    int lib; // lib index
    int index; // constant index
};

struct EConst {
    Key key;
    short property{};
    FixedData name;
    FixedData comment;

    union {
        FixedData data;
        Value value;
    };

};

struct EVar {
    Key key;
    int type{};
    short property{};
    FixedData name;
    FixedData comment;
    int *dimension{};
    int count{};
    void free() {
        delete[]dimension;
    }
};

struct ELibrary {
    FixedData path;
    HMODULE hModule{};
    PLIB_INFO info{};
};

struct EModule {
    Key key;
    Key base;
    FixedData name;
    FixedData comment;
    int property{};
    Key *include{};
    int number{};

    EVar *vars{};
    int varNumber{};

    inline bool has(Key test) {
        for (int i = 0; i < number; ++i) {
            if (test.value == include[i].value) {
                return true;
            }
        }
        return false;
    }

    void free() {
        for (int i = 0; i < varNumber; ++i) {
            vars[i].free();
        }
        delete[]include;
        delete[]vars;
    }
};

struct ESub {
    Key key{};

    int property{};
    int type{};
    FixedData name;
    FixedData comment;
    FixedData code[6];

    EVar *params{};
    int paramNumber{};

    EVar *locals{};
    int localNumber{};

    // 所属模块
    EModule *module;

    ASTProgramPtr ast;
    void free() {
        for (int i = 0; i < paramNumber; ++i) {
            params[i].free();
        }
        for (int i = 0; i < localNumber; ++i) {
            locals[i].free();
        }
        delete[]params;
        delete[]locals;
    }
};

struct EStruct {
    Key key{};
    int property{};
    FixedData name;
    FixedData comment;
    EVar *members{};
    int memberNumber{};
    void free() {
        for (int i = 0; i < memberNumber; ++i) {
            members[i].free();
        }
        delete[]members;
    }

};

struct EDllSub {
    Key key{};
    int property{};
    int type{};
    FixedData name;
    FixedData comment;
    FixedData lib;
    FixedData func;
    EVar *params{};
    int paramNumber{};
    void free() {
        for (int i = 0; i < paramNumber; ++i) {
            params[i].free();
        }
        delete[]params;
    }

};

struct ECode {
    uint8_t *code{};
    // 源码信息
    BasicInfo info;
    //窗口
    EWindow *windows{};
    int windowNumber{};
    // 常量
    EConst *constants{};
    int constantNumber{};
    // 支持库
    ELibrary *libraries{};
    int libraryNumber{};
    // 程序集/类
    EModule *modules{};
    int moduleNumber{};
    // 所有子程序
    ESub *subs{};
    int subNumber{};
    // 全局变量
    EVar *globals{};
    int globalNumber{};
    // 自定义数据类型
    EStruct *structs{};
    int structNumber{};
    // dll
    EDllSub *dlls{};
    int dllNumber{};

    void free() {
        ::free(code);
        for (int i = 0; i < libraryNumber; ++i) {
            FreeLibrary(libraries[i].hModule);
        }
        for (int i = 0; i < moduleNumber; ++i) {
            modules[i].free();
        }
        for (int i = 0; i < subNumber; ++i) {
            subs[i].free();
        }
        for (int i = 0; i < globalNumber; ++i) {
            globals[i].free();
        }
        for (int i = 0; i < structNumber; ++i) {
            structs[i].free();
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

    bool Check(const char *check, size_t length);
    bool Cmp(const char *check, size_t length);
    void SetElibPath(char *path);
    void Parse();
    ECode &GetECode() {
        return code;
    }
private:

    bool CheckSegment(int num);
    void SkipSegment();
    Key ParseKey();
    void ParseCodeSegement();
    void ParseLibrary();
    void ParseModule();
    void ParseInfoSegement(int arg);
    void ParseWindow();
    void ParseResourceSegement();
    void ParseConstant();
    EVar *ParseVariable(int &num);
    void ParseSub();
    void ParseDataStruct();
    void ParseDll();
    void ParseAST();
    Value ParseValue(FileBuffer &buf, uint8_t type);
    ASTProgramPtr ParseSubCode(FileBuffer &buf);;
    ASTNodePtr ParseLineNode(FileBuffer &buf, uint8_t type);
    ASTIfStmtPtr ParseIf(FileBuffer &buf);
    ASTIfStmtPtr ParseIfTrue(FileBuffer &buf);
    ASTJudgePtr ParseJudge(FileBuffer &buf);
    ASTLoopPtr ParseLoop(FileBuffer &buf);
    ASTFunCallPtr ParseFunCall(FileBuffer &buf);
    ASTNodePtr ParseNode(FileBuffer &buf, uint8_t type);
    ASTArgsPtr ParseArgs(FileBuffer &buf);

};


#endif //PARSE_E_FILE_ECODEPARSER_H
