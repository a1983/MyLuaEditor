#include "AstInfo.h"

QString AstTypeText( AstInfo::Type type )
{
	switch( type ) {
	case AstInfo::Global :				return "Global";

	case AstInfo::Block :				return "Block";

	case AstInfo::DoStatement :			return "DoStatement";
	case AstInfo::WhileStatement :		return "WhileStatement";
	case AstInfo::RepeatStatement :		return "RepeatStatement";
	case AstInfo::IfStatement :			return "IfStatement";
	case AstInfo::ForIndexStatement :	return "ForIndexStatement";
	case AstInfo::ForIteratorStatement :return "ForIteratorStatement";
	case AstInfo::FunctionStatement :	return "FunctionStatement";
	case AstInfo::LocalStatement :		return "LocalStatement";
	case AstInfo::CallStatement :		return "CallStatement";
	case AstInfo::AssignStatement :		return "AssignStatement";
	case AstInfo::ReturnStatement :		return "ReturnStatement";
	case AstInfo::BreakStatement :		return "BreakStatement";

	case AstInfo::Prefix :				return "Prefix";
	case AstInfo::Name :				return "Name";
	case AstInfo::Dots :				return "Dots";

	case AstInfo::ExpressionList :		return "ExpressionList";
	case AstInfo::Expression :			return "Expression";

	case AstInfo::Literal :				return "Literal";

	case AstInfo::Args :				return "Args";
	case AstInfo::Constructor :			return "Constructor";
	case AstInfo::Field :				return "Field";

	case AstInfo::UnaryOperator :		return "UnaryOperator";
	case AstInfo::BinaryOperator :		return "BinaryOperator";

	case AstInfo::FunctionBody :		return "FunctionBody";
	default:
		return QString( "Unknown type: %1" ).arg( type );
	}
}
