#ifndef OBJECT_MODEL_H
#define OBJECT_MODEL_H

#include <QAbstractItemModel>

class VariableInfo;

class ObjectModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit ObjectModel( QObject* parent = 0 );
	~ObjectModel();

	void SetNewRoot( VariableInfo* newRoot );

signals:

public slots:

	// QAbstractItemModel interface
public:
	virtual QModelIndex index   ( int row, int column, const QModelIndex& parent ) const;
	virtual QModelIndex parent  ( const QModelIndex& child ) const;
	virtual int rowCount        ( const QModelIndex& parent ) const;
	virtual int columnCount     ( const QModelIndex& parent ) const;
	virtual bool hasChildren    ( const QModelIndex& parent ) const;
	virtual QVariant data       ( const QModelIndex& index, int role ) const;
	virtual Qt::ItemFlags flags ( const QModelIndex& index) const;

private:
	VariableInfo* _root;
};

#endif // OBJECT_MODEL_H
