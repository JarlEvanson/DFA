#ifndef DFA_BUFFER
#define DFA_BUFFER

#include "common.h"

#include <stdio.h>
#include <stdarg.h>

typedef struct Buffer {
    FILE* mFile;
    const char* mCurrentFile;
    uint8_t mBuffer[2][4096];
    uint8_t* mCurrBufferLoc;
} Buffer;

void initBuffer(Buffer* buffer, const char* fileName);
size_t writeBuffer(Buffer* buffer, const char* fmt, ...);
void clearBuffer(Buffer* buffer);
void closeFile(Buffer* buffer);

#endif