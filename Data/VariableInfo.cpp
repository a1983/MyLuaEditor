#include "VariableInfo.h"

//
VariableInfo::VariableInfo() :
	_parent( nullptr )
{

}

VariableInfo::~VariableInfo()
{
	if( _parent )
		qFatal( "VariableInfo :: destroing item with parent is permitted!" );

	foreach( VariableInfo* item, _children ) {
		item->_parent = nullptr;
		delete item;
	}
}

//
void VariableInfo::SetText( const QString& text )
{
	_properties[ TextProperty ].setValue( text );
}

const QString VariableInfo::Text() const
{
	return _properties[ TextProperty ].toString();
}

//
VariableInfo* VariableInfo::Parent()
{
	return _parent;
}

void VariableInfo::AppendChild( VariableInfo* child )
{
	if( child->_parent )
		qFatal( "VariableInfo :: appending child with parent is permitted!" );

	child->_parent = this;

	_children.append( child );
	_indexByChild.insert( child, _children.size() - 1 );
}

VariableInfo* VariableInfo::Child( int index )
{
	return _children.value( index );
}

int VariableInfo::SiblingPos() const
{
	if( _parent )
		return _parent->_indexByChild.value( const_cast< VariableInfo* >( this ) );
	return 0;
}

int VariableInfo::ChildrenCount() const
{
	return _children.size();
}
