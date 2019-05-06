//
// Created by ≤‹À≥ on 2019/5/3.
//

#ifndef PARSE_E_FILE_FILEBUFFER_H
#define PARSE_E_FILE_FILEBUFFER_H

#include <iostream>
#include <cstdio>
#include <sys/stat.h>
#include <cstdlib>
struct FixedData {
    char *data{nullptr};
    int length{0};
    FixedData(char *data, int length) : data(data), length(length) {}
    FixedData() = default;
};

class FileBuffer {
public:
    uint8_t *code{nullptr};
    size_t length{0};
    size_t pos{0};
    explicit FileBuffer(const char *file) {
        FILE *f = fopen(file, "rb");
        struct stat st{};
        stat(file, &st);
        length = static_cast<size_t>(st.st_size);
        code = (uint8_t *) malloc(length);
        if (fread(code, sizeof(uint8_t), length, f) < length) {
            std::cout << "read error"  << std::endl;
        }
        fclose(f);
    }

    FileBuffer(char *data, size_t length) : code((uint8_t *) data), length(length) {}

    inline bool Good() {
        return pos < length;
    }

    inline void Skip(int step) {
        pos += step;
    }

    int ReadInt() {
        // –°∂À◊÷Ω⁄–Ú
        pos += 4;
        return (code[pos - 4] | (code[pos - 3] << 8) | (code[pos - 2] << 16) | (code[pos - 1] << 24));
    }

    short ReadShort() {
        // –°∂À◊÷Ω⁄–Ú
        pos += 2;
        return (code[pos - 2] | (code[pos - 1] << 8));
    }

    uint8_t ReadByte() {
        return code[pos++];
    }

    inline FixedData ReadFixedData() {
        int length = ReadInt();
        if (length > 0) {
            FixedData data((char *) &code[pos], length);
            pos += data.length;
            return data;
        } else {
            return {nullptr, 0};
        }

    }

    inline FixedData ReadString() {
        int length = strlen((char *) &code[pos]);
        FixedData data((char *) &code[pos], length);
        pos += length + 1;
        return data;
    }

    inline FixedData Read(int length) {
        FixedData data((char *) &code[pos], length);
        pos += data.length;
        return data;
    }

    inline bool Match(uint8_t byte) {
        if (code[pos] == byte) {
            pos++;
            return true;
        }
        return false;
    }

    inline bool Check(uint8_t byte) {
        return code[pos] == byte;
    }

};


#endif //PARSE_E_FILE_FILEBUFFER_H
