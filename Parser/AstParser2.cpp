#include "AstParser2.h"

#include <QScopedPointer>
#include <QString>

AstParser2::AstParser2( const QString& source ) :
	_source ( source ),
	_current(),
	_advance(),

	_global( AstInfo::Global )
{
	_current = Lexer2( &_source ),
	_advance = _current;
}

bool AstParser2::Parse()
{
	if( !TryBlock( &_global ) )
		return false;

	// No lua statements, invalid source
	if( !_global.HasChildren() ) {
		GenerateError( "Empty source" );
		return false;
	}

	if( _current.CurrentType() != TT_END_OF_FILE ) {
		GenerateError( "Expected end of file" );
		return false;
	}

	return true;
}

bool AstParser2::HasError() const
{
	return _error.size() > 0;
}

QString AstParser2::Error() const
{
	return _error;
}

AstItem* AstParser2::Result()
{
	return &_global;
}

QString AstParser2::Debug()
{
	return _global.DebugString();
}

bool AstParser2::TryBlock( AstItem* item )
{
	QScopedPointer< AstItem > block( new AstItem( AstInfo::Block ) );
	while( TryStatement( block.data() ) ) {
		// Skip ending ';'
		if( _advance.CurrentType() == TT_SEMICOLON ) {
			_advance.Next();
			Confirm();
		}
	}
	if( HasError() )
		return false;

	TryLastStatement( block.data() );
	if( HasError() )
		return false;

	item->AppendChild( block.take() );
	return true;
}

bool AstParser2::TryStatement( AstItem* item )
{
	switch( _advance.CurrentType() ) {
	// Try do block end | 													# DO
	case TT_DO : {
		if( TryDoStatement( item ) )
			return true;
		break;
	}
	// Try while exp do block end | 										# WHILE
	case TT_WHILE : {
		if( TryWhileStatement( item ) )
			return true;
		break;
	}
	// Try repeat block until exp | 										# REPEAT
	case TT_REPEAT : {
		if( TryRepeatStatement( item ) )
			return true;
		break;
	}
	// Try if exp then block {elseif exp then block} [else block] end |		# IF
	case TT_IF : {
		if( TryIfStatement( item ) )
			return true;
		break;
	}
	// Try for Name `=´ exp `,´ exp [`,´ exp] do block end |				# FOR
	// Try for namelist in explist do block end | 							# FOR
	case TT_FOR : {
		if( TryForStatement( item ) )
			return true;
		break;
	}
	// Try function funcname funcbody | 									# DEFINE GLOBAL FUNCTION
	case TT_FUNCTION : {
		if( TryFunctionStatement( item ) )
			return true;
		break;
	}
	// Try local function Name funcbody | 									# DEFINE LOCAL FUNCTION
	// Try local namelist [`=´ explist] 									# DEFINE LOCAL VARLIST
	case TT_LOCAL : {
		if( TryLocalStatement( item ) )
			return true;
		break;
	}
	default:
		// Function call or global assigment
		if( TryCallOrAssign( item ) ) {
			return true;
		}
	}

	return false;
}

bool AstParser2::TryLastStatement( AstItem* item )
{
	switch( _advance.CurrentType() ) {
	case TT_RETURN : {
		_advance.Next(); // skip 'return' keyword
		Confirm();

		QScopedPointer< AstItem > returnStatement( new AstItem( AstInfo::ReturnStatement ) );
		TryExpressionList( returnStatement.data() );
		if( HasError() )
			return false;
		item->AppendChild( returnStatement.take() );
		return true;
	}
	case TT_BREAK : {
		_advance.Next(); // skip 'break' keyword
		Confirm();

		new AstItem( AstInfo::BreakStatement, item );
		return true;
	}
	}
	return false;
}

bool AstParser2::TryDoStatement( AstItem* item )
{
	QScopedPointer< AstItem > doStatement( new AstItem( AstInfo::DoStatement ) );
	_advance.Next(); // skip 'do' keyword
	Confirm();

	TryBlock( doStatement.data() );
	if( HasError() )
		return false;

	if( _advance.CurrentType() != TT_END ) {
		GenerateError( "Expected 'end' statement to close 'do'" );
		return false;
	}
	_advance.Next(); // skip 'end' keyword
	Confirm();

	item->AppendChild( doStatement.take() );
	return true;
}

