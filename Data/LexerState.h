#ifndef LEXERSTATE_H
#define LEXERSTATE_H

#include "TokenType.h"

#include <QChar>

struct LexerState
{
	const QChar*	Begin;
	const QChar*	End;

	const QChar*	Current;
	const QChar*	Previos;

	TokenType		Type;
	int				LineNumber;
};

#endif // LEXERSTATE_H
