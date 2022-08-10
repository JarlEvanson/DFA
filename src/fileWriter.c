#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "node.h"

#include "vector.h"
#include "tokens.h"
#include "map.h"
#include "buffer.h"

static void printChar( Buffer* buffer, uint8_t character ) {
    if( character >= 33 && character <= 126 && ( character != '\\' && character != '\'' ) ) {
        writeBuffer( buffer, "%c", character );
    } else {
        switch( character ) {
            case '\n':
                writeBuffer( buffer, "\\n" );
                break;
            case '\r':
                writeBuffer( buffer, "\\r" );
                break;
            case '\t':
                writeBuffer( buffer, "\\t" );
                break;
            case '\\':
                writeBuffer( buffer, "\\\\" );
                break;
            case '\'':
                writeBuffer( buffer, "\\\'" );
                break;
            default:
                writeBuffer( buffer, "\\%o", character );
                break;
        }
    }
}

void buildNode( Buffer* buffer, Node* node, Map* map ) {
    writeBuffer( buffer, "s%u:\n\t", getNodeState( node ) );

    bool hasIf = false;
    if( getNodeState( node ) == 0 ) {
        writeBuffer( buffer, "ch = nextChar( scanner );\n" );
        writeBuffer( buffer, "\tif( ch == \'\\0\' ) {\n" );
        writeBuffer( buffer, "\t\ttoken.type = END_OF_FILE;\n\t\tgoto accept;\n" );

        hasIf = true;
    }


    for( uint32_t connIndex = 0; connIndex < getNodeConnectionCount( node ); ) {
        if( !hasIf ) {
            writeBuffer( buffer, "ch = nextChar( scanner );\n" );
        }
        writeBuffer( buffer, "\t" );
        if( hasIf ) {
            writeBuffer( buffer, "} else " );
        }
        uint32_t numSameConnectee = consecutiveSameNodeConnections( node, connIndex );
        if( numSameConnectee < 4 ) {
            writeBuffer( buffer, "if( ch == \'" );
            Edge* edge = getNthConnection( node, connIndex );
            printChar( buffer, getEdgeKey( edge ) );
            writeBuffer( buffer, "\' ) {\n\t\tgoto s%u;\n", getNodeState( getEdgeConnectee( edge ) ) );
            connIndex++;
        } else {
            writeBuffer( buffer, "if( ch >= \'" );
            Edge* edge1 = getNthConnection( node, connIndex );
            printChar( buffer, getEdgeKey( edge1 ) );
            writeBuffer( buffer, "\' && ch <= \'" );
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
            writeBuffer( buffer, "\t} else {\n\t\t" );
        }
        uint32_t highestPrecedence = 0;
        const char* string = NULL;
        for( uint32_t stringIndex = 0; stringIndex < numString; stringIndex++ ) {
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
            writeBuffer( buffer, "\t\trollbackSize = 1;\n" );
            writeBuffer( buffer, "\t" );
        }
        writeBuffer( buffer, "\tgoto accept;\n" );
        if( hasIf )
            writeBuffer( buffer, "\t}\n" );
    } else {
        if( hasIf ) {
            writeBuffer( buffer, "\t} else {\n\t\t" );
        }

        writeBuffer( buffer, "token.type = ERROR;\n" );
        if( hasIf ) {
            writeBuffer( buffer, "\t\trollbackSize = 1;\n" );
            writeBuffer( buffer, "\t" );
        }

        writeBuffer( buffer, "\tgoto sE;\n" );
        if( hasIf )
            writeBuffer( buffer, "\t}\n" );
    }
}

void genScannerInitDeinit( Buffer* buffer ) {
    writeBuffer( buffer, "void initScanner( Scanner* scanner, const char* fileName ) {\n" );
    writeBuffer( buffer, "\tscanner->mFile = fopen( fileName, \"r\" );\n" );
    writeBuffer( buffer, "\tif( scanner->mFile == NULL ) {\n" );
    writeBuffer( buffer, "\t\tprintf(\"Failed to open %%s\\n\", fileName );\n" );
    writeBuffer( buffer, "\t\texit(5);\n\t}\n" );
    writeBuffer( buffer, "\tscanner->mCurrentFile = fileName;\n\tscanner->mLine = 0;\n\tscanner->mCurrentPos = 0;\n" );
    writeBuffer( buffer, "\tscanner->mForward = &scanner->mBuffer[1][0];\n\tscanner->mBuffer[0][bufferSize] = \'\\0\';\n\tscanner->mBuffer[1][bufferSize] = \'\\0\';\n" );
    writeBuffer( buffer, "\tfillBuffer( scanner );\n\tscanner->mLexemmeBegin = &scanner->mBuffer[1][0];" );
    writeBuffer( buffer, "\n\tnext( scanner );\n}\n\n" );
    writeBuffer( buffer, "void freeScanner( Scanner* scanner ) {\n\tif( scanner->mFile != NULL )\n\t\tfclose( scanner->mFile );\n}\n\n" );
}

