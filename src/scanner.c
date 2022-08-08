
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "tokens.h"
#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t bufferSize = 4096;
const size_t bufferSizeSentinal = 4097;

static void fillBuffer(Scanner* scanner);
static void rollback(Scanner* scanner, size_t numChars);
static uint8_t nextChar(Scanner* scanner);

void initScanner(Scanner* scanner, const char* fileName) {
    scanner->mFile = fopen( fileName, "r" );
    if( scanner->mFile == NULL ) {
        printf( "Failed to open %s\n", fileName );
        exit(5);
    }
    scanner->mCurrentFile = fileName;
    scanner->mLine = 0;
    scanner->mCurrentPos = 0;
    scanner->mForward = &scanner->mBuffer[1][0];
    scanner->mBuffer[0][bufferSize] = '\0';
    scanner->mBuffer[1][bufferSize] = '\0';
    fillBuffer( scanner );
    scanner->mLexemmeBegin = &scanner->mBuffer[1][0];
}

void fillBuffer(Scanner* scanner) {
    //Copy the entire second buffer, excluding sentinal values, to the beginning of the first buffer
    memcpy( scanner->mBuffer, &scanner->mBuffer[1][0], bufferSize );

    size_t objectsRead = fread( &scanner->mBuffer[1][0], sizeof( uint8_t ), bufferSize, scanner->mFile );
    if( objectsRead != bufferSize ) {
        if( feof( scanner->mFile ) ) {
            scanner->mBuffer[1][objectsRead] = '\0';
            scanner->mForward = &scanner->mBuffer[1][0];
            scanner->mLexemmeBegin -= bufferSize;
        } else if( ferror( scanner->mFile ) ) {
            printf("Failed to read from %s\n", scanner->mCurrentFile);
            exit( 5 );
        }
    } else {
        scanner->mForward = &scanner->mBuffer[1][0];
        scanner->mLexemmeBegin -= bufferSize;
    }
}

uint8_t nextChar(Scanner* scanner) {
    uint8_t character = *scanner->mForward;
    if( character == '\0' ) {
        if( 
            scanner->mForward == &scanner->mBuffer[0][bufferSize] || 
            scanner->mForward == &scanner->mBuffer[1][bufferSize] 
        ) {
            fillBuffer( scanner );
            return nextChar( scanner );
        } else {
            if( feof( scanner->mFile ) ) {
				scanner->mForward++;
                return character;
            }
        }
    } else if( character == '\n' ) {
        scanner->mLine++;
    }
    scanner->mForward++;
    scanner->mCurrentPos++;
    return character;
}

void rollback(Scanner* scanner, size_t numChars) {
    for( uint32_t index = 0; index < numChars; index++ ) {
        --scanner->mCurrentPos;
        --scanner->mForward;
        if( 
			*scanner->mForward == '\0' && 
			(
				scanner->mForward == &scanner->mBuffer[0][bufferSize] || 
            	scanner->mForward == &scanner->mBuffer[1][bufferSize] 
			) 
		) {
            --scanner->mForward;
        } else if( *scanner->mForward == '\n' ) {
            scanner->mLine--;
        }
    }
}