bool AstParser2::TryWhileStatement( AstItem* item )
{
	QScopedPointer< AstItem > whileStatement( new AstItem( AstInfo::WhileStatement ) );
	_advance.Next(); // skip 'while' keyword
	Confirm();

	if( !TryExpression( whileStatement.data() ) ) {
		if( !HasError() )
			GenerateError( "Expected expression after 'while' keyword" );
		return false;
	}

	if( _advance.CurrentType() != TT_DO ) {
		GenerateError( "Expected 'do' statement before 'while' block body" );
		return false;
	}
	_advance.Next(); // skip 'do' keyword
	Confirm();

	TryBlock( whileStatement.data() );
	if( HasError() )
		return false;

	if( _advance.CurrentType() != TT_END ) {
		GenerateError( "Expected 'end' statement to close 'while'" );
		return false;
	}
	_advance.Next(); // skip 'end' keyword
	Confirm();

	item->AppendChild( whileStatement.take() );
	return true;
}

bool AstParser2::TryRepeatStatement( AstItem* item )
{
	QScopedPointer< AstItem > repeatStatement( new AstItem( AstInfo::RepeatStatement ) );
	_advance.Next(); // skip 'repeat' keyword
	Confirm();

	TryBlock( repeatStatement.data() );
	if( HasError() )
		return false;

	if( _advance.CurrentType() != TT_UNTIL ) {
		GenerateError( "Expected 'until' statement to close 'repeat'" );
		return false;
	}
	_advance.Next(); // skip 'until' keyword
	Confirm();

	if( !TryExpression( repeatStatement.data() ) ) {
		if( !HasError() )
			GenerateError( "Expected expression after 'until' keyword" );
		return false;
	}

	item->AppendChild( repeatStatement.take() );
	return true;
}

bool AstParser2::TryIfStatement( AstItem* item )
{
	QScopedPointer< AstItem > ifStatement( new AstItem( AstInfo::IfStatement ) );
	_advance.Next(); // skip 'if' keyword
	Confirm();

	if( !TryExpression( ifStatement.data() ) ) {
		if( !HasError() )
			GenerateError( "Expected expression after 'if' keyword" );
		return false;
	}

	if( _advance.CurrentType() != TT_THEN ) {
		GenerateError( "Expected 'then' statement after 'if' expression" );
		return false;
	}
	_advance.Next(); // skip 'then' keyword
	Confirm();

	TryBlock( ifStatement.data() );
	if( HasError() )
		return false;

	while( _advance.CurrentType() == TT_ELSEIF ) {
		_advance.Next(); // skip 'elseif' keyword
		Confirm();

		if( !TryExpression( ifStatement.data() ) ) {
			if( !HasError() )
				GenerateError( "Expected expression after 'elseif' keyword" );
			return false;
		}

		if( _advance.CurrentType() != TT_THEN ) {
			GenerateError( "Expected 'then' statement after 'elseif' expression" );
			return false;
		}
		_advance.Next(); // skip 'then' keyword
		Confirm();

		TryBlock( ifStatement.data() );
		if( HasError() )
			return false;
	}

	if( _advance.CurrentType() == TT_ELSE ) {
		_advance.Next(); // skip 'else' keyword
		Confirm();

		TryBlock( ifStatement.data() );
		if( HasError() )
			return false;
	}

	if( _advance.CurrentType() != TT_END ) {
		GenerateError( "Expected 'end' statement to close 'if' statement" );
		return false;
	}
	_advance.Next(); // skip 'end' keyword
	Confirm();

	item->AppendChild( ifStatement.take() );
	return true;
}

