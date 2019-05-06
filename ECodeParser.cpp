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
    code.libraries = new Library[num];
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
    code.modules = new Module[num];
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
    code.windows = new Window[code.windowNumber];
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
    code.constants = (Constant *) malloc(code.constantNumber * sizeof(Constant));
    _buffer.Skip(4);
    for (int i = 0; i < code.constantNumber; ++i) {
        code.constants[i].key = ParseKey();
    }
    _buffer.Skip(code.constantNumber * 4);
    for (int j = 0; j < code.constantNumber; ++j) {
        _buffer.Skip(4);
        code.constants[j].property = _buffer.ReadInt();
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

Variable *ECodeParser::ParseVariable(int &num) {
    num = _buffer.ReadInt();
    size_t offset = _buffer.ReadInt() + _buffer.pos;
    Variable *var = num ? new Variable[num] : nullptr;
    for (int k = 0; k < num; ++k) {
        var[k].key = ParseKey();
    }
    _buffer.Skip(num * 4);
    for (int i = 0; i < num; ++i) {
        size_t next_offset = _buffer.ReadInt() + _buffer.pos;
        var[i].type = _buffer.ReadShort();
        var[i].property = _buffer.ReadShort();
        var[i].count = _buffer.ReadByte();
        var[i].dimension = var[i].count ? new int[var[i].count] : nullptr;
        for (int j = 0; j < var[i].count; ++j) {
            var[i].dimension[j] = _buffer.ReadInt();
        }
        _buffer.Skip(2);
        var[i].name = _buffer.ReadString();
        var[i].comment = _buffer.ReadString();
        _buffer.pos = next_offset;
    }
    _buffer.pos = offset;
    return var;
}

void ECodeParser::ParseSub() {
    code.subNumber = _buffer.ReadInt() >> 3;
    code.subs = new Sub[code.subNumber];
    for (int i = 0; i < code.subNumber; ++i) {
        code.subs[i].key = ParseKey();
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
    code.structs = new DataStruct[code.structNumber];
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
    code.dlls = new DllFunc[code.dllNumber];
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
