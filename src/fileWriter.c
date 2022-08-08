#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "node.h"

#include "vector.h"
#include "tokens.h"
#include "map.h"
#include "buffer.h"

static void printChar(Buffer* buffer, uint8_t character ) {
    if( character >= 33 && character <= 126 && (character != '\\' && character != '\'') ) {
        writeBuffer( buffer, "%c", character );
    } else {
        switch( character ) {
            case '\n':
                writeBuffer( buffer, "\\n");
                break;
            case '\r':
                writeBuffer( buffer, "\\r");
                break;
            case '\t':
                writeBuffer( buffer, "\\t");
                break;
            case '\\':
                writeBuffer( buffer, "\\\\");
                break;
            case '\'':
                writeBuffer( buffer, "\\\'");
                break;
            default:
                writeBuffer( buffer, "\\%o", character);
                break;
        }
    }
}

void buildNode(Buffer* buffer, Node* node, Map* map) {
    writeBuffer( buffer, "s%u:\n\t", getNodeState( node ));

    bool hasIf = false;
    for( uint32_t connIndex = 0; connIndex < getNodeConnectionCount( node ); ) {
        if( !hasIf ) {
            writeBuffer( buffer, "ch = this->nextChar();\n");
        } 
        writeBuffer( buffer, "\t");
        if( hasIf ) {
            writeBuffer( buffer, "} else ");
        }
        uint32_t numSameConnectee =  consecutiveSameNodeConnections( node, connIndex );
        if( numSameConnectee < 4 ) {
            writeBuffer( buffer, "if( ch == \'");
            Edge* edge = getNthConnection( node, connIndex );
            printChar( buffer, getEdgeKey( edge ) );
            writeBuffer( buffer, "\' ) {\n\t\tgoto s%u;\n", getNodeState( getEdgeConnectee( edge ) ) );
            connIndex++;
        } else {
            writeBuffer( buffer, "if( ch >= \'" );
            Edge* edge1 = getNthConnection( node, connIndex );
            printChar( buffer, getEdgeKey( edge1 ) );
            writeBuffer( buffer, "\' && ch <= \'");
            Edge* edge2 = getNthConnection( node, connIndex + numSameConnectee - 1 );
            printChar( buffer, getEdgeKey( edge2 ) );
            writeBuffer( buffer, "\' ) {\n\t\tgoto s%u;\n", getNodeState( getEdgeConnectee( edge1 ) ) );
            connIndex += numSameConnectee;
        }
        
        hasIf = true; 
    }

    if( isNodeAccepting( node ) ) {
        uint32_t numString = 0;
        const char** strings = getNodeStrings( node, &numString );

        if( hasIf ) {
            writeBuffer( buffer, "\t} else {\n\t\t");
        }
        uint32_t highestPrecedence = 0;
        char* string = NULL;
        for( uint32_t stringIndex = 0; stringIndex < numString; stringIndex++) {
            for( uint32_t keyIndex = 0; keyIndex < map->numPairs; keyIndex++ ) {
               if( strcmp( strings[stringIndex], map->pairs[keyIndex].type ) == 0 ) {
                  if( map->pairs[keyIndex].precedence >= highestPrecedence ) {
                     string = map->pairs[keyIndex].type;
                     highestPrecedence = map->pairs[keyIndex].precedence;
                  } 
               }
            }
        }
        writeBuffer( buffer, "token.type = %s;\n", string );
        if( hasIf ) {
            writeBuffer( buffer, "\t\trollback = 1;\n");
            writeBuffer( buffer, "\t");
        }
        writeBuffer( buffer, "\tgoto accept;\n");
        if( hasIf )
            writeBuffer( buffer, "\t}\n");
    } else {
        if( hasIf ) {
            writeBuffer( buffer, "\t} else {\n\t\t");
        }

        writeBuffer( buffer, "token.type = ERROR;\n");
        if( hasIf ) {
            writeBuffer( buffer, "\t\trollback = 1;\n");
            writeBuffer( buffer, "\t");
        }

        writeBuffer( buffer, "\tgoto sE;\n");
        if( hasIf )
            writeBuffer( buffer, "\t}\n");
    }
}

