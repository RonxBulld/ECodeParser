//
// Created by ��˳ on 2019/5/3.
//

#ifndef PARSE_E_FILE_ECODEPARSER_H
#define PARSE_E_FILE_ECODEPARSER_H
#define assert(condition, str) \
    do { \
    if(!(condition)) \
        std::cout << str;\
    } while(0);
#include <vector>
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
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;
    FixedData cursor;
    FixedData mark;
    int visible = 0;
    int bidden = 0;
    int border = 0;
    int bgSize = 0;
    int bgColor = 0;
    int maxBtn = 0;
    int minBtn = 0;
    int ctrlBtn = 0;
    int position = 0;
    int movable = 0;
    int musicTimes = 0;
    int enterFocus = 0;
    int escClose = 0;
    int f1Help = 0;
    int helpMark = 0;
    int showInTaskbar = 0;
    int mov = 0; // �����ƶ�
    int shape = 0;
    int alwaysTop = 0;
    int alwaysActive = 0;
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
    short property = 0;
    FixedData name;
    FixedData comment;
    FixedData data;
    Value value;
};

struct EVar {
    Key key;
    int type = 0;
    short property = 0;
    FixedData name;
    FixedData comment;
    std::vector<int> dimension;
};

struct ELibrary {
    FixedData path;
    HMODULE hModule = nullptr;
    PLIB_INFO info = nullptr;
};

struct EModule {
    Key key;
    Key base;
    FixedData name;
    FixedData comment;
    int property = 0;
    std::vector<Key> include;
    std::vector<EVar> vars;
    inline bool has(Key test) {
        for (auto & i : include) {
            if (test.value == i.value) {
                return true;
            }
        }
        return false;
    }
};

struct ESub {
    Key key{};
    int property{};
    int type{};
    FixedData name;
    FixedData comment;
    FixedData code[6];
    std::vector<EVar> params;
    std::vector<EVar> locals;
    EModule *module; // ����ģ��
    ASTProgramPtr ast;
};

struct EStruct {
    Key key{};
    int property{};
    FixedData name;
    FixedData comment;
    std::vector<EVar> members;
};

struct EDllSub {
    Key key{};
    int property{};
    int type{};
    FixedData name;
    FixedData comment;
    FixedData lib;
    FixedData func;
    std::vector<EVar> params;
};

struct ECode {
    uint8_t *code{};
    BasicInfo info; // Դ����Ϣ
    std::vector<EWindow> windows; //����
    std::vector<EConst> constants; //����
    std::vector<ELibrary> libraries; //֧�ֿ�
    std::vector<EModule> modules; // ����/��
    std::vector<ESub> subs; // �����ӳ���
    std::vector<EVar> globals; // ȫ�ֱ���
    std::vector<EStruct> structs; // �Զ�����������
    std::vector<EDllSub> dlls; // dll
    void free() {
        ::free(code);
        for (auto & librarie : libraries) {
            FreeLibrary(librarie.hModule);
        }
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
    void ParseVariable(std::vector<EVar> &vars);
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
