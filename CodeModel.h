#ifndef CODE_MODEL_H
#define CODE_MODEL_H

#include <QAbstractItemModel>

struct Context {
	Context( const QString& name, Context* parent = 0 ) :
		Name( name ),
		Parent( parent )
	{
		if( Parent )
			Parent->Children.append( this );
	}

	~Context() {
		qDeleteAll( Children );
	}

	QString Name;

	Context* Parent;
	QList< Context* > Children;

	int Pos() const {
		if( Parent ) {
			int pos = Parent->Children.indexOf( const_cast< Context* >( this ) );
			return pos == -1 ? 0 : pos;
		}

		return 0;
	}

    void Append( const QString& text ) {
        Context* c = new Context( text );
        Append( c );
    }

    void Append( Context* c ) {
        c->Parent = this;
        Children.append( c );
    }

	Context* Child( int i ) {
		return Children.value( i, 0 );
	}

	int ChildrenCount() const {
		return Children.count();
	}

    bool HasChild() const {
        return Children.count();
    }

	void Clear() {
		qDeleteAll( Children );
		Children.clear();
	}

    QString DebugString( int n = 0 ) const {
        QString result;

        result.append( Name );
        for( int i = 0; i < ChildrenCount(); ++i ) {
            result.append( '\n' );
            result.append( QString( ( n + 1 ) * 4, ' ' ) );
            result.append( Children[ i ]->DebugString( n + 1 ) );
        }
        return result;
    }
};

class CodeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit CodeModel( QObject* parent = 0 );
	~CodeModel();

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
	Context* _root;
};

#endif // CODE_MODEL_H
