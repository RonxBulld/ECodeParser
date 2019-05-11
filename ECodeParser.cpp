//
// Created by 曹顺 on 2019/5/3.
//

#include "ECodeParser.h"

ostream &operator<<(ostream &os, Key &key) {
    os << "{" << key.index << "-" << (int) key.type << "-" << (int) key.code << "}";
    return os;
}

ostream &operator<<(ostream &os, FixedData &data) {
    string str(data.data, static_cast<unsigned int>(data.length));
    os << str;
    return os;
}


bool ECodeParser::Check(const char *check, size_t length) {
    if (Cmp(check, length)) {
        _buffer.pos += length;
        return true;
    } else {
        return false;
    }
}

bool ECodeParser::Cmp(const char *check, size_t length) {
    if (_buffer.pos + length > _buffer.length) {
        return false;
    }
    return memcmp(&_buffer.code[_buffer.pos], check, length) == 0;
}

void ECodeParser::SetElibPath(char *path) {
    int length = strlen(path);
    delete[]_eLibPath;
    _eLibPath = new char[length + 1];
    strcpy(_eLibPath, path);
}

void ECodeParser::Parse() {
    assert(Check("CNWTEPRG", 8), "not a e code file!");
    while (_buffer.Good()) {
        // 查找段
        if (Check(seg_start, 4)) {
            _check++; // 用来校检
            int arg = _buffer.ReadInt();
            int flag = _buffer.ReadInt();
            int type = (flag >> 24) & 0xff;
            while (!Cmp(sec_start, 2))
                _buffer.pos++;
            SkipSegment();
            switch (type) {
                case 1:
                    ParseInfoSegement(arg);
                    break;
                case 2:
                    _buffer.Skip(60);
                    continue;
                case 3:
                    ParseCodeSegement();
                    break;
                case 4:
                    ParseResourceSegement();
                    break;
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                    ParseAST();
                    return;
                default:
                    break;
            }

        } else {
            _buffer.pos++;
        }
    }
}

bool ECodeParser::CheckSegment(int num) {
    _buffer.Skip(4 * num + 1);
    int temp_check = _buffer.ReadInt();
    if (temp_check != _check) {
        std::cout << "open e file error!" << std::endl;
        return false;
    }
    _buffer.Skip(52);
    return true;
}

void ECodeParser::SkipSegment() {
    while (Cmp(sec_start, 2))
        _buffer.Skip(4);
    _buffer.Skip(1);
    int check = _buffer.ReadInt();
    if (check != _check) {
        std::cout << "文件可能有错误!" << std::endl;
    }
    _buffer.Skip(52);
}

Key ECodeParser::ParseKey() {
    Key key{};
    key.index = _buffer.ReadShort();
    key.code = _buffer.ReadByte();
    key.type = (KeyType) _buffer.ReadByte();
    return key;
}

void ECodeParser::ParseCodeSegement() {
    ParseLibrary();
    ParseModule();
    ParseSub();
    code.globals = ParseVariable(code.globalNumber);
    ParseDataStruct();
    ParseDll();
}

void ECodeParser::ParseLibrary() {
    Key key = ParseKey();
    _buffer.Skip(4);
    int num = code.libraryNumber = _buffer.ReadInt() >> 2;
    code.libraries = new ELibrary[num];
    _buffer.Skip(10 + num * 8);
    int length = strlen(_eLibPath);
    char buf[255] = {'\0'};
    strcpy(buf, _eLibPath);
    for (int i = 0; i < num; ++i) {
        code.libraries[i].path = _buffer.ReadFixedData();
        char *data = code.libraries[i].path.data;
        int j = 0;
        do {
            buf[length + j++] = *data;
        } while (*++data != '\r');
        buf[length + j] = '\0';
        strcat(buf, ".fne");
        code.libraries[i].hModule = LoadLibraryA(buf);
        auto fn = (PFN_GET_LIB_INFO) GetProcAddress(code.libraries[i].hModule, FUNCNAME_GET_LIB_INFO);
        code.libraries[i].info = fn();
    }

}

