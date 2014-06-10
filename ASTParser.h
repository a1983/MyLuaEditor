#ifndef ASTPARSER_H
#define ASTPARSER_H

#include "Lexer.h"
#include "CodeModel.h"

class ASTParser
{
	enum ParseResult {
		Parse_OK,
		Parse_Fail,
		Parse_Mismatch
	};

public:
	ASTParser( const Lexer& lexer );

	bool Parse();

	Context* Result();

	const QString Error() const;

private:
	bool ParseBlock( Context* context );

	bool ParseGlobalDefineOrFunctionCall          ( Context* context );
	bool ParseGlobalFunctionDefine  ( Context* context );
	bool ParseDoStatement           ( Context* context );
	bool ParseWhileStatement        ( Context* context );
	bool ParseRepeatStatement       ( Context* context );
	bool ParseIfStatement           ( Context* context );
	bool ParseForStatement          ( Context* context );
	bool ParseLocalDefine           ( Context* context );
	bool ParseReturnStatement       ( Context* context );

	bool ParseExpressionList        ( Context* context );
	bool ParseExpression            ( Context* context );

	bool ParseTableConstructor      ( Context* context );
	bool ParseAnonymousFunction     ( Context* context );
	bool ParsePrefixExpression      ( Context* context );
	ParseResult ParsePrefixSubExpression      ( Context* context );

	ParseResult ParseFunctionArgs			( Context* context );
	bool ParseFieldDefine			( Context* context );


	void GenerateError();

private:
	Context*    _result;
	Lexer       _lexer;

	QString     _error;
};

#endif // ASTPARSER_H