bool AstParser2::TryForStatement( AstItem* item )
{
	QScopedPointer< AstItem > forStatement( new AstItem( AstInfo::ForIndexStatement ) );
	_advance.Next(); // skip 'for' keyword
	Confirm();

	if( !ShouldNameList( forStatement.data() ) )
		return false;

	if( forStatement->ChildrenCount() > 1 ) {
		// iterator
		forStatement->SetType( AstInfo::ForIteratorStatement );

		if( _advance.CurrentType() != TT_IN ) {
			GenerateError( "Expected 'in' after namelist in 'for' statement" );
			return false;
		}
		_advance.Next(); // skip 'in' keyword
		Confirm();

		if( !TryExpressionList( forStatement.data() ) ) {
			if( !HasError() )
				GenerateError( "Expected expressionlist after 'in'' keyword in 'for' statement" );
			return false;
		}
	}
	else {
		// index
		if( _advance.CurrentType() != TT_ASSIGN ) {
			GenerateError( "Expected '=' after name in 'for' statement" );
			return false;
		}
		_advance.Next(); // skip '='
		Confirm();

		if( !TryExpression( forStatement.data() ) ) {
			if( !HasError() )
				GenerateError( "Expected expression after '=' in for statement" );
			return false;
		}

		if( _advance.CurrentType() != TT_COMMA ) {
			GenerateError( "Expected '=' after name in 'for' statement" );
			return false;
		}
		_advance.Next(); // skip ','
		Confirm();

		if( !TryExpression( forStatement.data() ) ) {
			if( !HasError() )
				GenerateError( "Expected expression after ',' in for statement" );
			return false;
		}

		// last expression - step
		if( _advance.CurrentType() == TT_COMMA ) {
			_advance.Next(); // skip ','
			Confirm();
			if( !TryExpression( forStatement.data() ) ) {
				if( !HasError() )
					GenerateError( "Expected expression after ',' in for statement" );
				return false;
			}
		}
	}

	// for body
	if( _advance.CurrentType() != TT_DO ) {
		GenerateError( "Expected 'do' statement before 'for' block body" );
		return false;
	}
	_advance.Next(); // skip 'do' keyword
	Confirm();

	TryBlock( forStatement.data() );
	if( HasError() )
		return false;

	if( _advance.CurrentType() != TT_END ) {
		GenerateError( "Expected 'end' statement to close 'for' statement" );
		return false;
	}
	_advance.Next(); // skip 'end' keyword
	Confirm();

	item->AppendChild( forStatement.take() );
	return true;
}

bool AstParser2::TryFunctionStatement( AstItem* item )
{
	QScopedPointer< AstItem > functionStatement( new AstItem( AstInfo::FunctionStatement ) );
	_advance.Next(); // skip 'function' keyword
	Confirm();

	if( _advance.CurrentType() != TT_NAME ) {
		GenerateError( "Expected name after 'function' keyword" );
		return false;
	}
	functionStatement->AppendChild( new AstItem( AstInfo::Name ) );
	_advance.Next(); // skip name
	Confirm();

	while( _advance.CurrentType() == TT_POINT ) {
		_advance.Next(); // skip '.'
		Confirm();

		if( _advance.CurrentType() != TT_NAME ) {
			GenerateError( "Expected name after '.' in 'function' statement" );
			return false;
		}
		functionStatement->AppendChild( new AstItem( AstInfo::Name ) );
		_advance.Next(); // skip name
		Confirm();
	}

	if( _advance.CurrentType() == TT_COLON ) {
		_advance.Next(); // skip ':'
		Confirm();

		if( _advance.CurrentType() != TT_NAME ) {
			GenerateError( "Expected name after ':' in 'function' statement" );
			return false;
		}
		functionStatement->AppendChild( new AstItem( AstInfo::Name ) );
		_advance.Next(); // skip name
		Confirm();
	}

	if( !ShouldFunctionBody( functionStatement.data() ) )
		return false;

	item->AppendChild( functionStatement.take() );
	return true;
}

bool AstParser2::TryLocalStatement( AstItem* item )
{
	QScopedPointer< AstItem > localStatement( new AstItem( AstInfo::LocalStatement ) );
	_advance.Next(); // skip 'local' keyword
	Confirm();

	if( _advance.CurrentType() == TT_FUNCTION ) {
		// single local function define
		_advance.Next(); // skip 'function' keyword
		Confirm();

		if( _advance.CurrentType() != TT_NAME ) {
			GenerateError( "Expected name after ':' in 'function' statement" );
			return false;
		}
		localStatement->AppendChild( new AstItem( AstInfo::Name ) );
		_advance.Next(); // skip name
		Confirm();

		if( !ShouldFunctionBody( localStatement.data() ) )
			return false;
	}
	else {
		// local assignment or define
		if( !ShouldNameList( localStatement.data() ) )
			return false;

		if( _advance.CurrentType() == TT_ASSIGN ) {
			_advance.Next(); // skip '='
			Confirm();
			if( !TryExpressionList( localStatement.data() ) ) {
				if( !HasError() )
					GenerateError( "Expected expression list after '=' in local assignment" );
				return false;
			}
		}
	}

	item->AppendChild( localStatement.take() );
	return true;
}

bool IsCall( const AstItem* item ) {
	const AstItem* suffix = item->Child( 1 );
	if( suffix && suffix->Is( AstInfo::Prefix ) )
		return IsCall( suffix );

	const AstItem* args = item->Child( 0 );
	return args && args->Is( AstInfo::Args );
}

