#ifndef ASTINFO_H
#define ASTINFO_H

#include <QString>

struct AstInfo {
	enum Type {
		Unknown = -1,

		Global,

		Block,

		DoStatement,
		WhileStatement,
		RepeatStatement,
		IfStatement,
		ForIndexStatement,
		ForIteratorStatement,
		FunctionStatement,
		LocalStatement,
		CallStatement,
		AssignStatement,
		ReturnStatement,
		BreakStatement,

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

	Type	AstType;

	QString Text;

	int		Priority;

	AstInfo() :
		AstType( Unknown ),
		Priority( -1 ) {}
};

QString AstTypeText( AstInfo::Type type );

#endif // ASTINFO_H
