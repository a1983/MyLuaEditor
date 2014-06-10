#include "ASTParser.h"

#include <QDebug>

#include <QScopedPointer>

ASTParser::ASTParser( const Lexer& lexer ) :
	_lexer( lexer ),
	_result( new Context( "global" ) )
{
}

bool ASTParser::Parse()
{
	if( _lexer.HasNext()
		&& ParseBlock( _result )
		&& !_lexer.HasNext() )
	{
		return true;
	}

	GenerateError();
	return false;
}

Context* ASTParser::Result()
{
	return _result;
}

bool ASTParser::ParseBlock( Context* context )
{
	// Create AST block, if parsing done, append as child to context
	QScopedPointer< Context > block( new Context( "Block" ) );

	// Parse first lexer
	while( _lexer.HasNext() ) {
		// Advance lexer state
		int tokenType = _lexer.Next();

		// Switch on token type
		switch( tokenType ) {
		//  TK_NAME -> Global Definition or Function Call
		case Lexer::TK_NAME :		{ if( ParseGlobalDefineOrFunctionCall	( block.data() ) ) continue; break;	}
		//  TK_FUNCTION -> Global Function Definition
		case Lexer::TK_FUNCTION :	{ if( ParseGlobalFunctionDefine			( block.data() ) ) continue; break; }
		//  TK_DO, TK_WHILE, TK_REPEAT, TK_IF, TK_FOR -> Statement
		case Lexer::TK_DO :			{ if( ParseDoStatement					( block.data() ) ) continue; break; }
		case Lexer::TK_WHILE :		{ if( ParseWhileStatement   			( block.data() ) ) continue; break; }
		case Lexer::TK_REPEAT :		{ if( ParseRepeatStatement  			( block.data() ) ) continue; break; }
		case Lexer::TK_IF :			{ if( ParseIfStatement      			( block.data() ) ) continue; break; }
		case Lexer::TK_FOR :		{ if( ParseForStatement     			( block.data() ) ) continue; break; }
		//  TK_LOCAL -> Local definition
		case Lexer::TK_LOCAL :		{ if( ParseLocalDefine      			( block.data() ) ) continue; break; }
		//  TK_RETURN, TK_BREAK -> Last statement -> exit
		case Lexer::TK_RETURN :		{ if( ParseReturnStatement				( block.data() ) ) goto end_of_block; break; }
		case Lexer::TK_BREAK :		{ goto end_of_block; }
		//  TK_END_OF_FILE -> exit
		case Lexer::TK_END_OF_FILE : {
			if( block->HasChild() )
				goto end_of_block;
			break;
		}
		}

		//  default: -> Error.
		return false;
	}

end_of_block:
	// All done, set block ownership
	context->Append( block.take() );
	return true;
}

bool ASTParser::ParseGlobalDefineOrFunctionCall( Context* context )
{
	// Create AST block, if parsing done, append as child to context
	QScopedPointer< Context > statement( new Context( "gdfc" ) );
	if( !ParsePrefixExpression( statement.take() ) )
		return false;
	if( _lexer.PeekNextTokenType() == L'=' ) {
		if( !ParseExpressionList( statement.data() ) )
			return false;
	}

	while( _lexer.PeekNextTokenType() == L',' ) {
		_lexer.Next();
		if( statement->Children.last()->Name == "arg" )
			return false;
	}
}

bool ASTParser::ParseGlobalFunctionDefine(Context* context)
{
	return false;
}

bool ASTParser::ParseDoStatement(Context* context)
{
	return false;
}

bool ASTParser::ParseWhileStatement(Context* context)
{
	return false;
}

bool ASTParser::ParseRepeatStatement(Context* context)
{
	return false;
}

bool ASTParser::ParseIfStatement(Context* context)
{
	return false;
}

bool ASTParser::ParseForStatement( Context* context )
{
	return false;
}

bool ASTParser::ParseLocalDefine( Context* context )
{
	return false;
}

bool ASTParser::ParseReturnStatement( Context* context )
{
	// Create AST block, if parsing done, append as child to context
	QScopedPointer< Context > returnStatement( new Context( "return" ) );

	if( !ParseExpressionList( returnStatement.data() ) )
		return false;

	context->Append( returnStatement.take() );
	return true;
}

