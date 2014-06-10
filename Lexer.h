#ifndef LEXER_H
#define LEXER_H

#include <QString>

class Lexer
{
public:
	enum TokenType {
		TK_ERROR            = -1,
		TK_END_OF_FILE      = 0,

		TK_EQUAL,
		TK_LESS_OR_EQUAL,
		TK_GREAT_OR_EQUAL,
		TK_NOT_EQUAL,

		TK_DOUBLE_COLON,    /* ':' */

		TK_DOTS,            /* '...' */
		TK_CONCAT,          /* '..' */

		TK_NIL,
		TK_TRUE,
		TK_FALSE,

		TK_NUMBER,
		TK_STRING,

		TK_NAME,

		TK_LOCAL,
		TK_FUNCTION,

		TK_OR,
		TK_AND,
		TK_NOT,

		TK_DO,
		TK_WHILE,
		TK_REPEAT,
		TK_IF,
		TK_FOR,
		TK_RETURN,
		TK_BREAK
	};

	struct Token {
		int Type;
		QChar* Begin;
		QChar* End;
		int LineNumber;
	};

public:
	Lexer( const QString& text );

	bool            HasNext() const;
	int             Next();

	const QString   CurrentString() const;
	int             CurrentTokenType() const;
	int             CurrentLine() const;
	int             CurrentPos() const;

	int             PeekNextTokenType() const;

private:
	int             AdvanceToken();

	int     SkipMultiLineSeparator();
	bool    SkipMultiLineContent( int count );

	bool    CurrIsNewline() const;
	void    SkipNewLine();

	bool    ReadString();
	bool    SkipDecimalEscapeSequence();
	bool    SkipHexEscapeSequence();

	void    SkipNumber();

	bool    CurrentIsAlpha() const;
	bool    CurrentIsAlphaOrNumber() const;
	int     CurrentKeyword() const;
	const QString   LexerString() const;

private:
	QString _text;

	QChar*  _next;
	QChar*  _nextTokenBegin;
	int     _nextTokenType;

	QChar*  _begin;
	QChar*  _end;

	int     _lineNumber;

	Token   _currentToken;
};

#endif // LEXER_H
