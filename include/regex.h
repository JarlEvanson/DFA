#ifndef COMPILER_REGEX
#define COMPILER_REGEX

#include "common.h"
#include "node.h"
#include "map.h"

Node* buildRegexSet( Map* map );

Node* genMinimalDFA( const char* str );
Node* subsetConstruction(Node* root);

//Node* genRegexes( Map map );
//void buildRegexFunction( Node* root, const char* fileName, Map map );

#endif