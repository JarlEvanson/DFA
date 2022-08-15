
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "tokens.h"
#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token nextToken( Scanner* scanner );

const size_t bufferSize = 4096;
const size_t bufferSizeSentinal = 4097;

static void fillBuffer( Scanner* scanner );
static void rollback( Scanner* scanner, size_t numChars );
static uint8_t nextChar( Scanner* scanner );

void initScanner( Scanner* scanner, const char* fileName ) {
	scanner->mFile = fopen( fileName, "r" );
	if( scanner->mFile == NULL ) {
		printf( "Failed to open %s\n", fileName );
		exit( 5 );
	}
	scanner->mCurrentFile = fileName;
	scanner->mLine = 0;
	scanner->mCurrentPos = 0;
	scanner->mBuffer[0][bufferSize] = '\0';
	scanner->mBuffer[1][bufferSize] = '\0';
	memset( &scanner->mBuffer[0][0], 0, bufferSizeSentinal * 2 );
	fillBuffer( scanner );
	scanner->mLexemmeBegin = &scanner->mBuffer[1][0];
	scanner->mForward = scanner->mLexemmeBegin;
	scanner->mNextToken = nextToken( scanner );
}

void fillBuffer( Scanner* scanner ) {
	//Copy the entire second buffer, excluding sentinal values, to the beginning of the first buffer
	memcpy( scanner->mBuffer, &scanner->mBuffer[1][0], bufferSize );

	size_t objectsRead = fread( &scanner->mBuffer[1][0], sizeof( uint8_t ), bufferSize, scanner->mFile );
	if( objectsRead != bufferSize ) {
		if( feof( scanner->mFile ) ) {
			scanner->mBuffer[1][objectsRead] = '\0';
			scanner->mForward = &scanner->mBuffer[1][0];
			scanner->mLexemmeBegin -= bufferSize;
		} else if( ferror( scanner->mFile ) ) {
			printf( "Failed to read from %s\n", scanner->mCurrentFile );
			exit( 5 );
		}
	} else {
		scanner->mForward = &scanner->mBuffer[1][0];
		scanner->mLexemmeBegin -= bufferSize;
	}
}

