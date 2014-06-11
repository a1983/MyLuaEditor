#include "CodeModel2.h"

#include <QDebug>

#include "Parser/AstParser2.h"

CodeModel2::CodeModel2( QObject* parent ) :
	QAbstractItemModel( parent )
{
	_root = new AstItem( AstInfo::Global );
	_parser = nullptr;
}

CodeModel2::~CodeModel2()
{
	delete _root;
	delete _parser;
}

void CodeModel2::RebuildModel( const QString& source )
{
	beginResetModel();
//	delete _root;
//	_root = new AstItem( "Global" );
	if( _parser )
		delete _parser;
	_parser = new AstParser2( source );
	_parser->Parse();
	_root = _parser->Result();

//    qDebug() << _root->DebugString();

//	_root = Parse( source );
	endResetModel();
}


QModelIndex CodeModel2::index( int row, int column, const QModelIndex& parent ) const
{
	if( !hasIndex( row, column, parent ) )
		return QModelIndex();

	AstItem* ancestor = parent.isValid()
			? static_cast< AstItem* >( parent.internalPointer() )
			: _root;

	AstItem* astItem = const_cast< AstItem* >( ancestor->Child( row ) );
	if( !astItem )
		return QModelIndex();
	return createIndex( row, column, astItem );
}

QModelIndex CodeModel2::parent( const QModelIndex& child ) const
{
	if( !child.isValid() )
		return QModelIndex();

	AstItem* astItem = static_cast< AstItem* >( child.internalPointer() );
	if( !astItem )
		return QModelIndex();

	AstItem* parent = astItem->Parent();
	if( !parent || parent == _root )
		return QModelIndex();

	return createIndex( parent->SiblingPos(), 0, parent );
}

int CodeModel2::rowCount( const QModelIndex& parent ) const
{
	if( parent.isValid() ) {
		AstItem* astItem = static_cast< AstItem* >( parent.internalPointer() );
		return astItem->ChildrenCount();
	}
	else {
		return _root->ChildrenCount();
	}
}

int CodeModel2::columnCount( const QModelIndex& /*parent*/ ) const
{
	return 1;
}

//bool CodeModel::hasChildren( const QModelIndex& parent ) const
//{
//	if( parent.isValid() ) {
//		AstItem* AstItem = static_cast< AstItem* >( parent.internalPointer() );
//		return AstItem->ChildrenCount() > 0;
//	}
//	return _root->ChildrenCount() > 0;
//}

QVariant CodeModel2::data( const QModelIndex& index, int role ) const
{
	AstItem* astItem = static_cast< AstItem* >( index.internalPointer() );

	QVariant result;
	switch ( role ) {
	case Qt::DisplayRole :
		if( astItem )
			result = astItem->TypeText();
		break;
	default:
		break;
	}

	return result;
}

Qt::ItemFlags CodeModel2::flags( const QModelIndex& index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
