#ifndef ASTPARSER_2_H
#define ASTPARSER_2_H

#include "Lexer/Lexer2.h"

#include "Data/AstItem.h"

class AstParser2
{
public:
	AstParser2( const QString& source );

	bool Parse();

	bool HasError() const;
	QString Error() const;

	QString Debug();

private:
	bool TryBlock				( AstItem* item );
	bool TryStatement			( AstItem* item );
	bool TryLastStatement		( AstItem* item );

	bool TryCallOrAssign		( AstItem* item );
	bool TryPrefixExpression	( AstItem* item );
	bool TryPrefixSubExpression	( AstItem* item );
	bool TryArgs				( AstItem* item );
	bool TryConstructor			( AstItem* item );
	bool TryField				( AstItem* item );

	bool TryExpressionList		( AstItem* item );
	bool TryExpression			( AstItem* item );

	bool ShouldFunctionBody		( AstItem* item );
	bool TryFunctionParams		( AstItem* item );
	bool TryFunctionParam		( AstItem* item );

private:
	void Confirm();
	void Decline();
	void GenerateError( const QString& description );

private:
	Lexer2 _current;
	Lexer2 _advance;

	const QString _source;

	QString _error;

	AstItem _global;
};

#endif // ASTPARSER_2_H
