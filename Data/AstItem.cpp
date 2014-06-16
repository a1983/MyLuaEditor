#include "AstItem.h"

#include <QtAlgorithms>

AstItem::AstItem( AstItem* parent ) :
    _pos( -1 ),
    _size( -1 ),

    _parent( parent )
{
    if( _parent ) {
        _parent->_children.append( this );
    }
}

AstItem::~AstItem()
{
    qDeleteAll( _children );
}

bool AstItem::HasParent() const
{
    return _parent;
}

bool AstItem::HasSiblings() const
{
   return _parent->_children.size() > 1;
}

bool AstItem::HasChildren() const
{
    return _children.size() > 0;
}

int AstItem::SiblingPos() const
{
    if( !_parent )
        return 0;


}
