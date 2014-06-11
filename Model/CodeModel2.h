#ifndef CODE_MODEL_2_H
#define CODE_MODEL_2_H

#include <QAbstractItemModel>

class AstItem;
class AstParser2;

class CodeModel2 : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit CodeModel2( QObject* parent = 0 );
	~CodeModel2();

	void RebuildModel( const QString& source );

signals:

public slots:

	// QAbstractItemModel interface
public:
	virtual QModelIndex index   ( int row, int column, const QModelIndex& parent ) const;
	virtual QModelIndex parent  ( const QModelIndex& child ) const;
	virtual int rowCount        ( const QModelIndex& parent ) const;
	virtual int columnCount     ( const QModelIndex& parent ) const;
//	virtual bool hasChildren    ( const QModelIndex& parent ) const;
	virtual QVariant data       ( const QModelIndex& index, int role ) const;
	virtual Qt::ItemFlags flags ( const QModelIndex& index) const;

private:
	AstItem* _root;
	AstParser2* _parser;
};

#endif // CODE_MODEL_H
