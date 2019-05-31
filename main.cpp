#define _X86_
#include <iostream>
#include <windows.h>
#include "FileBuffer.h"
#include "ECodeParser.h"
#include <c_translator.h>
#include "test.h"
using namespace std;

int main() {
    FileBuffer buffer("code.e");
    ECodeParser parser(buffer);
    parser.Parse();

    DumpVisitor dv(&parser.code);

    int num = parser.code.subNumber;
    for (int i = 0; i < num; ++i) {
        cout << endl << endl;
        cout << "sub : " << parser.code.subs[i].name << endl;
        dv.current = &parser.code.subs[i];
        parser.code.subs[i].ast->accept(&dv);
    }

    parser.code.free();

    return 0;
}