void genScannerNextChar( Buffer* buffer ) {
    writeBuffer( buffer, "uint8_t nextChar( Scanner* scanner ) {\n" );
    writeBuffer( buffer, "\tuint8_t character = *scanner->mForward;\n" );
    writeBuffer( buffer, "\tif( character == \'\\0\' ) {\n" );
    writeBuffer( buffer, "\t\tif( scanner->mForward == &scanner->mBuffer[0][bufferSize] || scanner->mForward == &scanner->mBuffer[1][bufferSize] ) {\n" );
    writeBuffer( buffer, "\t\t\tfillBuffer( scanner );\n" );
    writeBuffer( buffer, "\t\t\treturn nextChar( scanner );\n" );
    writeBuffer( buffer, "\t\t} else {\n" );
    writeBuffer( buffer, "\t\t\tif( feof( scanner->mFile ) ) {\n" );
    writeBuffer( buffer, "\t\t\t\tscanner->mForward++;\n" );
    writeBuffer( buffer, "\t\t\t\treturn character;\n" );
    writeBuffer( buffer, "\t\t\t}\n\t\t}\n\t} else if( character == \'\\n\' ) {\n" );
    writeBuffer( buffer, "\t\tscanner->mLine++;\n\t}\n" );
    writeBuffer( buffer, "\tscanner->mForward++;\n\tscanner->mCurrentPos++;\n\treturn character;\n}\n\n" );
}

void genScannerFillBuffer( Buffer* buffer ) {
    writeBuffer( buffer, "void fillBuffer( Scanner* scanner ) {\n" );
    writeBuffer( buffer, "\tmemcpy( scanner->mBuffer, &scanner->mBuffer[1][0], bufferSize );\n" );
    writeBuffer( buffer, "\tsize_t objectsRead = fread( &scanner->mBuffer[1][0], sizeof(uint8_t), bufferSize, scanner->mFile );\n" );
    writeBuffer( buffer, "\tif( objectsRead != bufferSize ) {\n\t\tif( feof( scanner->mFile ) ) {\n" );
    writeBuffer( buffer, "\t\t\tscanner->mBuffer[1][objectsRead] = \'\\0\';\n" );
    writeBuffer( buffer, "\t\t\tscanner->mForward = &scanner->mBuffer[1][0];\n\t\t\tscanner->mLexemmeBegin -= bufferSize;\n" );
    writeBuffer( buffer, "\t\t} else if( ferror( scanner->mFile ) ) {\n" );
    writeBuffer( buffer, "\t\t\tprintf(\"Failed to read from %%s\\n\", scanner->mCurrentFile);\n" );
    writeBuffer( buffer, "\t\t\texit( 5 );\n\t\t}\n\t} else {\n" );
    writeBuffer( buffer, "\t\tscanner->mForward = &scanner->mBuffer[1][0];\n" );
    writeBuffer( buffer, "\t\tscanner->mLexemmeBegin -= bufferSize;\n\t}\n}\n\n" );
}

void genScannerRollback( Buffer* buffer ) {
    writeBuffer( buffer, "void rollback( Scanner* scanner, size_t numChars ) {\n" );
    writeBuffer( buffer, "\tfor( uint32_t index = 0; index < numChars; index++ ) {\n" );
    writeBuffer( buffer, "\n\t\tscanner->mForward--;\n\t\tscanner->mCurrentPos--;\n" );
    writeBuffer( buffer, "\t\tif( *scanner->mForward == \'\\0\' && (\n" );
    writeBuffer( buffer, "\t\t\tscanner->mForward == &scanner->mBuffer[0][bufferSize] || scanner->mForward == &scanner->mBuffer[1][bufferSize]\n" );
    writeBuffer( buffer, "\t\t)\n\t\t) {\n\t\t\t--scanner->mForward;\n" );
    writeBuffer( buffer, "\t\t} else if( *scanner->mForward == \'\\n\' ) {\n" );
    writeBuffer( buffer, "\t\t\tscanner->mLine--;\n\t\t}\n\t}\n}\n\n" );
}