void ECodeParser::ParseModule() {
    _buffer.Skip(8);
    _buffer.Skip(_buffer.ReadInt());
    _buffer.Skip(_buffer.ReadInt());
    int num = code.moduleNumber = _buffer.ReadInt() >> 3;
    code.modules = new EModule[num];
    for (int i = 0; i < num; ++i) {
        code.modules[i].key = ParseKey();
    }
    _buffer.Skip(num * 4);
    for (int j = 0; j < num; ++j) {
        code.modules[j].property = _buffer.ReadInt();
        code.modules[j].base = ParseKey();
        code.modules[j].name = _buffer.ReadFixedData();
        code.modules[j].comment = _buffer.ReadFixedData();
        int value = _buffer.ReadInt();
        int n = code.modules[j].number = value >> 2;
        code.modules[j].include = new Key[n];
        for (int i = 0; i < n; ++i) {
            code.modules[j].include[i] = ParseKey();
        }
        code.modules[j].vars = ParseVariable(code.modules[j].varNumber);
    }
}

void ECodeParser::ParseInfoSegement(int arg) {
    code.info.type = arg & 0xff;
    code.info.name = _buffer.ReadFixedData();
    code.info.description = _buffer.ReadFixedData();
    code.info.author = _buffer.ReadFixedData();
    code.info.code = _buffer.ReadFixedData();
    code.info.address = _buffer.ReadFixedData();
    code.info.telephone = _buffer.ReadFixedData();
    code.info.fox = _buffer.ReadFixedData();
    code.info.email = _buffer.ReadFixedData();
    code.info.host = _buffer.ReadFixedData();
    code.info.copyright = _buffer.ReadFixedData();
    code.info.version[0] = _buffer.ReadInt();
    code.info.version[1] = _buffer.ReadInt();
    code.info.create[0] = _buffer.ReadInt();
    code.info.create[1] = _buffer.ReadInt();
}

void ECodeParser::ParseWindow() {
    code.windowNumber = _buffer.ReadInt() >> 3;
    code.windows = new EWindow[code.windowNumber];
    for (int i = 0; i < code.windowNumber; ++i) {
        code.windows[i].key = ParseKey();
    }
    _buffer.Skip(code.windowNumber << 2);
    for (int j = 0; j < code.windowNumber; ++j) {
        _buffer.Skip(4);
        code.windows[j].belong = ParseKey();
        code.windows[j].name = _buffer.ReadFixedData();
        code.windows[j].comment = _buffer.ReadFixedData();
        code.windows[j].number = _buffer.ReadInt();

        // 下一个窗口的偏移
        int offset = _buffer.ReadInt();
        assert(offset > 0, "read windows error");
        offset += _buffer.pos;

        _buffer.Skip(34 + (code.windows[j].number << 3));

        code.windows[j].left = _buffer.ReadInt();
        code.windows[j].top = _buffer.ReadInt();
        code.windows[j].height = _buffer.ReadInt();
        code.windows[j].width = _buffer.ReadInt();

        _buffer.Skip(12);
        code.windows[j].cursor = _buffer.ReadFixedData();
        code.windows[j].mark = _buffer.ReadFixedData();

        _buffer.Skip(4);
        int n = _buffer.ReadInt();
        code.windows[j].visible = n & 1;
        code.windows[j].bidden = n & 2;

        _buffer.Skip(4);

        n = _buffer.ReadInt();
        _buffer.Skip(n << 3);
        if (_buffer.pos > offset - 24) {
            _buffer.pos = static_cast<size_t>(offset);
            continue;
        }

        _buffer.Skip(12);
        int property = _buffer.ReadInt();
        if (property > 0 && property <= 6) {
            code.windows[j].border = _buffer.ReadInt();
            code.windows[j].bgSize = _buffer.ReadInt();
            code.windows[j].bgColor = _buffer.ReadInt();
            code.windows[j].maxBtn = _buffer.ReadInt();
            code.windows[j].minBtn = _buffer.ReadInt();
            code.windows[j].ctrlBtn = _buffer.ReadInt();
            code.windows[j].position = _buffer.ReadInt();
            code.windows[j].movable = _buffer.ReadInt();
            code.windows[j].musicTimes = _buffer.ReadInt();
            code.windows[j].enterFocus = _buffer.ReadInt();
            code.windows[j].escClose = _buffer.ReadInt();
            code.windows[j].f1Help = _buffer.ReadInt();
            code.windows[j].helpMark = _buffer.ReadInt();
            code.windows[j].helpMark = _buffer.ReadInt();
            code.windows[j].helpMark = _buffer.ReadInt();
            code.windows[j].helpMark = _buffer.ReadInt();
            code.windows[j].helpMark = _buffer.ReadInt();
            code.windows[j].showInTaskbar = _buffer.ReadInt();
            code.windows[j].mov = _buffer.ReadInt();
            code.windows[j].shape= _buffer.ReadInt();
            code.windows[j].alwaysTop= _buffer.ReadInt();
            code.windows[j].alwaysActive= _buffer.ReadInt();
        }
        if (property == 6) {
            code.windows[j].className = _buffer.ReadFixedData();
        }
        if (property >= 2) {
            code.windows[j].title = _buffer.ReadFixedData();
        }
        code.windows[j].helpFileName = _buffer.ReadFixedData();
        _buffer.pos = static_cast<size_t>(offset);
    }

}

