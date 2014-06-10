#include "AstInfo.h"

QString AstTypeText( AstInfo::Type type )
{
	switch( type ) {
	case AstInfo::Global :			return "Global";

	case AstInfo::Block :			return "Block";
	case AstInfo::Statement :		return "Statement";
	case AstInfo::Prefix :			return "Prefix";
	case AstInfo::Name :			return "Name";
	case AstInfo::Dots :			return "Dots";

	case AstInfo::ExpressionList :	return "ExpressionList";
	case AstInfo::Expression :		return "Expression";

	case AstInfo::Call :			return "Call";
	case AstInfo::Assign :			return "Assign";

	case AstInfo::Literal :			return "Literal";

	case AstInfo::Args :			return "Args";
	case AstInfo::Constructor :		return "Constructor";
	case AstInfo::Field :			return "Field";

	case AstInfo::UnaryOperator :	return "UnaryOperator";
	case AstInfo::BinaryOperator :	return "BinaryOperator";

	case AstInfo::FunctionBody :	return "FunctionBody";
	default:
		return QString( "Unknown type: %1" ).arg( type );
	}
}
