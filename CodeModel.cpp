#include "CodeModel.h"

#include <QDebug>
#include <QRegExp>
#include <QtConcurrentRun>

//#include "Tokenizer.h"
#include "ASTParser.h"

CodeModel::CodeModel( QObject* parent ) :
	QAbstractItemModel( parent )
{
	_root = new Context( "Global" );
}

CodeModel::~CodeModel()
{
	delete _root;
}

void CodeModel::RebuildModel( const QString& source )
{
	beginResetModel();
	delete _root;
//	_root = new Context( "Global" );
    Lexer lexer( source );

    ASTParser ast( lexer );
    ast.Parse();
    _root = ast.Result();

//    qDebug() << _root->DebugString();

//	_root = Parse( source );
	endResetModel();
}


QModelIndex CodeModel::index( int row, int column, const QModelIndex& parent ) const
{
	if( !hasIndex( row, column, parent ) )
		return QModelIndex();

	Context* ancestor = parent.isValid()
            ? static_cast< Context* >( parent.internalPointer() )
			: _root;

	Context* context = ancestor->Child( row );
	if( !context )
		return QModelIndex();
	return createIndex( row, column, context );
}

QModelIndex CodeModel::parent( const QModelIndex& child ) const
{
	if( !child.isValid() )
		return QModelIndex();

    Context* context = static_cast< Context* >( child.internalPointer() );
	if( !context )
		return QModelIndex();

	Context* parent = context->Parent;
	if( !parent || parent == _root )
		return QModelIndex();

	return createIndex( parent->Pos(), 0, parent );
}

int CodeModel::rowCount( const QModelIndex& parent ) const
{
	if( parent.isValid() ) {
        Context* context = static_cast< Context* >( parent.internalPointer() );
        return context->ChildrenCount();
	}
	else {
        return _root->ChildrenCount();
	}
}

int CodeModel::columnCount( const QModelIndex& /*parent*/ ) const
{
	return 1;
}

//bool CodeModel::hasChildren( const QModelIndex& parent ) const
//{
//	if( parent.isValid() ) {
//		Context* context = static_cast< Context* >( parent.internalPointer() );
//		return context->ChildrenCount() > 0;
//	}
//	return _root->ChildrenCount() > 0;
//}

QVariant CodeModel::data( const QModelIndex& index, int role ) const
{
    Context* context = static_cast< Context* >( index.internalPointer() );

	QVariant result;
	switch ( role ) {
	case Qt::DisplayRole :
		if( context )
			result = context->Name;
		break;
	default:
		break;
	}

    return result;
}

Qt::ItemFlags CodeModel::flags( const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