void ECodeParser::ParseResourceSegement() {
    ParseWindow();
    ParseConstant();
}

void ECodeParser::ParseConstant() {
    code.constantNumber = _buffer.ReadInt();
    code.constants = (EConst *) malloc(code.constantNumber * sizeof(EConst));
    _buffer.Skip(4);
    for (int i = 0; i < code.constantNumber; ++i) {
        code.constants[i].key = ParseKey();
    }
    _buffer.Skip(code.constantNumber * 4);
    for (int j = 0; j < code.constantNumber; ++j) {
        _buffer.Skip(4);
        code.constants[j].property = _buffer.ReadShort();
        code.constants[j].name = _buffer.ReadString();
        code.constants[j].comment = _buffer.ReadString();
        if (code.constants[j].key.type == KeyType_ImageRes || code.constants[j].key.type == KeyType_SoundRes) {
            code.constants[j].data = _buffer.ReadFixedData();
        } else {
            uint8_t  type = _buffer.ReadByte();
            code.constants[j].value = ParseValue(_buffer, type);
        }

    }

}

EVar *ECodeParser::ParseVariable(int &num) {
    num = _buffer.ReadInt();
    size_t offset = _buffer.ReadInt() + _buffer.pos;
    EVar *var = num ? new EVar[num] : nullptr;
    for (int k = 0; k < num; ++k) {
        var[k].key = ParseKey();
    }
    _buffer.Skip(num * 4);
    for (int i = 0; i < num; ++i) {
        size_t next_offset = _buffer.ReadInt() + _buffer.pos;
        var[i].type = _buffer.ReadInt();
        var[i].property = _buffer.ReadShort();
        var[i].count = _buffer.ReadByte();
        var[i].dimension = var[i].count ? new int[var[i].count] : nullptr;
        for (int j = 0; j < var[i].count; ++j) {
            var[i].dimension[j] = _buffer.ReadInt();
        }
        var[i].name = _buffer.ReadString();
        var[i].comment = _buffer.ReadString();
        _buffer.pos = next_offset;
    }
    _buffer.pos = offset;
    return var;
}