bool AstParser2::TryCallOrAssign( AstItem* item )
{
	QScopedPointer< AstItem > callOrAssign( new AstItem() );
	if( !TryPrefixExpression( callOrAssign.data() ) )
		return false;

	if( IsCall( callOrAssign->Child( 0 ) ) ) {
		callOrAssign->Info.AstType = AstInfo::CallStatement;
		item->AppendChild( callOrAssign.take() );
		return true;
	}

	// else var list
	while( _advance.CurrentType() == TT_COMMA )
	{
		_advance.Next();
		Confirm();

		if( !TryPrefixExpression( callOrAssign.data() ) )
			return false;
		if( callOrAssign->LastChild()->Info.AstType == AstInfo::CallStatement ) {
			GenerateError( "Wrong call" );
			return false;
		}
	}
	//
	// should be assigned to
	if( _advance.CurrentType() != TT_ASSIGN ) {
		GenerateError( "Expected '='" );
		return false;
	}
	_advance.Next();
	Confirm();

	//
	// expression list
	if( !TryExpressionList( callOrAssign.data() ) ) {
		if( !HasError() ) {
			GenerateError( "Expected expression list" );
		}
		return false;
	}

	callOrAssign->Info.AstType = AstInfo::AssignStatement;
	item->AppendChild( callOrAssign.take() );
	return true;
}

bool AstParser2::TryPrefixExpression( AstItem* item )
{
	QScopedPointer< AstItem > prefix( new AstItem( AstInfo::Prefix ) );
	// Can be started from Name or '('
	switch( _advance.CurrentType() ) {
	case TT_NAME : {
		new AstItem( AstInfo::Name, prefix.data() );
		_advance.Next();
		Confirm();
		break;
	}
	case TT_LEFT_BRACKET : {
		_advance.Next();
		Confirm();
		if( !TryExpression( prefix.data() ) ) {
			GenerateError( "Wrong bracket" );
			return false;
		}

		if( _advance.CurrentType() != TT_RIGHT_BRACKET ) {
			GenerateError( "Expected ')'" );
			return false;
		}
		_advance.Next();
		Confirm();
		break;
	}
	default :
		return false;
	}

	TryPrefixSubExpression( prefix.data() );
	if( HasError() )
		return false;

	item->AppendChild( prefix.take() );
	return true;
}

bool AstParser2::TryPrefixSubExpression( AstItem* item )
{
	QScopedPointer< AstItem > prefix( new AstItem( AstInfo::Prefix ) );
	switch( _advance.CurrentType() ) {
	case TT_POINT : {
		if( _advance.Next() != TT_NAME ) {
			GenerateError( "Name expected" );
			return false;
		}

		new AstItem( AstInfo::Name, prefix.data() );
		_advance.Next();
		Confirm();

		TryPrefixSubExpression( prefix.data() );
		break;
	}
	case TT_LEFT_SQUARE : {
		_advance.Next();
		Confirm();

		if( !TryExpression( prefix.data() ) ) {
			if( !HasError() )
				GenerateError( "Expected expression" );
			return false;
		}

		if( _advance.CurrentType() != TT_RIGHT_SQUARE ) {
			GenerateError( "Expected ']'" );
			return false;
		}

		_advance.Next();
		Confirm();

		TryPrefixSubExpression( prefix.data() );
		break;
	}
	case TT_COLON : {
		// Call with self
		if( _advance.Next() != TT_NAME ) {
			GenerateError( "Name expected" );
			return false;
		}

		new AstItem( AstInfo::Name, prefix.data() );
		_advance.Next();
		Confirm();

		AstItem* args = new AstItem( AstInfo::Prefix, prefix.data() );
		if( !TryArgs( args ) ) {
			if( !HasError() )
				GenerateError( "Expected function call" );
			return false;
		}

		TryPrefixSubExpression( args );
		break;
	}
	default:
		if( TryArgs( prefix.data() ) )
			TryPrefixSubExpression( prefix.data() );
		else
			return false;
	}

	if( HasError() )
		return false;

	item->AppendChild( prefix.take() );
	return true;
}

