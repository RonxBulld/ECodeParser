#define _X86_
#include <iostream>
#include <windows.h>
#include "FileBuffer.h"
#include "ECodeParser.h"
#include <c_translator.h>
#include "test.h"
using namespace std;

int main() {
    FileBuffer buffer("C:\\Users\\Administrator\\Desktop\\ECodeParser\\code.e");
    ECodeParser parser(buffer);
    parser.Parse();

    DumpVisitor dv(&parser.code);
    for (auto & sub : parser.code.subs) {
        cout << endl << endl;
        cout << "sub : " << sub.name << endl;
        dv.current = &sub;
        sub.ast->accept(&dv);
    }

    parser.code.free();

    return 0;
}
