#ifndef COMPILER_FILE_WRITER
#define COMPILER_FILE_WRITER

#include <stdbool.h>

#include "map.h"
#include "node.h"

void writeScanner( Node* node, Map* map, const char* fileName );

#endif