void ECodeParser::ParseSub() {
    code.subNumber = _buffer.ReadInt() >> 3;
    code.subs = new ESub[code.subNumber];
    for (int i = 0; i < code.subNumber; ++i) {
        code.subs[i].key = ParseKey();
        for (int j = 0; j < code.moduleNumber; ++j) {
            if (code.modules[j].has(code.subs[i].key)) {
                code.subs[i].module = &code.modules[j];
                break;
            }
        }
    }
    _buffer.Skip(code.subNumber * 4);
    for (int j = 0; j < code.subNumber; ++j) {
        _buffer.Skip(4);
        code.subs[j].property = _buffer.ReadInt();
        code.subs[j].type = _buffer.ReadInt();
        code.subs[j].name = _buffer.ReadFixedData();
        code.subs[j].comment = _buffer.ReadFixedData();
        code.subs[j].locals = ParseVariable(code.subs[j].localNumber);
        code.subs[j].params = ParseVariable(code.subs[j].paramNumber);
        for (auto &i : code.subs[j].code) {
            i = _buffer.ReadFixedData();
        }
    }

}

void ECodeParser::ParseDataStruct() {
    code.structNumber = _buffer.ReadInt() >> 3;
    code.structs = new EStruct[code.structNumber];
    for (int i = 0; i < code.structNumber; ++i) {
        code.structs[i].key = ParseKey();
    }
    _buffer.Skip(code.structNumber * 4);
    for (int j = 0; j < code.structNumber; ++j) {
        code.structs[j].property = _buffer.ReadInt();
        code.structs[j].name = _buffer.ReadFixedData();
        std::cout << code.structs[j].name << std::endl;
        code.structs[j].comment = _buffer.ReadFixedData();
        code.structs[j].members = ParseVariable(code.structs[j].memberNumber);
    }

}

void ECodeParser::ParseDll() {
    code.dllNumber = _buffer.ReadInt() >> 3;
    code.dlls = new EDllSub[code.dllNumber];
    for (int i = 0; i < code.dllNumber; ++i) {
        code.dlls[i].key = ParseKey();
    }
    _buffer.Skip(code.dllNumber * 4);
    for (int j = 0; j < code.dllNumber; ++j) {
        code.dlls[j].property = _buffer.ReadInt();
        code.dlls[j].type = _buffer.ReadInt();
        code.dlls[j].name = _buffer.ReadFixedData();
        code.dlls[j].comment = _buffer.ReadFixedData();
        code.dlls[j].lib = _buffer.ReadFixedData();
        code.dlls[j].func = _buffer.ReadFixedData();
        code.dlls[j].params = ParseVariable(code.dlls[j].paramNumber);
    }
}

void ECodeParser::ParseAST() {
    for (int i = 0; i < code.subNumber; ++i) {
        FileBuffer code_buf(code.subs[i].code[5].data, (size_t) code.subs[i].code[5].length);
        code.subs[i].ast = ParseSubCode(code_buf);

    }
}

