#ifndef TOKENTYPE_H
#define TOKENTYPE_H

enum TokenType {
	TT_ERROR            = -1,
	TT_END_OF_FILE      = 0,

	TT_EQUAL,
	TT_LESS_OR_EQUAL,
	TT_GREAT_OR_EQUAL,
	TT_NOT_EQUAL,

	TT_DOUBLE_COLON,    /* '::' */

	TT_DOTS,            /* '...' */
	TT_CONCAT,          /* '..' */
	TT_POINT,			/* '.' */

	TT_NIL,
	TT_TRUE,
	TT_FALSE,

	TT_NUMBER,
	TT_STRING,

	TT_NAME,

	TT_MINUS,		// '-'
	TT_PLUS,		// '+'
	TT_MAGNIFY,		// '*'
	TT_SLASH,		// '/'
	TT_ASSIGN,		// '='
	TT_LESS,		// '<'
	TT_GREAT,		// '>'
	TT_TILDA,		// '~'
	TT_COLON,		// ':'
	TT_CARET,		// '^'
	TT_PERCENT,		// '%'
	TT_SEMICOLON,	// ';'
	TT_COMMA,		// ','
	TT_NUMBER_SIGN,	// '#'

	TT_LEFT_SQUARE,		// '['
	TT_RIGHT_SQUARE,	// ']'
	TT_LEFT_BRACKET,	// '('
	TT_RIGHT_BRACKET,	// ')'
	TT_LEFT_CURLY,		// '{'
	TT_RIGHT_CURLY,		// '}'

	TT_DO,
	TT_WHILE,
	TT_REPEAT,
	TT_IF,
	TT_FOR,
	TT_RETURN,
	TT_BREAK,
	TT_END,

	TT_LOCAL,
	TT_FUNCTION,

	TT_OR,
	TT_AND,
	TT_NOT,
};

#endif // TOKENTYPE_H
