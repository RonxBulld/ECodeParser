#define _X86_
#include <iostream>
#include <windows.h>
#include "FileBuffer.h"
#include "ECodeParser.h"
using namespace std;

int main() {
    FileBuffer buffer("code.e");
    ECodeParser parser(buffer);

    parser.Parse();


    return 0;
}