bool AstParser2::TryArgs( AstItem* item )
{
	QScopedPointer< AstItem > args( new AstItem( AstInfo::Args ) );

	switch( _advance.CurrentType() ) {
	case TT_LEFT_BRACKET : {
		_advance.Next();
		Confirm();

		TryExpressionList( args.data() );
		if( HasError() )
			return false;

		if( _advance.CurrentType() != TT_RIGHT_BRACKET ) {
			GenerateError( "Expected ')'" );
			return false;
		}

		_advance.Next();
		Confirm();
		break;
	}
	case TT_LEFT_CURLY : {
		_advance.Next();
		Confirm();

		if( !TryConstructor( args.data() ) && HasError() )
			return false;

		if( _advance.CurrentType() != TT_RIGHT_CURLY ) {
			GenerateError( "Expected '}'" );
			return false;
		}

		_advance.Next();
		Confirm();
		break;
	}
	case TT_STRING : {
		new AstItem( AstInfo::Literal, args.data() );
		_advance.Next();
		Confirm();
		break;
	}
	default:
		return false;
	}

	item->AppendChild( args.take() );
	return true;
}

bool AstParser2::TryConstructor( AstItem* item )
{
	QScopedPointer< AstItem > constructor( new AstItem( AstInfo::Constructor ) );

	while( TryField( constructor.data() ) ) {
		if( _advance.CurrentType() == TT_COMMA
			|| _advance.CurrentType() == TT_SEMICOLON ) {
			_advance.Next();
			Confirm();
		}
	}

	if( HasError() )
		return false;

	item->AppendChild( constructor.take() );

	return true;
}

bool AstParser2::TryField( AstItem* item )
{
	QScopedPointer< AstItem > field( new AstItem( AstInfo::Field ) );
	if( _advance.CurrentType() == TT_LEFT_SQUARE ) {
		_advance.Next();
		if( !TryExpression( field.data() ) ) {
			if( !HasError() )
				GenerateError( "Expected expression" );
			return false;
		}

		if( _advance.CurrentType() != TT_RIGHT_SQUARE ) {
			GenerateError( "Expected ']'" );
			return false;
		}
		_advance.Next();

		if( _advance.CurrentType() != TT_ASSIGN ) {
			GenerateError( "Expected '='" );
			return false;
		}
		_advance.Next();

		if( !TryExpression( field.data() ) ) {
			if( !HasError() )
				GenerateError( "Expected expression" );
			return false;
		}
	}
	else if( _advance.CurrentType() == TT_NAME ) {
		new AstItem( AstInfo::Name, field.data() );

		_advance.Next();
		if( _advance.CurrentType() != TT_ASSIGN ) {
			GenerateError( "Expected '='" );
			return false;
		}
		_advance.Next();

		if( !TryExpression( field.data() ) ) {
			if( !HasError() )
				GenerateError( "Expected expression" );
			return false;
		}
	}
	else if( !TryExpression( field.data() ) ) {
		return false;
	}

	item->AppendChild( field.take() );
	return true;
}

bool AstParser2::TryExpressionList( AstItem* item )
{
	QScopedPointer< AstItem > list( new AstItem( AstInfo::ExpressionList ) );

	if( !TryExpression( list.data() ) )
		return false;

	while( _advance.CurrentType() == TT_COMMA ) {
		_advance.Next();
		Confirm();
		if( !TryExpression( list.data() ) ) {
			if( !HasError() )
				GenerateError( "Expected Expression" );
			return false;
		}
	}

	item->AppendChild( list.take() );
	return true;
}


bool IsBinaryOperator( TokenType type ) {
	switch( type ) {
	case TT_PLUS : case TT_MINUS : case TT_MAGNIFY : case TT_SLASH : case TT_CARET : case TT_PERCENT :
	case TT_LESS : case TT_GREAT : case TT_LESS_OR_EQUAL : case TT_GREAT_OR_EQUAL : case TT_EQUAL : case TT_NOT_EQUAL :
	case TT_AND : case TT_OR: case TT_CONCAT :
		return true;
	}

	return false;
}