bool ASTParser::ParseExpressionList( Context* context )
{
	// Create AST block, if parsing done, append as child to context
	QScopedPointer< Context > expressionList( new Context( "expressionList" ) );

	forever {
		_lexer.Next(); // advance to expression

		// parse exp
		if( !ParseExpression( expressionList.data() ) )
			return false;

		// end of list
		if( _lexer.PeekNextTokenType() != L',' )
			break;

		// skip comma ','
		_lexer.Next();
	}

	// if list is empty error
	if( !expressionList->HasChild() ) {
		return false;
	}

	context->Append( expressionList.take() );
	return true;
}

bool IsUnaryOperator( int token ) {
	switch( token ) {
	case Lexer::TK_NOT:
	case L'-': case L'#':
		return true;
	}

	return false;
}

bool IsBinaryOperator( int token ) {
	switch( token ) {
	case Lexer::TK_EQUAL:
	case Lexer::TK_LESS_OR_EQUAL:
	case Lexer::TK_GREAT_OR_EQUAL:
	case Lexer::TK_NOT_EQUAL:
	case Lexer::TK_CONCAT:
	case Lexer::TK_OR: case Lexer::TK_AND:
	case L'+': case L'-': case L'*': case L'/': case L'%': case L'^':
	case L'<': case L'>':
		return true;
	}

	return false;
}

bool ASTParser::ParseExpression( Context* context )
{
	// Create AST block, if parsing done, append as child to context
	QScopedPointer< Context > expression( new Context( "expression" ) );

	int tokenType = _lexer.CurrentTokenType();

	switch( tokenType ){
	case Lexer::TK_NIL:
	case Lexer::TK_TRUE:
	case Lexer::TK_FALSE:
	case Lexer::TK_NUMBER:
	case Lexer::TK_STRING:
	case Lexer::TK_DOTS:
		// Literals
		expression->Append( "literal" );
		break;
	case Lexer::TK_FUNCTION:
		// Function body
		if( !ParseAnonymousFunction( expression.data() ) )
			return false;
		break;
	case L'{':
		// Table constructor
		if( !ParseTableConstructor( expression.data() ) )
			return false;
		break;

	case L'(':
		_lexer.Next(); // advance to expression
		if( !ParseExpression( expression.data() ) )
			return false;
		if( _lexer.Next() != L')' )
			return false;
		break;

	default:
		if( ParsePrefixExpression( expression.data() ) )
			break;

		return false;
	}
	// lookaheed for binop, if binop, create new binop
	if( IsUnaryOperator( tokenType ) ) {
		_lexer.Next();
		expression->Name = _lexer.CurrentString();
		_lexer.Next();
		if( !ParseExpression( expression.data() ) )
			return false;
	}
	else if( IsBinaryOperator( _lexer.PeekNextTokenType() ) ) {
		_lexer.Next();
		QScopedPointer< Context > binopExpression( new Context( _lexer.CurrentString() ) ); {
			_lexer.Next();
			if( !ParseExpression( binopExpression.data() ) )
				return false;
			binopExpression->Append( expression.take() );
			context->Append( binopExpression.take() );
		}
		return true;
	}

	context->Append( expression.take() );
	return true;
}

bool ASTParser::ParseTableConstructor( Context* context )
{
	// Create AST block, if parsing done, append as child to context
	QScopedPointer< Context >tableExpression( new Context( "table construct" ) );

	// empty table
	if( _lexer.Next() == L'}' ) {
		context->Append( tableExpression.take() );
		return true;
	}

	forever {
		if( !ParseFieldDefine( tableExpression.data() ) )
			return false;

		int tokenType = _lexer.Next();
		if( tokenType == L',' || tokenType == L';' ) {
			if( _lexer.Next() == L'}' ) {
				_lexer.Next();
				break;
			}
		}
		else if( tokenType == L'}' ) {
			break;
		}
	}

	context->Append( tableExpression.take() );
	return true;
}

bool ASTParser::ParseAnonymousFunction( Context* context )
{
	return false;
}

