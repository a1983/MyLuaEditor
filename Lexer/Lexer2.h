#ifndef LEXER_2_H
#define LEXER_2_H

#include <QString>

#include "Data/LexerState.h"

class Lexer2
{
public:
	Lexer2();
	explicit Lexer2	( const QString* source );
//	explicit Lexer2	( const LexerState& state );

	bool            HasNext() const;
	bool            NextIf( TokenType type );
	TokenType       Next();


	bool            Is( TokenType type ) const;
	const QString   CurrentText() const;
	TokenType       CurrentType() const;
	int             CurrentLine() const;
	int             CurrentPos() const;

private:
	int				SkipMultiLineSeparator();
	bool			SkipMultiLineContent( int count );

	bool			CurrIsNewline() const;
	void			SkipNewLine();

	bool			ReadString();
	bool			SkipDecimalEscapeSequence();
	bool			SkipHexEscapeSequence();

	void			SkipNumber();

	bool			CurrentIsAlpha() const;
	bool			CurrentIsAlphaOrNumber() const;
	TokenType		CurrentKeyword() const;

private:
	LexerState _state;
};

#endif // LEXER_H
