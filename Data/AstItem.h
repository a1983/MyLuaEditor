#ifndef ASTITEM_H
#define ASTITEM_H

#include <QList>
#include <QString>

#include "AstInfo.h"

class AstItem
{
public:
	explicit AstItem( AstItem* parent = nullptr );
	explicit AstItem( const AstInfo::Type& type, AstItem* parent = nullptr );
	~AstItem();

	void AppendChild( AstItem* child );

	const AstItem* Child( int index ) const;
	const AstItem* LastChild();

	AstInfo Info;

	bool Is( AstInfo::Type type ) const;

	bool HasParent() const;
	bool HasSiblings() const;
	bool HasChildren() const;

	AstItem* Parent();

	int SiblingPos() const;
	int ChildrenPos( const AstItem* child ) const;

	QString DebugString( int level = 0 ) const;

private:
	AstItem*				_parent;
	QList< const AstItem* > _children;
};

#endif // ASTITEM_H