void genScannerInitDeinit(Buffer* buffer) {
    writeBuffer( buffer, "#include <cstdio>\n\n");
    writeBuffer( buffer, "Scanner::Scanner(const char* fileName) {\n");
    writeBuffer( buffer, "\tif( mFile == NULL ) {\n");
    writeBuffer( buffer, "\t\tprintf(\"Failed to open %%s\\n\", fileName );\n");
    writeBuffer( buffer, "\t\texit(5);\n\t}\n");
    writeBuffer( buffer, "\tmCurrentFile = fileName;\n\tmLine = 0;\n\tmCurrentPos = 0;\n");
    writeBuffer( buffer, "\tmForward = &mBuffer[1][0];\n\tmBuffer[0][bufferSize] = \'\\0\';\n\tmBuffer[1][bufferSize] = \'\\0\';\n");
    writeBuffer( buffer, "\tthis->fillBuffer();\n\tmLexemmeBegin = &mBuffer[1][0];\n\tmNextToken = this->nextToken();\n}\n\n");
    writeBuffer( buffer, "Scanner::~Scanner() {\n\tif( mFile != NULL )\n\t\tfclose( mFile );\n}\n\n");
}

void genScannerNextChar(Buffer* buffer) {
    writeBuffer( buffer, "uint8_t Scanner::nextChar() {\n");
    writeBuffer( buffer, "\tuint8_t character = *mForward;\n");
    writeBuffer( buffer, "\tif( character == \'\\0\' ) {\n");
    writeBuffer( buffer, "\t\tif( mForward == &mBuffer[0][bufferSize] || mForward == &mBuffer[1][bufferSize] ) {\n");
    writeBuffer( buffer, "\t\t\tthis->fillBuffer();\n");
    writeBuffer( buffer, "\t\t\treturn this->nextChar();\n");
    writeBuffer( buffer, "\t\t} else {\n");
    writeBuffer( buffer, "\t\t\tif( feof( mFile ) ) {\n");
    writeBuffer( buffer, "\t\t\t\tmForward++;\n");
    writeBuffer( buffer, "\t\t\t\treturn character;\n");
    writeBuffer( buffer, "\t\t\t}\n\t\t}\n\t} else if( character == \'\\n\' ) {\n");
    writeBuffer( buffer, "\t\tmLine++;\n\t}\n");
    writeBuffer( buffer, "\tmForward++;\n\tmCurrentPos++;\n\treturn character;\n}\n\n");
}

void genScannerRollback(Buffer* buffer) {
    writeBuffer( buffer, "void Scanner::rollback(size_t numChars) {\n");
    writeBuffer( buffer, "\tfor( uint32_t index = 0; index < numChars; index++ ) {\n");
    writeBuffer( buffer, "\t\tif( *mForward == \'\\0\' && (\n");
    writeBuffer( buffer, "\t\t\tmForward == &mBuffer[0][bufferSize] || mForward == &mBuffer[1][bufferSize]\n");
    writeBuffer( buffer, "\t\t)\n\t\t) {\n\t\t\t--mForward;\n");
    writeBuffer( buffer, "\t\t} else if( *mForward == \'\\n\' ) {\n");
    writeBuffer( buffer, "\t\t\tmLine--;\n\t\t}\n\t}\n}\n\n");
}

void genTokenGetFuncs(Buffer* buffer, Map* map) {
    writeBuffer( buffer, "Token Scanner::peek() {\n");
    writeBuffer( buffer, "\treturn mNextToken;\n}\n\n");
    writeBuffer( buffer, "Token Scanner::next() {\n");
    writeBuffer( buffer, "\tToken ret = mNextToken;\n\tmNextToken = this->nextToken();\n");
    bool foundDiscard = false;
    for( uint32_t index = 0; index < map->numPairs; index++ ) {
        if( map->pairs[index].shouldDiscard ) {
            if( !foundDiscard ) {
                writeBuffer( buffer, "\twhile( mNextToken != %s", map->pairs[index].type);
                foundDiscard = true;
            } else {
                writeBuffer( buffer, " && mNextToken != %s", map->pairs[index].type );
            }
        }
    }
    if( foundDiscard ) {
        writeBuffer( buffer, " ) {\n\t\tmNextToken = this->nextToken();\n\t}\n" );
    }
    writeBuffer( buffer, "\treturn ret;\n}\n\n" );
    writeBuffer( buffer, "Token Scanner::consume(TokenType type) {\n");
    writeBuffer( buffer, "\tif( this->peek().type != type ) {\n");
    writeBuffer( buffer, "\t\treturn Token { ERROR, NULL, mCurrentPos, mLine };\n");
    writeBuffer( buffer, "\t}\n\treturn this->next();\n}\n\n");
}