bool AstParser2::TryExpression( AstItem* item )
{
	QScopedPointer< AstItem > expression( new AstItem( AstInfo::Expression ) );

	switch( _advance.CurrentType() ) {
	case TT_NOT : case TT_MINUS : case TT_NUMBER_SIGN : {
		QScopedPointer< AstItem > unary( new AstItem( AstInfo::UnaryOperator ) );
		_advance.Next();
		Confirm();

		if( !TryExpression( unary.data() ) ) {
			if( !HasError() )
				GenerateError( "Expected expression" );
			return false;
		}
		expression->AppendChild( unary.take() );

		break;
	}
	case TT_NIL : case TT_TRUE : case TT_FALSE : case TT_DOTS :
	case TT_NUMBER : case TT_STRING : {
		expression->AppendChild( new AstItem( AstInfo::Literal ) );
		_advance.Next();
		Confirm();
		break;
	}
	case TT_FUNCTION : {
		_advance.Next();
		Confirm();

		if( !ShouldFunctionBody( expression.data() ) )
			return false;
		break;
	}
	case TT_LEFT_CURLY : {
		_advance.Next(); // skip '{'
		Confirm();

		if( !TryConstructor( expression.data() ) && HasError() )
			return false;

		if( _advance.CurrentType() != TT_RIGHT_CURLY ) {
			GenerateError( "Expected '}'" );
			return false;
		}
		_advance.Next();  // skip '}'
		Confirm();
		break;
	}
	default:
		if( !TryPrefixExpression( expression.data() ) )
			return false;
	}

	if( IsBinaryOperator( _advance.CurrentType() ) ) {
		_advance.Next();
		Confirm();

		QScopedPointer< AstItem > newBinaryExpression( new AstItem( AstInfo::Expression ) );
		AstItem* binary = new AstItem( AstInfo::BinaryOperator, newBinaryExpression.data() );
		if( !TryExpression( binary ) ) {
			if( !HasError() )
				GenerateError( "Expected expression" );
			return false;
		}

		binary->AppendChild( expression.take() );
		item->AppendChild( newBinaryExpression.take() );
		return true;
	}

	item->AppendChild( expression.take() );

	return true;
}

bool AstParser2::ShouldFunctionBody( AstItem* item )
{
	QScopedPointer< AstItem > functionBody( new AstItem( AstInfo::FunctionBody ) );

	if( _advance.CurrentType() != TT_LEFT_BRACKET ) {
		GenerateError( "Expected '(' to define arguments in function body" );
		return false;
	}
	_advance.Next();
	Confirm();

	TryFunctionParams( functionBody.data() );
	if( HasError() )
		return false;

	if( _advance.CurrentType() != TT_RIGHT_BRACKET ) {
		GenerateError( "Expected ')' to close function arguments in function body" );
		return false;
	}
	_advance.Next();
	Confirm();

	TryBlock( functionBody.data() );
	if( HasError() )
		return false;

	if( _advance.CurrentType() != TT_END ) {
		GenerateError( "Expected 'end' statement to close function body" );
		return false;
	}
	_advance.Next();
	Confirm();

	item->AppendChild( functionBody.take() );
	return true;
}

bool AstParser2::TryFunctionParams( AstItem* item )
{
	if( !TryFunctionParam( item ) )	 {
		return false;
	}

	while( _advance.CurrentType() == TT_COMMA ) {
		if( !TryFunctionParam( item ) ) {
			GenerateError( "Expected name or '...'" );
			return false;
		}
		if( item->LastChild()->Is( AstInfo::Dots ) )
			return true;
	}

	return item->HasChildren();
}

bool AstParser2::TryFunctionParam( AstItem* item )
{
	if( _advance.CurrentType() == TT_NAME )	 {
		new AstItem( AstInfo::Name, item );
		_advance.Next();
		Confirm();
		return true;
	}
	else if( _advance.CurrentType() == TT_DOTS ) {
		new AstItem( AstInfo::Dots, item );
		_advance.Next();
		Confirm();
		return true;
	}

	return false;
}

bool AstParser2::ShouldNameList( AstItem* item )
{
	if( _advance.CurrentType() != TT_NAME ) {
		GenerateError( "Expected name after 'for/local' keyword" );
		return false;
	}
	item->AppendChild( new AstItem( AstInfo::Name ) );
	_advance.Next(); // skip name
	Confirm();

	while( _advance.CurrentType() == TT_COMMA ) {
		_advance.Next(); // skip comma
		Confirm();
		if( _advance.CurrentType() != TT_NAME ) {
			GenerateError( "Expected name after ',' in 'for/local' statement" );
			return false;
		}
		item->AppendChild( new AstItem( AstInfo::Name ) );
		_advance.Next(); // skip name
		Confirm();
	}

	return true;
}

void AstParser2::Confirm()
{
	_current = _advance;
}

void AstParser2::GenerateError( const QString& description )
{
	if( _error.size() > 0 )
		qFatal( "error rethrow" );

	QString error( "Error: " );
	error.append( description ).append( "\n" )
			.append( "at pos: " ).append( QString::number( _current.CurrentPos() ) )
			.append( ", line: " ).append( QString::number( _current.CurrentLine() ) )
			.append( "\ntext: ").append( _current.CurrentType() == TT_END_OF_FILE ? "End of file" : _current.CurrentString() );

	_error = error;
	qDebug( qPrintable( error ) );
}