void genTokenGetFuncs( Buffer* buffer, Map* map ) {
    writeBuffer( buffer, "Token peek( Scanner* scanner ) {\n" );
    writeBuffer( buffer, "\treturn scanner->mNextToken;\n}\n\n" );
    writeBuffer( buffer, "Token next( Scanner* scanner ) {\n" );
    writeBuffer( buffer, "\tToken ret = scanner->mNextToken;\n\tscanner->mNextToken = nextToken( scanner );\n" );
    bool foundDiscard = false;
    for( uint32_t index = 0; index < map->numPairs; index++ ) {
        if( map->pairs[index].shouldDiscard ) {
            if( !foundDiscard ) {
                writeBuffer( buffer, "\twhile( scanner->mNextToken.type == %s", map->pairs[index].type );
                foundDiscard = true;
            } else {
                writeBuffer( buffer, " || scanner->mNextToken.type == %s", map->pairs[index].type );
            }
        }
    }
    if( foundDiscard ) {
        writeBuffer( buffer, " ) {\n\t\tscanner->mNextToken = nextToken( scanner );\n\t}\n" );
    }
    writeBuffer( buffer, "\treturn ret;\n}\n\n" );
    writeBuffer( buffer, "Token consume( Scanner* scanner, TokenType type ) {\n" );
    writeBuffer( buffer, "\tif( peek( scanner ).type != type ) {\n" );
    writeBuffer( buffer, "\t\tToken tok;\n\t\ttok.type = ERROR;\n\t\ttok.str = NULL;\n\t\ttok.charPos = scanner->mCurrentPos;\n\t\ttok.line = scanner->mLine;\n" );
    writeBuffer( buffer, "\t\treturn tok;\n" );
    writeBuffer( buffer, "\t}\n\treturn next( scanner );\n}\n\n" );
}

void genNextToken( Buffer* buffer, Node* node, Map* map ) {
    writeBuffer( buffer, "Token nextToken( Scanner* scanner ) {\n\tToken token;\n\tchar ch;\n\tuint32_t rollbackSize = 0;\n\tgoto s0;\n" );

    NodePtrVector vec = getDepthFirstList( true, node );

    for( uint32_t nodeIndex = 0; nodeIndex < vec.size; nodeIndex++ ) {
        buildNode( buffer, getNodePtrVector( &vec, nodeIndex ), map );
    }

    writeBuffer(
        buffer,
        "sE:\trollback( scanner, rollbackSize );\n\t" \
        "token.line = scanner->mLine;\n\t" \
        "token.charPos = scanner->mCurrentPos;\n\t" \
        "token.str = (char*) malloc( sizeof(char) * (scanner->mForward - scanner->mLexemmeBegin + 1) );\n\t"
        "memcpy( token.str, scanner->mLexemmeBegin, sizeof(char) * (scanner->mForward - scanner->mLexemmeBegin) );\n\t" \
        "token.str[scanner->mForward - scanner->mLexemmeBegin] = \'\\0\';\n\t" \
        "scanner->mLexemmeBegin = scanner->mForward;\n\treturn token;\n"
    );

    writeBuffer(
        buffer,
        "accept:\n\trollback( scanner, rollbackSize );\n\t" \
        "token.line = scanner->mLine;\n\t" \
        "token.charPos = scanner->mCurrentPos;\n\t" \
        "token.str = (char*) malloc( sizeof(char) * (scanner->mForward - scanner->mLexemmeBegin + 1) );\n\t"
        "memcpy( token.str, scanner->mLexemmeBegin, sizeof(char) * (scanner->mForward - scanner->mLexemmeBegin) );\n\t" \
        "token.str[scanner->mForward - scanner->mLexemmeBegin] = \'\\0\';\n\t" \
        "scanner->mLexemmeBegin = scanner->mForward;\n\treturn token;\n}\n\n"
    );
}

void genScannerDef( Buffer* buffer, Node* node, Map* map ) {
    writeBuffer( buffer, "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n" );

    writeBuffer( buffer, "void fillBuffer();\nvoid rollback( Scanner* scanner, size_t numChars );\n" );
    writeBuffer( buffer, "uint8_t nextChar(Scanner* scanner);\nToken nextToken( Scanner* scanner );\n\n" );
    
    writeBuffer( buffer, "const size_t bufferSize = 4096;\nconst size_t bufferSizeSentinal = 4097;\n\n" );

    genScannerInitDeinit( buffer );
    genScannerFillBuffer( buffer );
    genScannerNextChar( buffer );
    genScannerRollback( buffer );
    genTokenGetFuncs( buffer, map );

    genNextToken( buffer, node, map );
}

