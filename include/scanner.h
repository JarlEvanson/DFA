#ifndef COMPILER_SCANNER
#define COMPILER_SCANNER

#include <stdio.h>

#include "common.h"
#include "tokens.h"


typedef struct Scanner {
    FILE* mFile;
    const char* mCurrentFile;
    uint8_t mBuffer[2][4097];
    uint8_t* mLexemmeBegin;
    uint8_t* mForward;
    size_t mCurrentPos;
    size_t mLine;
} Scanner;

void initScanner(Scanner* scanner, const char* fileName);
Token nextToken(Scanner* scanner);

#endif