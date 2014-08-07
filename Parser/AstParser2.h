#ifndef ASTPARSER_2_H
#define ASTPARSER_2_H

#include "Lexer/Lexer2.h"

#include "Data/AstItem.h"
#include "Data/VariableInfo.h"

class AstParser2
{
public:
	AstParser2( const QString& source );

	bool Parse();

	bool HasError() const;
	QString Error() const;

	AstItem* Result();
	VariableInfo* Global();

	QString Debug();

private:
	bool TryBlock				( AstItem* item );
	bool TryStatement			( AstItem* item );
	bool TryLastStatement		( AstItem* item );

	bool TryDoStatement			( AstItem* item );
	bool TryWhileStatement		( AstItem* item );
	bool TryRepeatStatement		( AstItem* item );
	bool TryIfStatement			( AstItem* item );
	bool TryForStatement		( AstItem* item );
	bool TryFunctionStatement	( AstItem* item );
	bool TryLocalStatement		( AstItem* item );
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

	bool ShouldNameList			( AstItem* item );

private:
	void GenerateError( const QString& description );

private:
	Lexer2 _lexer;

	const QString _source;

	QString _error;

	AstItem _global;
};

#endif // ASTPARSER_2_H