void writeScannerHeader( Buffer* buffer ) {
    writeBuffer( buffer, "#ifndef DFA_SCANNER\n#define DFA_SCANNER\n\n" );
    writeBuffer( buffer, "#include <stdint.h>\n#include <stddef.h>\n#include <stdio.h>\n\n" );
    writeBuffer( buffer, "#include \"%s\"\n\n", "token.h" );
    writeBuffer( buffer, "\ntypedef struct Scanner {\n\tFILE* mFile;\n" );
    writeBuffer( buffer, "\tconst char* mCurrentFile;\n\tuint8_t mBuffer[2][4097];\n" );
    writeBuffer( buffer, "\tuint8_t* mLexemmeBegin;\n\tuint8_t* mForward;\n" );
    writeBuffer( buffer, "\tsize_t mCurrentPos;\n\tsize_t mLine;\n\tToken mNextToken;\n} Scanner;\n\n" );
    writeBuffer( buffer, "void initScanner( Scanner* scanner, const char* fileName );\nvoid freeScanner( Scanner* scanner );" );
    writeBuffer( buffer, "\nToken next( Scanner* scanner );\nToken consume( Scanner* scanner, TokenType type );\n" );
    writeBuffer( buffer, "Token peek( Scanner* scanner );\n" );
    writeBuffer( buffer, "\n#endif" );
}

void writeTokenHeader( Buffer* buffer, Map* map ) {
    writeBuffer( buffer, "#ifndef DFA_TOKEN\n#define DFA_TOKEN\n\n#include <stdint.h>\n\n" );
    writeBuffer( buffer, "typedef enum TokenType {\n" );

    for( uint32_t index = 0; index < map->numPairs; index++ ) {
        writeBuffer( buffer, "\t%s,\n", map->pairs[index].type );
    }

    writeBuffer( buffer, "\tERROR,\n\tEND_OF_FILE\n" );

    writeBuffer( buffer, "} TokenType;\n\n" );

    writeBuffer( buffer,
        "typedef struct Token {\n\tTokenType type;\n\tchar* str; \
        \n\tsize_t charPos;\n\tsize_t line;\n} Token;\n"
    );

    writeBuffer( buffer, "\nconst char* toString(TokenType type);\n" );
    writeBuffer( buffer, "#endif" );
}

void writeTokenDeclaration( Buffer* buffer, Map* map, const char* str ) {
    writeBuffer( buffer, "#include <stddef.h>\n\n#include \"%s\"\n\n", str );
    writeBuffer( buffer, "const char* toString( TokenType type ) {\n" );
    writeBuffer( buffer, "\tswitch( type ) {\n" );
    for( uint32_t index = 0; index < map->numPairs; index++ ) {
        writeBuffer( buffer, "\t\tcase %s:\n", map->pairs[index].type );
        writeBuffer( buffer, "\t\t\treturn \"%s\";\n", map->pairs[index].type );
    }
    writeBuffer( buffer, "\t\tcase %s:\n", "ERROR" );
    writeBuffer( buffer, "\t\t\treturn \"%s\";\n", "ERROR" );
    writeBuffer( buffer, "\t\tcase %s:\n", "END_OF_FILE" );
    writeBuffer( buffer, "\t\t\treturn \"%s\";\n", "END_OF_FILE" );

    writeBuffer( buffer, "\t}\n\treturn NULL;\n}\n\n" );
}

void writeScanner( Node* node, Map* map, const char* fileName ) {
    sortNodeTreeConnections( node );
    renumberNodes( node );

    #ifdef DEBUG
    printNodeTree( true, node );
    #endif

    char* codeLoc = (char*) malloc( sizeof( char ) * ( strlen( fileName ) + 3 ) );
    memcpy( codeLoc, fileName, strlen( fileName ) );
    memcpy( codeLoc + strlen( fileName ), ".c", 3 );

    Buffer buffer;
    initBuffer( &buffer, codeLoc );

    uint32_t currPos = 0;
    for( uint32_t index = 0; index < strlen( codeLoc ); index++ ) {
        if( *( codeLoc + index ) == '/' ) {
            currPos = index + 1;
        }
    }

    codeLoc[strlen( fileName ) + 1] = 'h';
    writeBuffer( &buffer, "#include \"%s\"\n", &codeLoc[currPos] );
    writeBuffer( &buffer, "#include \"token.h\"\n\n" );

    genScannerDef( &buffer, node, map );

    clearBuffer( &buffer );
    closeFile( &buffer );

    initBuffer( &buffer, codeLoc );

    writeScannerHeader( &buffer );

    clearBuffer( &buffer );
    closeFile( &buffer );

    char* tokH = "token.h";
    char* tokC = "token.c";

    initBuffer( &buffer, tokH );

    writeTokenHeader( &buffer, map );

    clearBuffer( &buffer );
    closeFile( &buffer );

    initBuffer( &buffer, tokC );

    writeTokenDeclaration( &buffer, map, tokH );

    clearBuffer( &buffer );
    closeFile( &buffer );
}
