#ifndef ASTITEM_H
#define ASTITEM_H

#include <QList>
#include <QString>

#include "AstInfo.h"

class Lexer2;

class AstItem
{
public:
	explicit AstItem( AstItem* parent = nullptr );
	explicit AstItem( const AstInfo::Type& type, AstItem* parent = nullptr );
	explicit AstItem( const AstInfo::Type& type, const Lexer2& lexer );
	~AstItem();

	void AppendChild( AstItem* child );

	void SwapInfo( AstItem* other );

	const AstItem* Child( int index ) const;
	const AstItem* LastChild() const;

	void SetType( AstInfo::Type type );
	QString TypeText() const;

	void SetText( const QString& text );
	QString Text() const;

	void SetPriority( int priority );
	int Priority() const;

	bool	Is( AstInfo::Type type ) const;
	bool	HasParent() const;
	bool	HasSiblings() const;
	bool	HasChildren() const;
	int		ChildrenCount() const;

	AstItem* Parent();

	int SiblingPos() const;
	int ChildrenPos( const AstItem* child ) const;

	QString DebugString( int level = 0 ) const;

private:
	AstInfo Info;

	AstItem*				_parent;
	QList< const AstItem* > _children;
};

#endif // ASTITEM_H
