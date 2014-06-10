#ifndef ASTINFO_H
#define ASTINFO_H

#include <QString>

struct AstInfo {
	enum Type {
		Global,

		Block,
		Statement,

		Call,
		Assign,

		Prefix,
		Name,
		Dots,
		ExpressionList,
		Expression,

		Args,
		Constructor,
		Field,
		Literal,
		UnaryOperator,
		BinaryOperator,

		FunctionBody
	};
	Type AstType;
};

QString AstTypeText( AstInfo::Type type );

#endif // ASTINFO_H
