#include <stdio.h>

#include "common.h"
#include "regex.h"
#include "fileWriter.h"
#include "map.h"
#include "scanner.h"
#include "vector.h"

#include <stdlib.h>
#include <string.h>

#pragma region 

static void printChar(uint8_t character ) {
    if( character >= 33 && character <= 126 && (character != '\\' && character != '\'') ) {
        printf("%c", character );
    } else {
        switch( character ) {
            case '\n':
                printf("\\n");
                break;
            case '\r':
                printf("\\r");
                break;
            case '\t':
                printf("\\t");
                break;
            case '\\':
                printf("\\\\");
                break;
            case '\'':
                printf("\\\'");
                break;
            default:
                printf("\\%o", character);
                break;
        }
    }
}

#pragma endregion

DECLARE_VECTOR(Token, Token);
DEFINE_VECTOR(Token, Token);

int main(int argc, char const *argv[]) {
    Map map;
    if( argc == 3 ) {
        Scanner scanner;
        initScanner( &scanner, argv[2] );

        Token tok = nextToken( &scanner );
        while( 1 ) {
            if( tok.type == WHITESPACE || tok.type == COMMENT ) {
                tok = nextToken( &scanner );
            } else {
                break;
            }
        }

        TokenVector tokVec = initTokenVector( 100 );

        while( tok.type != ERROR ) {
            pushTokenVector( &tokVec, tok );
            tok = nextToken( &scanner );
            while( 1 ) {
                if( tok.type == WHITESPACE || tok.type == COMMENT ) {
                    tok = nextToken( &scanner );
                } else {
                    break;
                }
            }
        }
        
        for( uint32_t index = 0; index < tokVec.size; index++ ) {
            printf("%s ", getTokenVector( &tokVec, index ).str );
            if( index % 3 == 2 )
               printf("\n"); 
        }
        
        printf("\n");

        if( tokVec.size % 3 != 0 ) {
            printf("Invalid file\n");
            exit(1);
        }

        map.numPairs = tokVec.size / 3;
        map.pairs = (Pair*) calloc( map.numPairs, sizeof(Pair) );

        for( uint32_t pairIndex = 0; pairIndex < map.numPairs; pairIndex++ ) {
            sscanf( popTokenVector( &tokVec ).str, "%d", &map.pairs[pairIndex].precedence );
            map.pairs[pairIndex].type = popTokenVector( &tokVec ).str;
            map.pairs[pairIndex].key = popTokenVector( &tokVec ).str;
            printf("Pair { key: \"%s\", type: %s, precedence: %d }\n", map.pairs[pairIndex].key, map.pairs[pairIndex].type, map.pairs[pairIndex].precedence);
        }

    } else if( (argc - 2) % 3 == 0 && argc != 2 ) {
        map.numPairs = (argc - 1) / 3;
        map.pairs = (Pair*) calloc( map.numPairs, sizeof(Pair) );
        const char** args = (const char**) calloc( map.numPairs, sizeof(char*) );

        for( uint32_t index = 0; index < map.numPairs; index++ ) {
            args[index] = argv[(index * 3) + 2]; 
            map.pairs[index].key = argv[(index * 3) + 2];
            map.pairs[index].type = argv[(index * 3 + 2) + 1];
            sscanf( argv[(index * 3 + 2) + 1], "%d", &map.pairs[index].precedence );
            printf("Pair { key: \"%s\", type: %s, precedence: %d }\n", map.pairs[index].key, map.pairs[index].type, map.pairs[index].precedence);
        }
    } else {
        printf("Invalid Args\n,/DFA {outFileName} {regex} {tokenName} {precedence} ...\n./DFA {outFileName} {inFileName}\n");
        return 1;
    }

    Node* root = buildRegexSet( &map );

    writeScanner( root, &map, argv[1] );

    return 0;
}
