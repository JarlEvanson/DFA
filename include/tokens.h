#ifndef COMPILER_TOKENS
#define COMPILER_TOKENS

#include "common.h"

typedef enum TokenType {
    WORD,
    NUMBER_LITERAL,
    COMMENT,
    WHITESPACE,
    ERROR
} TokenType;

typedef struct Token {
    TokenType type;
    char* str;
    size_t charPos;
    uint32_t line;
} Token;

#endif