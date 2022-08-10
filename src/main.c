#include <stdio.h>

#include "common.h"
#include "regex.h"
#include "fileWriter.h"
#include "map.h"
#include "scanner.h"
#include "vector.h"
#include "tokens.h"

#include <stdlib.h>
#include <string.h>

DECLARE_VECTOR( Token, Token );
DEFINE_VECTOR( Token, Token );

DECLARE_VECTOR( Pair, Pair );
DEFINE_VECTOR( Pair, Pair );

int main( int argc, char const* argv[] ) {
    Map map;
    if( argc == 3 ) {
        Scanner scanner;
        initScanner( &scanner, argv[2] );

        Token tok = next( &scanner );

        TokenVector tokVec = initTokenVector( 100 );

        while( tok.type != ERROR && tok.type != END_OF_FILE ) {
            pushTokenVector( &tokVec, tok );
            tok = next( &scanner );
        }

        pushTokenVector( &tokVec, tok );
        PairVector pairVec = initPairVector( 10 );
        for( uint32_t index = 0; index < tokVec.size; ) {
            Pair p;
            TokenVector vec = initTokenVector( 10 );

            Token tok = getTokenVector( &tokVec, index );
            index++;

            while( tok.type != WHITESPACE ) {
                pushTokenVector( &vec, tok );
                tok = getTokenVector( &tokVec, index );
                index++;
            }

            shrinkTokenVector( &vec );
            if( vec.size == 0 ) {
                printf( "Invalid Format\n" );
                exit( 1 );
            }

            p.toks = vec.vec;
            p.tokCount = vec.size;

            tok = getTokenVector( &tokVec, index );
            index++;
            if( tok.type != IDENTIFIER && tok.type != DISCARD ) {
                printf( "Invalid Format\n" );
                exit( 1 );
            }

            p.type = tok.str;

            if( getTokenVector( &tokVec, index ).type != WHITESPACE ) {
                printf( "Invalid Format\n" );
                exit( 1 );
            }
            index++;

            tok = getTokenVector( &tokVec, index );
            index++;
            if( tok.type != NUMBER ) {
                printf( "Invalid Format\n" );
                exit( 1 );
            }

            p.precedence = atoi( tok.str );

            if(
                getTokenVector( &tokVec, index ).type == WHITESPACE &&
                getTokenVector( &tokVec, index + 1 ).type == DISCARD
                ) {
                p.shouldDiscard = true;
                index += 2;
            } else {
                p.shouldDiscard = false;
            }

            pushPairVector( &pairVec, p );

            while( getTokenVector( &tokVec, index ).type == WHITESPACE ) {
                index++;
            }

            if(
                getTokenVector( &tokVec, index ).type != NEWLINE &&
                getTokenVector( &tokVec, index ).type != END_OF_FILE
                ) {
                printf( "Invalid Format\n" );
                exit( 1 );
            }
            index++;
        }

        shrinkPairVector( &pairVec );
        map.numPairs = pairVec.size;
        map.pairs = pairVec.vec;

    } else {
        printf( "Invalid Args\n./DFA {outFileName} {inFileName}\n" );
        return 1;
    }

    for( uint32_t index = 0; index < map.numPairs; index++ ) {
        printf( "%s: %d\n", map.pairs[index].type, map.pairs[index].precedence );
    }

    Node* root = buildRegexSet( &map );

    writeScanner( root, &map, argv[1] );

    return 0;
}