void writeScannerBoilerplate(Buffer* buffer, Map* map) {
    genScannerInitDeinit( buffer );
    genScannerNextChar( buffer );
    genScannerRollback( buffer );
    genTokenGetFuncs( buffer, map );
}

void genNextToken(Buffer* buffer, Node* node, Map* map) {
    writeBuffer( buffer, "Token Scanner::nextToken() {\n\tToken token;\n\tchar ch;\n\tuint32_t rollback = 0;\n\tgoto s0;\n");

    NodePtrVector vec = getDepthFirstList( true, node );

    for( uint32_t nodeIndex = 0; nodeIndex < vec.size; nodeIndex++ ) {
        buildNode( buffer, getNodePtrVector( &vec, nodeIndex ), map );
    }

    writeBuffer( 
        buffer,
        "sE:\tthis->rollback(rollback);\n\t" \
        "token.line = mLine;\n\t" \
        "token.charPos = mCurrentPos;\n\t" \
        "token.str = (char*) malloc( sizeof(char) * (mForward - mLexemmeBegin + 1) );\n\t"
        "memcpy( token.str, mLexemmeBegin, sizeof(char) * (mForward - mLexemmeBegin) );\n\t" \
        "token.str[mForward - mLexemmeBegin] = \'\\0\';\n\t" \
        "mLexemmeBegin = mForward;\n\treturn token;\n"
    );

    writeBuffer( 
        buffer,
        "accept:\n\tthis->rollback(rollback);\n\t" \
        "token.line = mLine;\n\t" \
        "token.charPos = mCurrentPos;\n\t" \
        "token.str = (char*) malloc( sizeof(char) * (mForward - mLexemmeBegin + 1) );\n\t"
        "memcpy( token.str, mLexemmeBegin, sizeof(char) * (mForward - mLexemmeBegin) );\n\t" \
        "token.str[mForward - mLexemmeBegin] = \'\\0\';\n\t" \
        "mLexemmeBegin = mForward;\n\treturn token;\n}\n\n"
    );
}

void genScannerDef(Buffer* buffer, Node* node, Map* map) {
    writeScannerBoilerplate( buffer, map );
    genNextToken( buffer, node, map );
}

void genTokenToString(Buffer* buffer, Map* map) {
    writeBuffer( buffer, "const char* toString( TokenType type ) {\n");
    writeBuffer( buffer, "\tswitch( type ) {\n");
    for( uint32_t index = 0; index < map->numPairs; index++ ) {
        writeBuffer( buffer, "\t\tcase %s:\n", map->pairs[index].type);
        writeBuffer( buffer, "\t\t\treturn \"%s\";\n", map->pairs[index].type);
    }
    writeBuffer( buffer, "\t\tcase %s:\n", "ERROR");
    writeBuffer( buffer, "\t\t\treturn \"%s\";\n", "ERROR");

    writeBuffer( buffer, "\t}\n\treturn NULL;\n}\n\n");
}

void writeScanner( Node* node, Map* map, const char* fileName ) {
    sortNodeTreeConnections( node );
    renumberNodes( node );

    #ifdef DEBUG
    printNodeTree( true, node );
    #endif

    char* fileNameMod = (char*) malloc( sizeof(char) * (strlen(fileName) + 3) );
    memcpy( fileNameMod, fileName, strlen( fileName ) );
    memcpy( fileNameMod + strlen( fileName ), ".cpp", 5 );

    Buffer buffer;
    initBuffer( &buffer, fileNameMod );

    genScannerDef( &buffer, node, map );

    genTokenToString( &buffer, map );

    clearBuffer( &buffer );
    closeFile( &buffer );

    fileNameMod[strlen(fileName)+1] = 'h';
    initBuffer( &buffer, fileNameMod );

    writeBuffer( &buffer, "#ifndef DFA_TOKENS\n#define DFA_TOKENS\n\n#include <cstdint>\n#include <cstddef>\n\n");

    writeBuffer( &buffer, "enum TokenType {\n");

    for( uint32_t index = 0; index < map->numPairs; index++ ) {
        writeBuffer( &buffer, "\t%s,\n", map->pairs[index].type );
    }

    writeBuffer( &buffer, "};\n\n");

    writeBuffer( &buffer, 
        "struct Token {\n\tTokenType type;\n\tchar* str; \
        \n\tsize_t charPos;\n\tuint32_t line;\n};\n"
    );

    writeBuffer( &buffer, "\n#endif");

    clearBuffer( &buffer );
    closeFile( &buffer );
}
