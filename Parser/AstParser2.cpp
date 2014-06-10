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

QString AstParser2::Debug()
{
	return _global.DebugString();
}

bool AstParser2::TryBlock( AstItem* item )
{
	QScopedPointer< AstItem > block( new AstItem( AstInfo::Block ) );
	while( TryStatement( block.data() ) ) {
		if( HasError() )
			return false;
	}

	TryLastStatement( block.data() );
	if( HasError() )
		return false;

	// No statement, invalid block
	if( !block->HasChildren() ) {
		GenerateError( "Empty statement" );
		return false;
	}

	item->AppendChild( block.take() );
	return true;
}

bool AstParser2::TryStatement( AstItem* item )
{
	// Try do block end | 													# DO
	// Try while exp do block end | 										# WHILE
	// Try repeat block until exp | 										# REPEAT
	// Try if exp then block {elseif exp then block} [else block] end |		# IF
	// Try for Name `=´ exp `,´ exp [`,´ exp] do block end |				# FOR
	// Try for namelist in explist do block end | 							# FOR
	// Try function funcname funcbody | 									# DEFINE FUNCTION
	// Try local function Name funcbody | 									# LOCAL DEFINE FUNCTION
	// Try local namelist [`=´ explist] 									# LOCAL DEFINE

	// Function call or global assigment
	if( !TryCallOrAssign( item ) ) {
		return false;
	}

	// Skip ending ';'
	if( _advance.CurrentType() == TT_SEMICOLON ) {
		_advance.Next();
		Confirm();
	}

	return true;
}

bool AstParser2::TryLastStatement( AstItem* item )
{
	return false;
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
		callOrAssign->Info.AstType = AstInfo::Call;
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
		if( callOrAssign->LastChild()->Info.AstType == AstInfo::Call ) {
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

	callOrAssign->Info.AstType = AstInfo::Assign;
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
		GenerateError( "Expected '('" );
		return false;
	}
	_advance.Next();
	Confirm();

	TryFunctionParams( functionBody.data() );
	if( HasError() )
		return false;

	if( _advance.CurrentType() != TT_RIGHT_BRACKET ) {
		GenerateError( "Expected ')'" );
		return false;
	}
	_advance.Next();
	Confirm();

	TryBlock( functionBody.data() );
	if( HasError() )
		return false;

	if( _advance.CurrentType() != TT_END ) {
		GenerateError( "Expected 'end' statement" );
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

void AstParser2::Confirm()
{
	_current = _advance;
}

void AstParser2::Decline()
{
	_advance = _current;
}

void AstParser2::GenerateError( const QString& description )
{
	if( _error.size() > 0 )
		qFatal( "error rethrow" );

	QString error( "Error: " );
	error.append( description ).append( "\n" )
			.append( "at pos: " ).append( QString::number( _current.CurrentPos() ) )
			.append( ", line: " ).append( QString::number( _current.CurrentLine() ) )
			.append( "\ntext: ").append( _current.CurrentString() );

	_error = error;
	qDebug( qPrintable( error ) );
}
