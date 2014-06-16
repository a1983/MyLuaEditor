#ifndef ASTITEM_H
#define ASTITEM_H

#include <QLinkedList>

class AstItem
{
public:
    AstItem( AstItem* parent );
    ~AstItem();

    bool HasParent() const;
    bool HasSiblings() const;
    bool HasChildren() const;

    int SiblingPos() const;

private:
    int _pos;
    int _size;

    AstItem*   _parent;
    QLinkedList< AstItem* > _children;
};

#endif // ASTITEM_H
