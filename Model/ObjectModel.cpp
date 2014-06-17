#include "ObjectModel.h"

#include <QDebug>

#include "Data/VariableInfo.h"

ObjectModel::ObjectModel( QObject* parent ) :
	QAbstractItemModel( parent ),
	_root( nullptr )
{
}

ObjectModel::~ObjectModel()
{
	if( _root )
		delete _root;
}

void ObjectModel::SetNewRoot( VariableInfo* newRoot )
{
	if( _root )
		delete _root;

	beginResetModel();

	_root = newRoot;

	endResetModel();
}

QModelIndex ObjectModel::index( int row, int column, const QModelIndex& parent ) const
{
	if( !hasIndex( row, column, parent ) )
		return QModelIndex();

	VariableInfo* ancestor = parent.isValid()
			? static_cast< VariableInfo* >( parent.internalPointer() )
			: _root;

	if( !ancestor )
		return QModelIndex();

	VariableInfo* item = const_cast< VariableInfo* >( ancestor->Child( row ) );
	if( !item )
		return QModelIndex();
	return createIndex( row, column, item );
}

QModelIndex ObjectModel::parent( const QModelIndex& child ) const
{
	if( !child.isValid() )
		return QModelIndex();

	VariableInfo* item = static_cast< VariableInfo* >( child.internalPointer() );
	if( !item )
		return QModelIndex();

	VariableInfo* parent = item->Parent();
	if( !parent || parent == _root )
		return QModelIndex();

	return createIndex( parent->SiblingPos(), 0, parent );
}

int ObjectModel::rowCount( const QModelIndex& parent ) const
{
	if( parent.isValid() ) {
		VariableInfo* item = static_cast< VariableInfo* >( parent.internalPointer() );
		return item->ChildrenCount();
	}
	else if( _root ) {
		return _root->ChildrenCount();
	}
	else {
		return 0;
	}
}

int ObjectModel::columnCount( const QModelIndex& /*parent*/ ) const
{
	return 1;
}

bool ObjectModel::hasChildren( const QModelIndex& parent ) const
{
	if( parent.isValid() ) {
		VariableInfo* item = static_cast< VariableInfo* >( parent.internalPointer() );
		return item->ChildrenCount() > 0;
	}

	if( _root )
		return _root->ChildrenCount() > 0;

	return 0;
}

QVariant ObjectModel::data( const QModelIndex& index, int role ) const
{
	VariableInfo* item = static_cast< VariableInfo* >( index.internalPointer() );

	QVariant result;
	switch ( role ) {
	case Qt::DisplayRole :
		if( item )
			result = item->Text();
		break;
	default:
		break;
	}

	return result;
}

Qt::ItemFlags ObjectModel::flags( const QModelIndex& index) const
{
	if( !index.isValid() )
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