/// tablename.field1[field2]['field3']:selfCall()
bool ASTParser::ParsePrefixExpression( Context* context )
{
	if( _lexer.CurrentTokenType() != Lexer::TK_NAME )
		return false;

	// Create AST block, if parsing done, append as child to context
	QScopedPointer< Context > prefixExpression( new Context( "prefix expression" ) );
	prefixExpression->Append( _lexer.CurrentString() );

	_lexer.Next(); // skip Name
	if( ParsePrefixSubExpression( prefixExpression.data() ) == Parse_Fail )
		return false;

	context->Append( prefixExpression.take() );
	return true;
}

ASTParser::ParseResult ASTParser::ParsePrefixSubExpression( Context* context )
{
	// Create AST block, if parsing done, append as child to context
	QScopedPointer< Context > prefixExpression( new Context( "prefix expression" ) );
	switch( _lexer.CurrentTokenType() ) {
	case L'.': {
		if( _lexer.Next() != Lexer::TK_NAME ) // skip dot
			return Parse_Fail;
		prefixExpression->Append( _lexer.CurrentString() );
		context->Append( prefixExpression.take() );

		_lexer.Next();
		return ParsePrefixSubExpression( prefixExpression.data() );
		break;
	}
	case L'[':
		_lexer.Next(); // skip brace [
		if( !ParseExpression( prefixExpression.data() ) )
			return Parse_Fail;
		if( _lexer.Next() != L']' )
			return Parse_Fail;
		break;
	case L':' : {
		// advance to next Name
		if( _lexer.Next() != Lexer::TK_NAME )
			return Parse_Fail;
		prefixExpression->Append( _lexer.CurrentString() );
		_lexer.Next(); // skip name

		QScopedPointer< Context > argPrefixExpression( new Context( "prefix expression" ) );
		// after self named item
		// should be function call
		if( ParseFunctionArgs( argPrefixExpression.data() ) != Parse_OK )
			return Parse_Fail;

		prefixExpression->Append( argPrefixExpression.take() );
		break;
	}
	default:
		switch( ParseFunctionArgs( prefixExpression.data() ) )
		case Parse_Mismatch :
			return Parse_OK;
		case Parse_Fail :
			return Parse_Fail;
	}

	context->Append( prefixExpression.data() );
	return ParsePrefixSubExpression( prefixExpression.take() );
}

ASTParser::ParseResult ASTParser::ParseFunctionArgs( Context* context )
{
	// Create AST block, if parsing done, append as child to context
	QScopedPointer< Context > args( new Context( "args" ) );

	switch( _lexer.PeekNextTokenType() ) {
	case L'(' :
		_lexer.Next();
		if( _lexer.PeekNextTokenType() == L')' ) {
			_lexer.Next(); // skip ')'
			context->Append( args.take() );
			return Parse_OK;
		}
		else if( !ParseExpressionList( args.data() ) )
			return Parse_Fail;

		if( _lexer.Next() != L')' ) {
			return Parse_Fail;
		}
		break;

	case Lexer::TK_STRING :
		_lexer.Next();
		args->Append( "string" );
		break;

	case L'{' :
		_lexer.Next();
		if( !ParseTableConstructor( args.data() ) ) {
			return Parse_Fail;
		}
	default:
		return Parse_Mismatch;
	}

	context->Append( args.take() );
	return Parse_OK;
}

bool ASTParser::ParseFieldDefine( Context* context )
{
	// Create AST block, if parsing done, append as child to context
	QScopedPointer< Context > field( new Context( "field" ) );

	if( _lexer.CurrentTokenType() == L'[' ) {
		_lexer.Next(); // advance to expression
		if( !ParseExpression( field.data() ) )
			return false;
		if( _lexer.Next() != L']' )
			return false;
	}
	else if( _lexer.CurrentTokenType() == Lexer::TK_NAME
			 && _lexer.PeekNextTokenType() == L'=' ) {
		_lexer.Next(); // skip '='
		field->Name = "field assign";
		_lexer.Next(); // advance to expression
		if( !ParseExpression( field.data() ) ) {
			return false;
		}
	}
	else if( !ParseExpression( field.data() ) ) {
		return false;
	}

	context->Append( field.take() );
	return true;
}

void ASTParser::GenerateError()
{
	_error = QString( "Unexpected token '%1' at line: %2, pos %3" )
			.arg( _lexer.CurrentString() )
			.arg( _lexer.CurrentLine() )
			.arg( _lexer.CurrentPos() );
	qWarning( "ASTParser :: %s", qPrintable( _error ) );
}
