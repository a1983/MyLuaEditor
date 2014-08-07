#include "AstItem.h"

#include "Lexer/Lexer2.h"

AstItem::AstItem( AstItem* parent ) :
	_parent( parent )
{
	if( _parent ) {
		_parent->_children.append( this );
	}
}

AstItem::AstItem( const AstInfo::Type& type, AstItem* parent) :
	_parent( parent )
{
	Info.AstType = type;

	if( _parent ) {
		_parent->_children.append( this );
	}
}

AstItem::AstItem( const AstInfo::Type& type, const Lexer2& lexer ) :
	_parent( nullptr )
{
	Info.AstType = type;
	Info.Text = lexer.CurrentText();
}

AstItem::~AstItem()
{
	qDeleteAll( _children );
}

void AstItem::AppendChild( AstItem* child )
{
	if( child->_parent )
		child->_parent->_children.removeOne( child );

	child->_parent = this;
	_children.append( child );
}

void AstItem::SwapInfo( AstItem* other )
{
	AstInfo swapInfo = Info;
	Info = other->Info;
	other->Info = swapInfo;
}

const AstItem* AstItem::Child( int index ) const
{
	return _children.value( index );
}

const AstItem* AstItem::LastChild() const
{
	return _children.last();
}

void AstItem::SetType( AstInfo::Type type )
{
	Info.AstType = type;
}

QString AstItem::TypeText() const
{
	return AstTypeText( Info.AstType );
}

void AstItem::SetText( const QString& text )
{
	Info.Text = text;
}

QString AstItem::Text() const
{
	return Info.Text;
}

void AstItem::SetPriority( int priority )
{
	Info.Priority = priority;
}

int AstItem::Priority() const
{
	return Info.Priority;
}

bool AstItem::Is( AstInfo::Type type ) const
{
	return type == Info.AstType;
}

bool AstItem::HasParent() const
{
	return _parent;
}

bool AstItem::HasSiblings() const
{
   return _parent && _parent->_children.size() > 1;
}

bool AstItem::HasChildren() const
{
	return _children.size() > 0;
}

int AstItem::ChildrenCount() const
{
	return _children.size();
}

AstItem* AstItem::Parent()
{
	return _parent;
}

int AstItem::SiblingPos() const
{
	if( !_parent )
		return -1;

	return _parent->ChildrenPos( this );
}

int AstItem::ChildrenPos( const AstItem* child ) const
{
	return _children.indexOf( child );
}

QString AstItem::DebugString( int level ) const
{
	QString result( level * 2, ' ' );
	result.append( AstTypeText( Info.AstType ) ).append( '\n' );
	foreach( const AstItem* child, _children ) {
		result.append( child->DebugString( level + 1 ) );
	}

	return result;
}
