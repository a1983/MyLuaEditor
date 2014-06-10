#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <QString>
#include <QRegExp>

struct Token {
	Token( const QString& text = QString(), int pos = -1 ) :
		Text( text ), Pos( pos ) {
	}

	bool IsValid() const {
		return Pos != -1;
	}

	bool operator==( const QString& rhs ) const {
		return Text == rhs;
	}

	QString Text;
	int Pos;
};

struct FindResult {
	FindResult( int pos = -1, int matchedLenght = -1 ) :
		Pos( pos ),
		MatchedLenght( matchedLenght ) {
	}

	int Pos;
	int MatchedLenght;
};

class Tokenizer
{
public:
	Tokenizer( const QString& source );

	int			CurrentPos() const;

	bool		HasNext() const;
	Token		Next();

	FindResult	Find( const QString& what, bool onlyOneLine = true );

	bool		IsCharEscaped( int pos ) const;

	QChar		PeekChar	() const;
	Token		PeekToken	() const;

	void		SkipTo( int pos );
	void		Skip( const QString& text, bool onlyOneLine = true );
	void		SkipLine();

private:
	QString _source;
	int		_pos;

	QRegExp _tokenRX;
};

#endif // TOKENIZER_H
