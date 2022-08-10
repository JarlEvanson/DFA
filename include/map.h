#ifndef COMPILER_MAP
#define COMPILER_MAP

#include "tokens.h"

typedef struct Pair {
    const Token* toks;
    uint32_t tokCount;
    const char* type; //TokenID
    uint32_t precedence; //0 is lowest
    bool shouldDiscard;
} Pair;

typedef struct Map {
    Pair* pairs;
    uint32_t numPairs;
} Map;

#endif