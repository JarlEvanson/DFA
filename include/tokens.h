#ifndef COMPILER_TOKENS
#define COMPILER_TOKENS

#include "common.h"

typedef enum TokenType {
	DISCARD,
	IDENTIFIER,
	NUMBER,
	ALNUM,
	NEWLINE,
	WHITESPACE,
	CHAR,
	DASH,
	OCTAL_ESCAPE,
	QUESTION_MARK_LITERAL,
	CLOSE_PAREN_LITERAL,
	OPEN_PAREN_LITERAL,
	CLOSE_BRACKET_LITERAL,
	OPEN_BRACKET_LITERAL,
	BACKSLASH_LITERAL,
	HEX_DIGIT,
	WORD,
	RETURN_LITERAL,
	TAB_LITERAL,
	NEWLINE_LITERAL,
	ERROR,
	PLUS_LITERAL,
	STAR_LITERAL,
	DOT_LITERAL,
	PIPE_LITERAL,
	END_OF_FILE
} TokenType;

typedef struct Token {
	TokenType type;
	char* str;
	size_t charPos;
	size_t line;
} Token;

#endif