uint8_t nextChar( Scanner* scanner ) {
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

void rollback( Scanner* scanner, size_t numChars ) {
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

Token peek( Scanner* scanner ) {
	return scanner->mNextToken;
}

Token next( Scanner* scanner ) {
	Token ret = scanner->mNextToken;
	scanner->mNextToken = nextToken( scanner );
	return ret;
}

Token nextToken( Scanner* scanner ) {
	Token token;
	char ch;
	uint32_t rollbackSize = 0;
	goto s0;
s0:
	ch = nextChar( scanner );
	if( ch == '\0' ) {
		token.type = END_OF_FILE;
		goto accept;
	} else if( ch >= '\1' && ch <= '\10' ) {
		goto s1;
	} else if( ch == '\13' ) {
		goto s1;
	} else if( ch == '\14' ) {
		goto s1;
	} else if( ch >= '\16' && ch <= '\37' ) {
		goto s1;
	} else if( ch >= '!' && ch <= ',' ) {
		goto s1;
	} else if( ch == '.' ) {
		goto s1;
	} else if( ch == '/' ) {
		goto s1;
	} else if( ch >= ':' && ch <= '@' ) {
		goto s1;
	} else if( ch == '[' ) {
		goto s1;
	} else if( ch >= ']' && ch <= '`' ) {
		goto s1;
	} else if( ch >= '{' && ch <= '\177' ) {
		goto s1;
	} else if( ch >= '0' && ch <= '9' ) {
		goto s2;
	} else if( ch == '\n' ) {
		goto s4;
	} else if( ch == '\t' ) {
		goto s5;
	} else if( ch == '\r' ) {
		goto s5;
	} else if( ch == '\40' ) {
		goto s5;
	} else if( ch == '-' ) {
		goto s7;
	} else if( ch == '\\' ) {
		goto s8;
	} else if( ch == 'D' ) {
		goto s25;
	} else if( ch == 'A' ) {
		goto s33;
	} else if( ch == 'B' ) {
		goto s33;
	} else if( ch == 'C' ) {
		goto s33;
	} else if( ch == 'E' ) {
		goto s33;
	} else if( ch >= 'G' && ch <= 'Z' ) {
		goto s33;
	} else if( ch >= 'a' && ch <= 'z' ) {
		goto s33;
	} else if( ch == 'F' ) {
		goto s33;
	} else {
		token.type = ERROR;
		rollbackSize = 1;
		goto sE;
	}
s1:
	token.type = CHAR;
	goto accept;
s2:
	ch = nextChar( scanner );
	if( ch >= '0' && ch <= '7' ) {
		goto s3;
	} else if( ch == '9' ) {
		goto s3;
	} else if( ch == '8' ) {
		goto s3;
	} else {
		token.type = NUMBER;
		rollbackSize = 1;
		goto accept;
	}
s3:
	ch = nextChar( scanner );
	if( ch >= '0' && ch <= '7' ) {
		goto s3;
	} else if( ch == '9' ) {
		goto s3;
	} else if( ch == '8' ) {
		goto s3;
	} else {
		token.type = NUMBER;
		rollbackSize = 1;
		goto accept;
	}
s4:
	ch = nextChar( scanner );
	if( ch == '\n' ) {
		goto s4;
	} else {
		token.type = NEWLINE;
		rollbackSize = 1;
		goto accept;
	}
s5:
	ch = nextChar( scanner );
	if( ch == '\t' ) {
		goto s6;
	} else if( ch == '\40' ) {
		goto s6;
	} else if( ch == '\r' ) {
		goto s6;
	} else {
		token.type = WHITESPACE;
		rollbackSize = 1;
		goto accept;
	}
s6:
	ch = nextChar( scanner );
	if( ch == '\t' ) {
		goto s6;
	} else if( ch == '\40' ) {
		goto s6;
	} else if( ch == '\r' ) {
		goto s6;
	} else {
		token.type = WHITESPACE;
		rollbackSize = 1;
		goto accept;
	}
s7:
	token.type = CHAR;
	goto accept;
s8:
	ch = nextChar( scanner );
	if( ch == '\\' ) {
		goto s9;
	} else if( ch == ']' ) {
		goto s10;
	} else if( ch == 'n' ) {
		goto s11;
	} else if( ch == 'r' ) {
		goto s12;
	} else if( ch == 't' ) {
		goto s13;
	} else if( ch == 'x' ) {
		goto s14;
	} else if( ch == '0' ) {
		goto s15;
	} else if( ch >= '1' && ch <= '7' ) {
		goto s19;
	} else if( ch == '(' ) {
		goto s20;
	} else if( ch == ')' ) {
		goto s21;
	} else if( ch == '?' ) {
		goto s22;
	} else if( ch == '[' ) {
		goto s23;
	} else if( ch == 'w' ) {
		goto s24;
	} else if( ch == '*' ) {
		goto s35;
	} else if( ch == '+' ) {
		goto s34;
	} else if( ch == '.' ) {
		goto s36;
	} else if( ch == '|' ) {
		goto s37;
	} else {
		token.type = CHAR;
		rollbackSize = 1;
		goto accept;
	}
s9:
	token.type = BACKSLASH_LITERAL;
	goto accept;
s10:
	token.type = CLOSE_BRACKET_LITERAL;
	goto accept;
s11:
	token.type = NEWLINE_LITERAL;
	goto accept;
s12:
	token.type = RETURN_LITERAL;
	goto accept;
s13:
	token.type = TAB_LITERAL;
	goto accept;
s14:
	token.type = HEX_DIGIT;
	goto accept;
s15:
	ch = nextChar( scanner );
	if( ch >= '1' && ch <= '7' ) {
		goto s16;
	} else if( ch == '0' ) {
		goto s18;
	} else {
		token.type = OCTAL_ESCAPE;
		rollbackSize = 1;
		goto accept;
	}
s16:
	ch = nextChar( scanner );
	if( ch >= '0' && ch <= '5' ) {
		goto s17;
	} else if( ch == '7' ) {
		goto s17;
	} else if( ch == '6' ) {
		goto s17;
	} else {
		token.type = OCTAL_ESCAPE;
		rollbackSize = 1;
		goto accept;
	}
s17:
	token.type = OCTAL_ESCAPE;
	goto accept;
s18:
	ch = nextChar( scanner );
	if( ch >= '0' && ch <= '5' ) {
		goto s17;
	} else if( ch == '7' ) {
		goto s17;
	} else if( ch == '6' ) {
		goto s17;
	} else {
		token.type = OCTAL_ESCAPE;
		rollbackSize = 1;
		goto accept;
	}
s19:
	ch = nextChar( scanner );
	if( ch >= '1' && ch <= '7' ) {
		goto s16;
	} else if( ch == '0' ) {
		goto s18;
	} else {
		token.type = OCTAL_ESCAPE;
		rollbackSize = 1;
		goto accept;
	}
s20:
	token.type = OPEN_PAREN_LITERAL;
	goto accept;
s21:
	token.type = CLOSE_PAREN_LITERAL;
	goto accept;
s22:
	token.type = QUESTION_MARK_LITERAL;
	goto accept;
s23:
	token.type = OPEN_BRACKET_LITERAL;
	goto accept;
s24:
	token.type = WORD;
	goto accept;
s25:
	ch = nextChar( scanner );
	if( ch >= '0' && ch <= '9' ) {
		goto s26;
	} else if( ch >= 'A' && ch <= 'H' ) {
		goto s26;
	} else if( ch >= 'J' && ch <= 'Z' ) {
		goto s26;
	} else if( ch == '_' ) {
		goto s26;
	} else if( ch >= 'a' && ch <= 'z' ) {
		goto s26;
	} else if( ch == 'I' ) {
		goto s27;
	} else {
		token.type = CHAR;
		rollbackSize = 1;
		goto accept;
	}
s26:
	ch = nextChar( scanner );
	if( ch >= '0' && ch <= '9' ) {
		goto s26;
	} else if( ch >= 'A' && ch <= 'Z' ) {
		goto s26;
	} else if( ch == '_' ) {
		goto s26;
	} else if( ch >= 'a' && ch <= 'x' ) {
		goto s26;
	} else if( ch == 'z' ) {
		goto s26;
	} else if( ch == 'y' ) {
		goto s26;
	} else {
		token.type = IDENTIFIER;
		rollbackSize = 1;
		goto accept;
	}
s27:
	ch = nextChar( scanner );
	if( ch == 'S' ) {
		goto s28;
	} else if( ch >= '0' && ch <= '9' ) {
		goto s26;
	} else if( ch >= 'A' && ch <= 'R' ) {
		goto s26;
	} else if( ch >= 'T' && ch <= 'Z' ) {
		goto s26;
	} else if( ch == '_' ) {
		goto s26;
	} else if( ch >= 'a' && ch <= 'x' ) {
		goto s26;
	} else if( ch == 'z' ) {
		goto s26;
	} else if( ch == 'y' ) {
		goto s26;
	} else {
		token.type = IDENTIFIER;
		rollbackSize = 1;
		goto accept;
	}
s28:
	ch = nextChar( scanner );
	if( ch == 'C' ) {
		goto s29;
	} else if( ch >= '0' && ch <= '9' ) {
		goto s26;
	} else if( ch == 'A' ) {
		goto s26;
	} else if( ch == 'B' ) {
		goto s26;
	} else if( ch >= 'D' && ch <= 'Z' ) {
		goto s26;
	} else if( ch == '_' ) {
		goto s26;
	} else if( ch >= 'a' && ch <= 'x' ) {
		goto s26;
	} else if( ch == 'z' ) {
		goto s26;
	} else if( ch == 'y' ) {
		goto s26;
	} else {
		token.type = IDENTIFIER;
		rollbackSize = 1;
		goto accept;
	}
s29:
	ch = nextChar( scanner );
	if( ch == 'A' ) {
		goto s30;
	} else if( ch >= '0' && ch <= '9' ) {
		goto s26;
	} else if( ch >= 'B' && ch <= 'Z' ) {
		goto s26;
	} else if( ch == '_' ) {
		goto s26;
	} else if( ch >= 'a' && ch <= 'x' ) {
		goto s26;
	} else if( ch == 'z' ) {
		goto s26;
	} else if( ch == 'y' ) {
		goto s26;
	} else {
		token.type = IDENTIFIER;
		rollbackSize = 1;
		goto accept;
	}
s30:
	ch = nextChar( scanner );
	if( ch == 'R' ) {
		goto s31;
	} else if( ch >= '0' && ch <= '9' ) {
		goto s26;
	} else if( ch >= 'A' && ch <= 'Q' ) {
		goto s26;
	} else if( ch >= 'S' && ch <= 'Z' ) {
		goto s26;
	} else if( ch == '_' ) {
		goto s26;
	} else if( ch >= 'a' && ch <= 'x' ) {
		goto s26;
	} else if( ch == 'z' ) {
		goto s26;
	} else if( ch == 'y' ) {
		goto s26;
	} else {
		token.type = IDENTIFIER;
		rollbackSize = 1;
		goto accept;
	}
s31:
	ch = nextChar( scanner );
	if( ch == 'D' ) {
		goto s32;
	} else if( ch >= '0' && ch <= '9' ) {
		goto s26;
	} else if( ch == 'A' ) {
		goto s26;
	} else if( ch == 'B' ) {
		goto s26;
	} else if( ch == 'C' ) {
		goto s26;
	} else if( ch >= 'E' && ch <= 'Z' ) {
		goto s26;
	} else if( ch == '_' ) {
		goto s26;
	} else if( ch >= 'a' && ch <= 'x' ) {
		goto s26;
	} else if( ch == 'z' ) {
		goto s26;
	} else if( ch == 'y' ) {
		goto s26;
	} else {
		token.type = IDENTIFIER;
		rollbackSize = 1;
		goto accept;
	}
s32:
	ch = nextChar( scanner );
	if( ch >= '0' && ch <= '9' ) {
		goto s26;
	} else if( ch >= 'A' && ch <= 'Z' ) {
		goto s26;
	} else if( ch == '_' ) {
		goto s26;
	} else if( ch >= 'a' && ch <= 'x' ) {
		goto s26;
	} else if( ch == 'z' ) {
		goto s26;
	} else if( ch == 'y' ) {
		goto s26;
	} else {
		token.type = DISCARD;
		rollbackSize = 1;
		goto accept;
	}
s33:
	ch = nextChar( scanner );
	if( ch >= '0' && ch <= '9' ) {
		goto s26;
	} else if( ch >= 'A' && ch <= 'Z' ) {
		goto s26;
	} else if( ch == '_' ) {
		goto s26;
	} else if( ch >= 'a' && ch <= 'x' ) {
		goto s26;
	} else if( ch == 'z' ) {
		goto s26;
	} else if( ch == 'y' ) {
		goto s26;
	} else {
		token.type = CHAR;
		rollbackSize = 1;
		goto accept;
	}
s34:
	token.type = PLUS_LITERAL;
	goto accept;
s35:
	token.type = STAR_LITERAL;
	goto accept;
s36:
	token.type = DOT_LITERAL;
	goto accept;
s37:
	token.type = PIPE_LITERAL;
	goto accept;
sE:	rollback( scanner, rollbackSize );
	token.line = scanner->mLine;
	token.charPos = scanner->mCurrentPos;
	token.str = (char*) malloc( sizeof( char ) * ( scanner->mForward - scanner->mLexemmeBegin + 1 ) );
	memcpy( token.str, scanner->mLexemmeBegin, sizeof( char ) * ( scanner->mForward - scanner->mLexemmeBegin ) );
	token.str[scanner->mForward - scanner->mLexemmeBegin] = '\0';
	scanner->mLexemmeBegin = scanner->mForward;
	return token;
accept:
	rollback( scanner, rollbackSize );
	token.line = scanner->mLine;
	token.charPos = scanner->mCurrentPos;
	token.str = (char*) malloc( sizeof( char ) * ( scanner->mForward - scanner->mLexemmeBegin + 1 ) );
	memcpy( token.str, scanner->mLexemmeBegin, sizeof( char ) * ( scanner->mForward - scanner->mLexemmeBegin ) );
	token.str[scanner->mForward - scanner->mLexemmeBegin] = '\0';
	scanner->mLexemmeBegin = scanner->mForward;
	return token;
}

const char* toString( TokenType type ) {
	switch( type ) {
		case DISCARD:
			return "DISCARD";
		case IDENTIFIER:
			return "IDENTIFIER";
		case NUMBER:
			return "NUMBER";
		case ALNUM:
			return "ALNUM";
		case NEWLINE:
			return "NEWLINE";
		case WHITESPACE:
			return "WHITESPACE";
		case CHAR:
			return "CHAR";
		case DASH:
			return "DASH";
		case OCTAL_ESCAPE:
			return "OCTAL_ESCAPE";
		case QUESTION_MARK_LITERAL:
			return "QUESTION_MARK_LITERAL";
		case CLOSE_PAREN_LITERAL:
			return "CLOSE_PAREN_LITERAL";
		case OPEN_PAREN_LITERAL:
			return "OPEN_PAREN_LITERAL";
		case CLOSE_BRACKET_LITERAL:
			return "CLOSE_BRACKET_LITERAL";
		case OPEN_BRACKET_LITERAL:
			return "OPEN_BRACKET_LITERAL";
		case BACKSLASH_LITERAL:
			return "BACKSLASH_LITERAL";
		case HEX_DIGIT:
			return "HEX_DIGIT";
		case WORD:
			return "WORD";
		case RETURN_LITERAL:
			return "RETURN_LITERAL";
		case TAB_LITERAL:
			return "TAB_LITERAL";
		case NEWLINE_LITERAL:
			return "NEWLINE_LITERAL";
		case PLUS_LITERAL:
			return "PLUS_LITERAL";
		case DOT_LITERAL:
			return "DOT_LITERAL";
		case STAR_LITERAL:
			return "STAR_LITERAL";
		case PIPE_LITERAL:
			return "PIPE_LITERAL";
		case ERROR:
			return "ERROR";
		case END_OF_FILE:
			return "END_OF_FILE";
	}
	return NULL;
}