Value ECodeParser::ParseValue(FileBuffer &buf, uint8_t type) {
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

ASTProgramPtr ECodeParser::ParseSubCode(FileBuffer &buf) {
    ASTProgramPtr ast = make_ptr(ASTProgram);
    while (buf.Good()) {
        ast->AddStmt(ParseLineNode(buf, buf.ReadByte()));
    }
    return ast;
}

ASTNodePtr ECodeParser::ParseLineNode(FileBuffer &buf, uint8_t type) {
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
    return nullptr;
}

ASTIfStmtPtr ECodeParser::ParseIf(FileBuffer &buf) {
    ASTIfStmtPtr ifstmt = make_ptr(ASTIfStmt);
    ASTFunCallPtr ptr = ParseFunCall(buf);
    if (!ptr->args->args.empty()) {
        ifstmt->condition = ptr->args->args[0];
    }
    ifstmt->then_block = make_ptr(ASTBlock);
    uint8_t next;
    do {
        next = buf.ReadByte();
        if (next == 80 || next == 81)
            break;
        ifstmt->then_block->AddStmt(ParseLineNode(buf, next));
    } while (buf.Good());
    if (next == 80) {
        ifstmt->else_block = make_ptr(ASTBlock);
        do {
            next = buf.ReadByte();
            if (next == 81)
                break;
            ifstmt->else_block->AddStmt(ParseLineNode(buf, next));
        } while (buf.Good());

    }
    buf.Match(114);
    return ifstmt;
}

ASTIfStmtPtr ECodeParser::ParseIfTrue(FileBuffer &buf) {
    ASTIfStmtPtr ifstmt = make_ptr(ASTIfStmt);
    ASTFunCallPtr ptr = ParseFunCall(buf);
    if (!ptr->args->args.empty()) {
        ifstmt->condition = ptr->args->args[0];
    }
    ifstmt->then_block = make_ptr(ASTBlock);
    uint8_t next;
    do {
        next = buf.ReadByte();
        if (next == 82)
            break;
        ifstmt->then_block->AddStmt(ParseLineNode(buf, next));
    } while (buf.Good());
    buf.Match(115);
    return ifstmt;
}

ASTJudgePtr ECodeParser::ParseJudge(FileBuffer &buf) {
    ASTJudgePtr ast = make_ptr(ASTJudge);
    uint8_t next;
    ASTBlockPtr block;
    do {
        next = buf.ReadByte();
        if (next == 110) {
            ASTFunCallPtr ptr = ParseFunCall(buf);
            if (!ptr->args->args.empty()) {
                ast->conditions.push_back(ptr->args->args[0]);
            }
            block = make_ptr(ASTBlock);
        } else if (next == 111) {
            ast->blocks.push_back(block);
            // 进入默认分支
            ast->default_block = make_ptr(ASTBlock);
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
            block->AddStmt(ParseLineNode(buf, next));
        }
    } while (buf.Good());

    buf.Match(116);
    return ast;
}

ASTLoopPtr ECodeParser::ParseLoop(FileBuffer &buf) {
    ASTLoopPtr ast = make_ptr(ASTLoop);
    ast->head = ParseFunCall(buf);
    ast->block = make_ptr(ASTBlock);
    uint8_t next;
    do {
        next = buf.ReadByte();
        if (next == 85)
            break;
        ast->block->AddStmt(ParseLineNode(buf, next));
    } while (buf.Good());
    if (buf.Match(113)) {
        ast->tail = ParseFunCall(buf);
    }
    return ast;
}

ASTFunCallPtr ECodeParser::ParseFunCall(FileBuffer &buf) {
    ASTFunCallPtr ptr = make_ptr(ASTFunCall);
    ptr->key.value = buf.ReadInt();
    ptr->lib = buf.ReadShort();
    ptr->unknown = buf.ReadShort();
    ptr->object = buf.ReadFixedData();
    ptr->comment = buf.ReadFixedData();
    ptr->args = ParseArgs(buf);
    return ptr;
}

ASTNodePtr ECodeParser::ParseNode(FileBuffer &buf, uint8_t type) {
    switch (type) {
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

            return make_ptr(ASTBrace, ParseArgs(buf));

            NOT_REACHED();
            break;
        case 32:
            // 右大括号
            NOT_REACHED();
            break;
        case 33:
            // 函数
            return ParseFunCall(buf);
        case 35:
            // 枚举常量
            return make_ptr(ASTEnumConstant, buf.ReadInt(), buf.ReadInt());
        case 54:
            // 左小括号
            return ParseNode(buf, buf.ReadByte());
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
    NOT_REACHED();
    return nullptr;
}

ASTArgsPtr ECodeParser::ParseArgs(FileBuffer &buf) {
    ASTArgsPtr ast = make_ptr(ASTArgs);
    buf.Match(54);

    uint8_t type;
    do {
        type = buf.ReadByte();
        if (type == 1 || type == 0 || type == 32) {
            break;
        }
        ast->AddArg(ParseNode(buf, type));
    } while (buf.Good());
    return ast;
}