Token nextToken(Scanner* scanner) {
	Token token;
	char ch;
	uint32_t rollbackSize = 0;
	goto s0;
s0:	ch = nextChar( scanner );
	if( ch == '\t' ) {
		goto s1;
	} else if( ch == '\n' ) {
		goto s1;
	} else if( ch == '\r' ) {
		goto s1;
	} else if( ch == '\40' ) {
		goto s1;
	} else if( ch == '!' ) {
		goto s3;
	} else if( ch == '"' ) {
		goto s3;
	} else if( ch == '#' ) {
		goto s3;
	} else if( ch == '$' ) {
		goto s3;
	} else if( ch == '%' ) {
		goto s3;
	} else if( ch == '&' ) {
		goto s3;
	} else if( ch == '\'' ) {
		goto s3;
	} else if( ch == '(' ) {
		goto s3;
	} else if( ch == ')' ) {
		goto s3;
	} else if( ch == '*' ) {
		goto s3;
	} else if( ch == '+' ) {
		goto s3;
	} else if( ch == ',' ) {
		goto s3;
	} else if( ch == '-' ) {
		goto s3;
	} else if( ch == '.' ) {
		goto s3;
	} else if( ch == '/' ) {
		goto s4;
	} else if( ch == '0' ) {
		goto s7;
	} else if( ch == '1' ) {
		goto s7;
	} else if( ch == '2' ) {
		goto s7;
	} else if( ch == '3' ) {
		goto s7;
	} else if( ch == '4' ) {
		goto s7;
	} else if( ch == '5' ) {
		goto s7;
	} else if( ch == '6' ) {
		goto s7;
	} else if( ch == '7' ) {
		goto s7;
	} else if( ch == '8' ) {
		goto s7;
	} else if( ch == '9' ) {
		goto s7;
	} else if( ch == ':' ) {
		goto s3;
	} else if( ch == ';' ) {
		goto s3;
	} else if( ch == '<' ) {
		goto s3;
	} else if( ch == '=' ) {
		goto s3;
	} else if( ch == '?' ) {
		goto s3;
	} else if( ch == '@' ) {
		goto s3;
	} else if( ch == 'A' ) {
		goto s3;
	} else if( ch == 'B' ) {
		goto s3;
	} else if( ch == 'C' ) {
		goto s3;
	} else if( ch == 'D' ) {
		goto s3;
	} else if( ch == 'E' ) {
		goto s3;
	} else if( ch == 'F' ) {
		goto s3;
	} else if( ch == 'G' ) {
		goto s3;
	} else if( ch == 'H' ) {
		goto s3;
	} else if( ch == 'I' ) {
		goto s3;
	} else if( ch == 'J' ) {
		goto s3;
	} else if( ch == 'K' ) {
		goto s3;
	} else if( ch == 'L' ) {
		goto s3;
	} else if( ch == 'M' ) {
		goto s3;
	} else if( ch == 'N' ) {
		goto s3;
	} else if( ch == 'O' ) {
		goto s3;
	} else if( ch == 'P' ) {
		goto s3;
	} else if( ch == 'Q' ) {
		goto s3;
	} else if( ch == 'R' ) {
		goto s3;
	} else if( ch == 'S' ) {
		goto s3;
	} else if( ch == 'T' ) {
		goto s3;
	} else if( ch == 'U' ) {
		goto s3;
	} else if( ch == 'V' ) {
		goto s3;
	} else if( ch == 'W' ) {
		goto s3;
	} else if( ch == 'X' ) {
		goto s3;
	} else if( ch == 'Y' ) {
		goto s3;
	} else if( ch == 'Z' ) {
		goto s3;
	} else if( ch == '[' ) {
		goto s3;
	} else if( ch == '\\' ) {
		goto s3;
	} else if( ch == ']' ) {
		goto s3;
	} else if( ch == '^' ) {
		goto s3;
	} else if( ch == '_' ) {
		goto s3;
	} else if( ch == '`' ) {
		goto s3;
	} else if( ch == 'a' ) {
		goto s3;
	} else if( ch == 'b' ) {
		goto s3;
	} else if( ch == 'c' ) {
		goto s3;
	} else if( ch == 'd' ) {
		goto s3;
	} else if( ch == 'e' ) {
		goto s3;
	} else if( ch == 'f' ) {
		goto s3;
	} else if( ch == 'g' ) {
		goto s3;
	} else if( ch == 'h' ) {
		goto s3;
	} else if( ch == 'i' ) {
		goto s3;
	} else if( ch == 'j' ) {
		goto s3;
	} else if( ch == 'k' ) {
		goto s3;
	} else if( ch == 'l' ) {
		goto s3;
	} else if( ch == 'm' ) {
		goto s3;
	} else if( ch == 'n' ) {
		goto s3;
	} else if( ch == 'o' ) {
		goto s3;
	} else if( ch == 'p' ) {
		goto s3;
	} else if( ch == 'q' ) {
		goto s3;
	} else if( ch == 'r' ) {
		goto s3;
	} else if( ch == 's' ) {
		goto s3;
	} else if( ch == 't' ) {
		goto s3;
	} else if( ch == 'u' ) {
		goto s3;
	} else if( ch == 'v' ) {
		goto s3;
	} else if( ch == 'w' ) {
		goto s3;
	} else if( ch == 'x' ) {
		goto s3;
	} else if( ch == 'y' ) {
		goto s3;
	} else if( ch == 'z' ) {
		goto s3;
	} else if( ch == '{' ) {
		goto s3;
	} else if( ch == '|' ) {
		goto s3;
	} else if( ch == '}' ) {
		goto s3;
	} else if( ch == '~' ) {
		goto s3;
	} else {
		token.type = ERROR;
		rollbackSize = 1;
		goto sE;
	}
s1:	ch = nextChar( scanner );
	if( ch == '\t' ) {
		goto s1;
	} else if( ch == '\n' ) {
		goto s1;
	} else if( ch == '\r' ) {
		goto s1;
	} else if( ch == '\40' ) {
		goto s1;
	} else {
		token.type = WHITESPACE;
		rollbackSize = 1;
		goto accept;
	}
s2:	ch = nextChar( scanner );
	if( ch == '\t' ) {
		goto s1;
	} else if( ch == '\n' ) {
		goto s1;
	} else if( ch == '\r' ) {
		goto s1;
	} else if( ch == '\40' ) {
		goto s2;
	} else if( ch == '!' ) {
		goto s3;
	} else if( ch == '"' ) {
		goto s3;
	} else if( ch == '#' ) {
		goto s3;
	} else if( ch == '$' ) {
		goto s3;
	} else if( ch == '%' ) {
		goto s3;
	} else if( ch == '&' ) {
		goto s3;
	} else if( ch == '\'' ) {
		goto s3;
	} else if( ch == '(' ) {
		goto s3;
	} else if( ch == ')' ) {
		goto s3;
	} else if( ch == '*' ) {
		goto s3;
	} else if( ch == '+' ) {
		goto s3;
	} else if( ch == ',' ) {
		goto s3;
	} else if( ch == '-' ) {
		goto s3;
	} else if( ch == '.' ) {
		goto s3;
	} else if( ch == '/' ) {
		goto s3;
	} else if( ch == '0' ) {
		goto s3;
	} else if( ch == '1' ) {
		goto s3;
	} else if( ch == '2' ) {
		goto s3;
	} else if( ch == '3' ) {
		goto s3;
	} else if( ch == '4' ) {
		goto s3;
	} else if( ch == '5' ) {
		goto s3;
	} else if( ch == '6' ) {
		goto s3;
	} else if( ch == '7' ) {
		goto s3;
	} else if( ch == '8' ) {
		goto s3;
	} else if( ch == '9' ) {
		goto s3;
	} else if( ch == ':' ) {
		goto s3;
	} else if( ch == ';' ) {
		goto s3;
	} else if( ch == '<' ) {
		goto s3;
	} else if( ch == '=' ) {
		goto s3;
	} else if( ch == '?' ) {
		goto s3;
	} else if( ch == '@' ) {
		goto s3;
	} else if( ch == 'A' ) {
		goto s3;
	} else if( ch == 'B' ) {
		goto s3;
	} else if( ch == 'C' ) {
		goto s3;
	} else if( ch == 'D' ) {
		goto s3;
	} else if( ch == 'E' ) {
		goto s3;
	} else if( ch == 'F' ) {
		goto s3;
	} else if( ch == 'G' ) {
		goto s3;
	} else if( ch == 'H' ) {
		goto s3;
	} else if( ch == 'I' ) {
		goto s3;
	} else if( ch == 'J' ) {
		goto s3;
	} else if( ch == 'K' ) {
		goto s3;
	} else if( ch == 'L' ) {
		goto s3;
	} else if( ch == 'M' ) {
		goto s3;
	} else if( ch == 'N' ) {
		goto s3;
	} else if( ch == 'O' ) {
		goto s3;
	} else if( ch == 'P' ) {
		goto s3;
	} else if( ch == 'Q' ) {
		goto s3;
	} else if( ch == 'R' ) {
		goto s3;
	} else if( ch == 'S' ) {
		goto s3;
	} else if( ch == 'T' ) {
		goto s3;
	} else if( ch == 'U' ) {
		goto s3;
	} else if( ch == 'V' ) {
		goto s3;
	} else if( ch == 'W' ) {
		goto s3;
	} else if( ch == 'X' ) {
		goto s3;
	} else if( ch == 'Y' ) {
		goto s3;
	} else if( ch == 'Z' ) {
		goto s3;
	} else if( ch == '[' ) {
		goto s3;
	} else if( ch == '\\' ) {
		goto s3;
	} else if( ch == ']' ) {
		goto s3;
	} else if( ch == '^' ) {
		goto s3;
	} else if( ch == '_' ) {
		goto s3;
	} else if( ch == '`' ) {
		goto s3;
	} else if( ch == 'a' ) {
		goto s3;
	} else if( ch == 'b' ) {
		goto s3;
	} else if( ch == 'c' ) {
		goto s3;
	} else if( ch == 'd' ) {
		goto s3;
	} else if( ch == 'e' ) {
		goto s3;
	} else if( ch == 'f' ) {
		goto s3;
	} else if( ch == 'g' ) {
		goto s3;
	} else if( ch == 'h' ) {
		goto s3;
	} else if( ch == 'i' ) {
		goto s3;
	} else if( ch == 'j' ) {
		goto s3;
	} else if( ch == 'k' ) {
		goto s3;
	} else if( ch == 'l' ) {
		goto s3;
	} else if( ch == 'm' ) {
		goto s3;
	} else if( ch == 'n' ) {
		goto s3;
	} else if( ch == 'o' ) {
		goto s3;
	} else if( ch == 'p' ) {
		goto s3;
	} else if( ch == 'q' ) {
		goto s3;
	} else if( ch == 'r' ) {
		goto s3;
	} else if( ch == 's' ) {
		goto s3;
	} else if( ch == 't' ) {
		goto s3;
	} else if( ch == 'u' ) {
		goto s3;
	} else if( ch == 'v' ) {
		goto s3;
	} else if( ch == 'w' ) {
		goto s3;
	} else if( ch == 'x' ) {
		goto s3;
	} else if( ch == 'y' ) {
		goto s3;
	} else if( ch == 'z' ) {
		goto s3;
	} else if( ch == '{' ) {
		goto s3;
	} else if( ch == '|' ) {
		goto s3;
	} else if( ch == '}' ) {
		goto s3;
	} else if( ch == '~' ) {
		goto s3;
	} else {
		token.type = WHITESPACE;
		rollbackSize = 1;
		goto accept;
	}
s3:	ch = nextChar( scanner );
	if( ch == '!' ) {
		goto s3;
	} else if( ch == '"' ) {
		goto s3;
	} else if( ch == '#' ) {
		goto s3;
	} else if( ch == '$' ) {
		goto s3;
	} else if( ch == '%' ) {
		goto s3;
	} else if( ch == '&' ) {
		goto s3;
	} else if( ch == '\'' ) {
		goto s3;
	} else if( ch == '(' ) {
		goto s3;
	} else if( ch == ')' ) {
		goto s3;
	} else if( ch == '*' ) {
		goto s3;
	} else if( ch == '+' ) {
		goto s3;
	} else if( ch == ',' ) {
		goto s3;
	} else if( ch == '-' ) {
		goto s3;
	} else if( ch == '.' ) {
		goto s3;
	} else if( ch == '/' ) {
		goto s3;
	} else if( ch == '0' ) {
		goto s3;
	} else if( ch == '1' ) {
		goto s3;
	} else if( ch == '2' ) {
		goto s3;
	} else if( ch == '3' ) {
		goto s3;
	} else if( ch == '4' ) {
		goto s3;
	} else if( ch == '5' ) {
		goto s3;
	} else if( ch == '6' ) {
		goto s3;
	} else if( ch == '7' ) {
		goto s3;
	} else if( ch == '8' ) {
		goto s3;
	} else if( ch == '9' ) {
		goto s3;
	} else if( ch == ':' ) {
		goto s3;
	} else if( ch == ';' ) {
		goto s3;
	} else if( ch == '<' ) {
		goto s3;
	} else if( ch == '=' ) {
		goto s3;
	} else if( ch == '?' ) {
		goto s3;
	} else if( ch == '@' ) {
		goto s3;
	} else if( ch == 'A' ) {
		goto s3;
	} else if( ch == 'B' ) {
		goto s3;
	} else if( ch == 'C' ) {
		goto s3;
	} else if( ch == 'D' ) {
		goto s3;
	} else if( ch == 'E' ) {
		goto s3;
	} else if( ch == 'F' ) {
		goto s3;
	} else if( ch == 'G' ) {
		goto s3;
	} else if( ch == 'H' ) {
		goto s3;
	} else if( ch == 'I' ) {
		goto s3;
	} else if( ch == 'J' ) {
		goto s3;
	} else if( ch == 'K' ) {
		goto s3;
	} else if( ch == 'L' ) {
		goto s3;
	} else if( ch == 'M' ) {
		goto s3;
	} else if( ch == 'N' ) {
		goto s3;
	} else if( ch == 'O' ) {
		goto s3;
	} else if( ch == 'P' ) {
		goto s3;
	} else if( ch == 'Q' ) {
		goto s3;
	} else if( ch == 'R' ) {
		goto s3;
	} else if( ch == 'S' ) {
		goto s3;
	} else if( ch == 'T' ) {
		goto s3;
	} else if( ch == 'U' ) {
		goto s3;
	} else if( ch == 'V' ) {
		goto s3;
	} else if( ch == 'W' ) {
		goto s3;
	} else if( ch == 'X' ) {
		goto s3;
	} else if( ch == 'Y' ) {
		goto s3;
	} else if( ch == 'Z' ) {
		goto s3;
	} else if( ch == '[' ) {
		goto s3;
	} else if( ch == '\\' ) {
		goto s3;
	} else if( ch == ']' ) {
		goto s3;
	} else if( ch == '^' ) {
		goto s3;
	} else if( ch == '_' ) {
		goto s3;
	} else if( ch == '`' ) {
		goto s3;
	} else if( ch == 'a' ) {
		goto s3;
	} else if( ch == 'b' ) {
		goto s3;
	} else if( ch == 'c' ) {
		goto s3;
	} else if( ch == 'd' ) {
		goto s3;
	} else if( ch == 'e' ) {
		goto s3;
	} else if( ch == 'f' ) {
		goto s3;
	} else if( ch == 'g' ) {
		goto s3;
	} else if( ch == 'h' ) {
		goto s3;
	} else if( ch == 'i' ) {
		goto s3;
	} else if( ch == 'j' ) {
		goto s3;
	} else if( ch == 'k' ) {
		goto s3;
	} else if( ch == 'l' ) {
		goto s3;
	} else if( ch == 'm' ) {
		goto s3;
	} else if( ch == 'n' ) {
		goto s3;
	} else if( ch == 'o' ) {
		goto s3;
	} else if( ch == 'p' ) {
		goto s3;
	} else if( ch == 'q' ) {
		goto s3;
	} else if( ch == 'r' ) {
		goto s3;
	} else if( ch == 's' ) {
		goto s3;
	} else if( ch == 't' ) {
		goto s3;
	} else if( ch == 'u' ) {
		goto s3;
	} else if( ch == 'v' ) {
		goto s3;
	} else if( ch == 'w' ) {
		goto s3;
	} else if( ch == 'x' ) {
		goto s3;
	} else if( ch == 'y' ) {
		goto s3;
	} else if( ch == 'z' ) {
		goto s3;
	} else if( ch == '{' ) {
		goto s3;
	} else if( ch == '|' ) {
		goto s3;
	} else if( ch == '}' ) {
		goto s3;
	} else if( ch == '~' ) {
		goto s3;
	} else {
		token.type = WORD;
		rollbackSize = 1;
		goto accept;
	}
s4:	ch = nextChar( scanner );
	if( ch == '\40' ) {
		goto s3;
	} else if( ch == '!' ) {
		goto s3;
	} else if( ch == '"' ) {
		goto s3;
	} else if( ch == '#' ) {
		goto s3;
	} else if( ch == '$' ) {
		goto s3;
	} else if( ch == '%' ) {
		goto s3;
	} else if( ch == '&' ) {
		goto s3;
	} else if( ch == '\'' ) {
		goto s3;
	} else if( ch == '(' ) {
		goto s3;
	} else if( ch == ')' ) {
		goto s3;
	} else if( ch == '*' ) {
		goto s3;
	} else if( ch == '+' ) {
		goto s3;
	} else if( ch == ',' ) {
		goto s3;
	} else if( ch == '-' ) {
		goto s3;
	} else if( ch == '.' ) {
		goto s3;
	} else if( ch == '/' ) {
		goto s5;
	} else if( ch == '0' ) {
		goto s3;
	} else if( ch == '1' ) {
		goto s3;
	} else if( ch == '2' ) {
		goto s3;
	} else if( ch == '3' ) {
		goto s3;
	} else if( ch == '4' ) {
		goto s3;
	} else if( ch == '5' ) {
		goto s3;
	} else if( ch == '6' ) {
		goto s3;
	} else if( ch == '7' ) {
		goto s3;
	} else if( ch == '8' ) {
		goto s3;
	} else if( ch == '9' ) {
		goto s3;
	} else if( ch == ':' ) {
		goto s3;
	} else if( ch == ';' ) {
		goto s3;
	} else if( ch == '<' ) {
		goto s3;
	} else if( ch == '=' ) {
		goto s3;
	} else if( ch == '?' ) {
		goto s3;
	} else if( ch == '@' ) {
		goto s3;
	} else if( ch == 'A' ) {
		goto s3;
	} else if( ch == 'B' ) {
		goto s3;
	} else if( ch == 'C' ) {
		goto s3;
	} else if( ch == 'D' ) {
		goto s3;
	} else if( ch == 'E' ) {
		goto s3;
	} else if( ch == 'F' ) {
		goto s3;
	} else if( ch == 'G' ) {
		goto s3;
	} else if( ch == 'H' ) {
		goto s3;
	} else if( ch == 'I' ) {
		goto s3;
	} else if( ch == 'J' ) {
		goto s3;
	} else if( ch == 'K' ) {
		goto s3;
	} else if( ch == 'L' ) {
		goto s3;
	} else if( ch == 'M' ) {
		goto s3;
	} else if( ch == 'N' ) {
		goto s3;
	} else if( ch == 'O' ) {
		goto s3;
	} else if( ch == 'P' ) {
		goto s3;
	} else if( ch == 'Q' ) {
		goto s3;
	} else if( ch == 'R' ) {
		goto s3;
	} else if( ch == 'S' ) {
		goto s3;
	} else if( ch == 'T' ) {
		goto s3;
	} else if( ch == 'U' ) {
		goto s3;
	} else if( ch == 'V' ) {
		goto s3;
	} else if( ch == 'W' ) {
		goto s3;
	} else if( ch == 'X' ) {
		goto s3;
	} else if( ch == 'Y' ) {
		goto s3;
	} else if( ch == 'Z' ) {
		goto s3;
	} else if( ch == '[' ) {
		goto s3;
	} else if( ch == '\\' ) {
		goto s3;
	} else if( ch == ']' ) {
		goto s3;
	} else if( ch == '^' ) {
		goto s3;
	} else if( ch == '_' ) {
		goto s3;
	} else if( ch == '`' ) {
		goto s3;
	} else if( ch == 'a' ) {
		goto s3;
	} else if( ch == 'b' ) {
		goto s3;
	} else if( ch == 'c' ) {
		goto s3;
	} else if( ch == 'd' ) {
		goto s3;
	} else if( ch == 'e' ) {
		goto s3;
	} else if( ch == 'f' ) {
		goto s3;
	} else if( ch == 'g' ) {
		goto s3;
	} else if( ch == 'h' ) {
		goto s3;
	} else if( ch == 'i' ) {
		goto s3;
	} else if( ch == 'j' ) {
		goto s3;
	} else if( ch == 'k' ) {
		goto s3;
	} else if( ch == 'l' ) {
		goto s3;
	} else if( ch == 'm' ) {
		goto s3;
	} else if( ch == 'n' ) {
		goto s3;
	} else if( ch == 'o' ) {
		goto s3;
	} else if( ch == 'p' ) {
		goto s3;
	} else if( ch == 'q' ) {
		goto s3;
	} else if( ch == 'r' ) {
		goto s3;
	} else if( ch == 's' ) {
		goto s3;
	} else if( ch == 't' ) {
		goto s3;
	} else if( ch == 'u' ) {
		goto s3;
	} else if( ch == 'v' ) {
		goto s3;
	} else if( ch == 'w' ) {
		goto s3;
	} else if( ch == 'x' ) {
		goto s3;
	} else if( ch == 'y' ) {
		goto s3;
	} else if( ch == 'z' ) {
		goto s3;
	} else if( ch == '{' ) {
		goto s3;
	} else if( ch == '|' ) {
		goto s3;
	} else if( ch == '}' ) {
		goto s3;
	} else if( ch == '~' ) {
		goto s3;
	} else {
		token.type = WORD;
		rollbackSize = 1;
		goto accept;
	}
s5:	ch = nextChar( scanner );
	if( ch == '\1' ) {
		goto s6;
	} else if( ch == '\2' ) {
		goto s6;
	} else if( ch == '\3' ) {
		goto s6;
	} else if( ch == '\4' ) {
		goto s6;
	} else if( ch == '\5' ) {
		goto s6;
	} else if( ch == '\6' ) {
		goto s6;
	} else if( ch == '\7' ) {
		goto s6;
	} else if( ch == '\10' ) {
		goto s6;
	} else if( ch == '\t' ) {
		goto s6;
	} else if( ch == '\13' ) {
		goto s6;
	} else if( ch == '\14' ) {
		goto s6;
	} else if( ch == '\r' ) {
		goto s6;
	} else if( ch == '\16' ) {
		goto s6;
	} else if( ch == '\17' ) {
		goto s6;
	} else if( ch == '\20' ) {
		goto s6;
	} else if( ch == '\21' ) {
		goto s6;
	} else if( ch == '\22' ) {
		goto s6;
	} else if( ch == '\23' ) {
		goto s6;
	} else if( ch == '\24' ) {
		goto s6;
	} else if( ch == '\25' ) {
		goto s6;
	} else if( ch == '\26' ) {
		goto s6;
	} else if( ch == '\27' ) {
		goto s6;
	} else if( ch == '\30' ) {
		goto s6;
	} else if( ch == '\31' ) {
		goto s6;
	} else if( ch == '\32' ) {
		goto s6;
	} else if( ch == '\33' ) {
		goto s6;
	} else if( ch == '\34' ) {
		goto s6;
	} else if( ch == '\35' ) {
		goto s6;
	} else if( ch == '\36' ) {
		goto s6;
	} else if( ch == '\37' ) {
		goto s6;
	} else if( ch == '\40' ) {
		goto s5;
	} else if( ch == '!' ) {
		goto s5;
	} else if( ch == '"' ) {
		goto s5;
	} else if( ch == '#' ) {
		goto s5;
	} else if( ch == '$' ) {
		goto s5;
	} else if( ch == '%' ) {
		goto s5;
	} else if( ch == '&' ) {
		goto s5;
	} else if( ch == '\'' ) {
		goto s5;
	} else if( ch == '(' ) {
		goto s5;
	} else if( ch == ')' ) {
		goto s5;
	} else if( ch == '*' ) {
		goto s5;
	} else if( ch == '+' ) {
		goto s5;
	} else if( ch == ',' ) {
		goto s5;
	} else if( ch == '-' ) {
		goto s5;
	} else if( ch == '.' ) {
		goto s5;
	} else if( ch == '/' ) {
		goto s5;
	} else if( ch == '0' ) {
		goto s5;
	} else if( ch == '1' ) {
		goto s5;
	} else if( ch == '2' ) {
		goto s5;
	} else if( ch == '3' ) {
		goto s5;
	} else if( ch == '4' ) {
		goto s5;
	} else if( ch == '5' ) {
		goto s5;
	} else if( ch == '6' ) {
		goto s5;
	} else if( ch == '7' ) {
		goto s5;
	} else if( ch == '8' ) {
		goto s5;
	} else if( ch == '9' ) {
		goto s5;
	} else if( ch == ':' ) {
		goto s5;
	} else if( ch == ';' ) {
		goto s5;
	} else if( ch == '<' ) {
		goto s5;
	} else if( ch == '=' ) {
		goto s5;
	} else if( ch == '>' ) {
		goto s6;
	} else if( ch == '?' ) {
		goto s5;
	} else if( ch == '@' ) {
		goto s5;
	} else if( ch == 'A' ) {
		goto s5;
	} else if( ch == 'B' ) {
		goto s5;
	} else if( ch == 'C' ) {
		goto s5;
	} else if( ch == 'D' ) {
		goto s5;
	} else if( ch == 'E' ) {
		goto s5;
	} else if( ch == 'F' ) {
		goto s5;
	} else if( ch == 'G' ) {
		goto s5;
	} else if( ch == 'H' ) {
		goto s5;
	} else if( ch == 'I' ) {
		goto s5;
	} else if( ch == 'J' ) {
		goto s5;
	} else if( ch == 'K' ) {
		goto s5;
	} else if( ch == 'L' ) {
		goto s5;
	} else if( ch == 'M' ) {
		goto s5;
	} else if( ch == 'N' ) {
		goto s5;
	} else if( ch == 'O' ) {
		goto s5;
	} else if( ch == 'P' ) {
		goto s5;
	} else if( ch == 'Q' ) {
		goto s5;
	} else if( ch == 'R' ) {
		goto s5;
	} else if( ch == 'S' ) {
		goto s5;
	} else if( ch == 'T' ) {
		goto s5;
	} else if( ch == 'U' ) {
		goto s5;
	} else if( ch == 'V' ) {
		goto s5;
	} else if( ch == 'W' ) {
		goto s5;
	} else if( ch == 'X' ) {
		goto s5;
	} else if( ch == 'Y' ) {
		goto s5;
	} else if( ch == 'Z' ) {
		goto s5;
	} else if( ch == '[' ) {
		goto s5;
	} else if( ch == '\\' ) {
		goto s5;
	} else if( ch == ']' ) {
		goto s5;
	} else if( ch == '^' ) {
		goto s5;
	} else if( ch == '_' ) {
		goto s5;
	} else if( ch == '`' ) {
		goto s5;
	} else if( ch == 'a' ) {
		goto s5;
	} else if( ch == 'b' ) {
		goto s5;
	} else if( ch == 'c' ) {
		goto s5;
	} else if( ch == 'd' ) {
		goto s5;
	} else if( ch == 'e' ) {
		goto s5;
	} else if( ch == 'f' ) {
		goto s5;
	} else if( ch == 'g' ) {
		goto s5;
	} else if( ch == 'h' ) {
		goto s5;
	} else if( ch == 'i' ) {
		goto s5;
	} else if( ch == 'j' ) {
		goto s5;
	} else if( ch == 'k' ) {
		goto s5;
	} else if( ch == 'l' ) {
		goto s5;
	} else if( ch == 'm' ) {
		goto s5;
	} else if( ch == 'n' ) {
		goto s5;
	} else if( ch == 'o' ) {
		goto s5;
	} else if( ch == 'p' ) {
		goto s5;
	} else if( ch == 'q' ) {
		goto s5;
	} else if( ch == 'r' ) {
		goto s5;
	} else if( ch == 's' ) {
		goto s5;
	} else if( ch == 't' ) {
		goto s5;
	} else if( ch == 'u' ) {
		goto s5;
	} else if( ch == 'v' ) {
		goto s5;
	} else if( ch == 'w' ) {
		goto s5;
	} else if( ch == 'x' ) {
		goto s5;
	} else if( ch == 'y' ) {
		goto s5;
	} else if( ch == 'z' ) {
		goto s5;
	} else if( ch == '{' ) {
		goto s5;
	} else if( ch == '|' ) {
		goto s5;
	} else if( ch == '}' ) {
		goto s5;
	} else if( ch == '~' ) {
		goto s5;
	} else if( ch == '\177' ) {
		goto s6;
	} else {
		token.type = COMMENT;
		rollbackSize = 1;
		goto accept;
	}
s6:	ch = nextChar( scanner );
	if( ch == '\1' ) {
		goto s6;
	} else if( ch == '\2' ) {
		goto s6;
	} else if( ch == '\3' ) {
		goto s6;
	} else if( ch == '\4' ) {
		goto s6;
	} else if( ch == '\5' ) {
		goto s6;
	} else if( ch == '\6' ) {
		goto s6;
	} else if( ch == '\7' ) {
		goto s6;
	} else if( ch == '\10' ) {
		goto s6;
	} else if( ch == '\t' ) {
		goto s6;
	} else if( ch == '\13' ) {
		goto s6;
	} else if( ch == '\14' ) {
		goto s6;
	} else if( ch == '\r' ) {
		goto s6;
	} else if( ch == '\16' ) {
		goto s6;
	} else if( ch == '\17' ) {
		goto s6;
	} else if( ch == '\20' ) {
		goto s6;
	} else if( ch == '\21' ) {
		goto s6;
	} else if( ch == '\22' ) {
		goto s6;
	} else if( ch == '\23' ) {
		goto s6;
	} else if( ch == '\24' ) {
		goto s6;
	} else if( ch == '\25' ) {
		goto s6;
	} else if( ch == '\26' ) {
		goto s6;
	} else if( ch == '\27' ) {
		goto s6;
	} else if( ch == '\30' ) {
		goto s6;
	} else if( ch == '\31' ) {
		goto s6;
	} else if( ch == '\32' ) {
		goto s6;
	} else if( ch == '\33' ) {
		goto s6;
	} else if( ch == '\34' ) {
		goto s6;
	} else if( ch == '\35' ) {
		goto s6;
	} else if( ch == '\36' ) {
		goto s6;
	} else if( ch == '\37' ) {
		goto s6;
	} else if( ch == '\40' ) {
		goto s6;
	} else if( ch == '!' ) {
		goto s6;
	} else if( ch == '"' ) {
		goto s6;
	} else if( ch == '#' ) {
		goto s6;
	} else if( ch == '$' ) {
		goto s6;
	} else if( ch == '%' ) {
		goto s6;
	} else if( ch == '&' ) {
		goto s6;
	} else if( ch == '\'' ) {
		goto s6;
	} else if( ch == '(' ) {
		goto s6;
	} else if( ch == ')' ) {
		goto s6;
	} else if( ch == '*' ) {
		goto s6;
	} else if( ch == '+' ) {
		goto s6;
	} else if( ch == ',' ) {
		goto s6;
	} else if( ch == '-' ) {
		goto s6;
	} else if( ch == '.' ) {
		goto s6;
	} else if( ch == '/' ) {
		goto s6;
	} else if( ch == '0' ) {
		goto s6;
	} else if( ch == '1' ) {
		goto s6;
	} else if( ch == '2' ) {
		goto s6;
	} else if( ch == '3' ) {
		goto s6;
	} else if( ch == '4' ) {
		goto s6;
	} else if( ch == '5' ) {
		goto s6;
	} else if( ch == '6' ) {
		goto s6;
	} else if( ch == '7' ) {
		goto s6;
	} else if( ch == '8' ) {
		goto s6;
	} else if( ch == '9' ) {
		goto s6;
	} else if( ch == ':' ) {
		goto s6;
	} else if( ch == ';' ) {
		goto s6;
	} else if( ch == '<' ) {
		goto s6;
	} else if( ch == '=' ) {
		goto s6;
	} else if( ch == '>' ) {
		goto s6;
	} else if( ch == '?' ) {
		goto s6;
	} else if( ch == '@' ) {
		goto s6;
	} else if( ch == 'A' ) {
		goto s6;
	} else if( ch == 'B' ) {
		goto s6;
	} else if( ch == 'C' ) {
		goto s6;
	} else if( ch == 'D' ) {
		goto s6;
	} else if( ch == 'E' ) {
		goto s6;
	} else if( ch == 'F' ) {
		goto s6;
	} else if( ch == 'G' ) {
		goto s6;
	} else if( ch == 'H' ) {
		goto s6;
	} else if( ch == 'I' ) {
		goto s6;
	} else if( ch == 'J' ) {
		goto s6;
	} else if( ch == 'K' ) {
		goto s6;
	} else if( ch == 'L' ) {
		goto s6;
	} else if( ch == 'M' ) {
		goto s6;
	} else if( ch == 'N' ) {
		goto s6;
	} else if( ch == 'O' ) {
		goto s6;
	} else if( ch == 'P' ) {
		goto s6;
	} else if( ch == 'Q' ) {
		goto s6;
	} else if( ch == 'R' ) {
		goto s6;
	} else if( ch == 'S' ) {
		goto s6;
	} else if( ch == 'T' ) {
		goto s6;
	} else if( ch == 'U' ) {
		goto s6;
	} else if( ch == 'V' ) {
		goto s6;
	} else if( ch == 'W' ) {
		goto s6;
	} else if( ch == 'X' ) {
		goto s6;
	} else if( ch == 'Y' ) {
		goto s6;
	} else if( ch == 'Z' ) {
		goto s6;
	} else if( ch == '[' ) {
		goto s6;
	} else if( ch == '\\' ) {
		goto s6;
	} else if( ch == ']' ) {
		goto s6;
	} else if( ch == '^' ) {
		goto s6;
	} else if( ch == '_' ) {
		goto s6;
	} else if( ch == '`' ) {
		goto s6;
	} else if( ch == 'a' ) {
		goto s6;
	} else if( ch == 'b' ) {
		goto s6;
	} else if( ch == 'c' ) {
		goto s6;
	} else if( ch == 'd' ) {
		goto s6;
	} else if( ch == 'e' ) {
		goto s6;
	} else if( ch == 'f' ) {
		goto s6;
	} else if( ch == 'g' ) {
		goto s6;
	} else if( ch == 'h' ) {
		goto s6;
	} else if( ch == 'i' ) {
		goto s6;
	} else if( ch == 'j' ) {
		goto s6;
	} else if( ch == 'k' ) {
		goto s6;
	} else if( ch == 'l' ) {
		goto s6;
	} else if( ch == 'm' ) {
		goto s6;
	} else if( ch == 'n' ) {
		goto s6;
	} else if( ch == 'o' ) {
		goto s6;
	} else if( ch == 'p' ) {
		goto s6;
	} else if( ch == 'q' ) {
		goto s6;
	} else if( ch == 'r' ) {
		goto s6;
	} else if( ch == 's' ) {
		goto s6;
	} else if( ch == 't' ) {
		goto s6;
	} else if( ch == 'u' ) {
		goto s6;
	} else if( ch == 'v' ) {
		goto s6;
	} else if( ch == 'w' ) {
		goto s6;
	} else if( ch == 'x' ) {
		goto s6;
	} else if( ch == 'y' ) {
		goto s6;
	} else if( ch == 'z' ) {
		goto s6;
	} else if( ch == '{' ) {
		goto s6;
	} else if( ch == '|' ) {
		goto s6;
	} else if( ch == '}' ) {
		goto s6;
	} else if( ch == '~' ) {
		goto s6;
	} else if( ch == '\177' ) {
		goto s6;
	} else {
		token.type = COMMENT;
		rollbackSize = 1;
		goto accept;
	}
s7:	ch = nextChar( scanner );
	if( ch == '\40' ) {
		goto s3;
	} else if( ch == '!' ) {
		goto s3;
	} else if( ch == '"' ) {
		goto s3;
	} else if( ch == '#' ) {
		goto s3;
	} else if( ch == '$' ) {
		goto s3;
	} else if( ch == '%' ) {
		goto s3;
	} else if( ch == '&' ) {
		goto s3;
	} else if( ch == '\'' ) {
		goto s3;
	} else if( ch == '(' ) {
		goto s3;
	} else if( ch == ')' ) {
		goto s3;
	} else if( ch == '*' ) {
		goto s3;
	} else if( ch == '+' ) {
		goto s3;
	} else if( ch == ',' ) {
		goto s3;
	} else if( ch == '-' ) {
		goto s3;
	} else if( ch == '.' ) {
		goto s3;
	} else if( ch == '/' ) {
		goto s3;
	} else if( ch == '0' ) {
		goto s7;
	} else if( ch == '1' ) {
		goto s7;
	} else if( ch == '2' ) {
		goto s7;
	} else if( ch == '3' ) {
		goto s7;
	} else if( ch == '4' ) {
		goto s7;
	} else if( ch == '5' ) {
		goto s7;
	} else if( ch == '6' ) {
		goto s7;
	} else if( ch == '7' ) {
		goto s7;
	} else if( ch == '8' ) {
		goto s7;
	} else if( ch == '9' ) {
		goto s7;
	} else if( ch == ':' ) {
		goto s3;
	} else if( ch == ';' ) {
		goto s3;
	} else if( ch == '<' ) {
		goto s3;
	} else if( ch == '=' ) {
		goto s3;
	} else if( ch == '?' ) {
		goto s3;
	} else if( ch == '@' ) {
		goto s3;
	} else if( ch == 'A' ) {
		goto s3;
	} else if( ch == 'B' ) {
		goto s3;
	} else if( ch == 'C' ) {
		goto s3;
	} else if( ch == 'D' ) {
		goto s3;
	} else if( ch == 'E' ) {
		goto s3;
	} else if( ch == 'F' ) {
		goto s3;
	} else if( ch == 'G' ) {
		goto s3;
	} else if( ch == 'H' ) {
		goto s3;
	} else if( ch == 'I' ) {
		goto s3;
	} else if( ch == 'J' ) {
		goto s3;
	} else if( ch == 'K' ) {
		goto s3;
	} else if( ch == 'L' ) {
		goto s3;
	} else if( ch == 'M' ) {
		goto s3;
	} else if( ch == 'N' ) {
		goto s3;
	} else if( ch == 'O' ) {
		goto s3;
	} else if( ch == 'P' ) {
		goto s3;
	} else if( ch == 'Q' ) {
		goto s3;
	} else if( ch == 'R' ) {
		goto s3;
	} else if( ch == 'S' ) {
		goto s3;
	} else if( ch == 'T' ) {
		goto s3;
	} else if( ch == 'U' ) {
		goto s3;
	} else if( ch == 'V' ) {
		goto s3;
	} else if( ch == 'W' ) {
		goto s3;
	} else if( ch == 'X' ) {
		goto s3;
	} else if( ch == 'Y' ) {
		goto s3;
	} else if( ch == 'Z' ) {
		goto s3;
	} else if( ch == '[' ) {
		goto s3;
	} else if( ch == '\\' ) {
		goto s3;
	} else if( ch == ']' ) {
		goto s3;
	} else if( ch == '^' ) {
		goto s3;
	} else if( ch == '_' ) {
		goto s3;
	} else if( ch == '`' ) {
		goto s3;
	} else if( ch == 'a' ) {
		goto s3;
	} else if( ch == 'b' ) {
		goto s3;
	} else if( ch == 'c' ) {
		goto s3;
	} else if( ch == 'd' ) {
		goto s3;
	} else if( ch == 'e' ) {
		goto s3;
	} else if( ch == 'f' ) {
		goto s3;
	} else if( ch == 'g' ) {
		goto s3;
	} else if( ch == 'h' ) {
		goto s3;
	} else if( ch == 'i' ) {
		goto s3;
	} else if( ch == 'j' ) {
		goto s3;
	} else if( ch == 'k' ) {
		goto s3;
	} else if( ch == 'l' ) {
		goto s3;
	} else if( ch == 'm' ) {
		goto s3;
	} else if( ch == 'n' ) {
		goto s3;
	} else if( ch == 'o' ) {
		goto s3;
	} else if( ch == 'p' ) {
		goto s3;
	} else if( ch == 'q' ) {
		goto s3;
	} else if( ch == 'r' ) {
		goto s3;
	} else if( ch == 's' ) {
		goto s3;
	} else if( ch == 't' ) {
		goto s3;
	} else if( ch == 'u' ) {
		goto s3;
	} else if( ch == 'v' ) {
		goto s3;
	} else if( ch == 'w' ) {
		goto s3;
	} else if( ch == 'x' ) {
		goto s3;
	} else if( ch == 'y' ) {
		goto s3;
	} else if( ch == 'z' ) {
		goto s3;
	} else if( ch == '{' ) {
		goto s3;
	} else if( ch == '|' ) {
		goto s3;
	} else if( ch == '}' ) {
		goto s3;
	} else if( ch == '~' ) {
		goto s3;
	} else {
		token.type = NUMBER_LITERAL;
		rollbackSize = 1;
		goto accept;
	}
sE:	rollback(scanner, rollbackSize);
	token.line =scanner->mLine;
	token.charPos =scanner->mCurrentPos;
	token.str = (char*) malloc( sizeof(char) * (scanner->mForward -scanner->mLexemmeBegin + 1) );
	memcpy( token.str,scanner->mLexemmeBegin, sizeof(char) * (scanner->mForward -scanner->mLexemmeBegin) );
	token.str[scanner->mForward -scanner->mLexemmeBegin] = '\0';
	scanner->mLexemmeBegin =scanner->mForward;
	return token;
accept:
	rollback(scanner, rollbackSize);
	token.line =scanner->mLine;
	token.charPos =scanner->mCurrentPos;
	token.str = (char*) malloc( sizeof(char) * (scanner->mForward -scanner->mLexemmeBegin + 1) );
	memcpy( token.str,scanner->mLexemmeBegin, sizeof(char) * (scanner->mForward -scanner->mLexemmeBegin) );
	token.str[scanner->mForward -scanner->mLexemmeBegin] = '\0';
	scanner->mLexemmeBegin =scanner->mForward;
	return token;
}