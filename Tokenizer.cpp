#include "Tokenizer.h"

Tokenizer::Tokenizer( const QString& source ) :
	_source( source ),
	_pos( 0 ),
	_tokenRX( "(\\w+)|(\\S)" )
{
}

int Tokenizer::CurrentPos() const
{
	return _pos;
}

bool Tokenizer::HasNext() const
{
	return _pos != -1;
}

Token Tokenizer::Next()
{
	if( _pos == -1 )
		return Token();

	_pos = _tokenRX.indexIn( _source, _pos );
	if( _pos == -1 )
		return Token();

	Token result( _source.mid( _pos, _tokenRX.matchedLength() ), _pos );
	_pos += _tokenRX.matchedLength();
	return result;
}

FindResult Tokenizer::Find( const QString& what, bool onlyOneLine )
{
	FindResult result;

	if( onlyOneLine ) {
		QRegExp findRX( "(\\n)|" + what );
		int pos = findRX.indexIn( _source, _pos );
		if( pos != -1 && _source[ pos ] == '\n' ) {
			return result;
		}
		result.Pos = pos;
		result.MatchedLenght = findRX.matchedLength();
		return pos;
	}

	QRegExp findRX( what );
	int pos = findRX.indexIn( _source, _pos );
	return FindResult( pos, findRX.matchedLength() );
}

bool Tokenizer::IsCharEscaped( int pos ) const
{
	int count = 0;
	for( int i = pos - 1; i > _pos && _source.at( i ) == '\\'; --i )
		++count;

	return ( count & 1 ) == 0;
}

QChar Tokenizer::PeekChar() const
{
	if( _pos != -1 && _pos < _source.size() )
		return _source.at( _pos + 1 );

	return QChar();
}

Token Tokenizer::PeekToken() const
{
	if( _pos == -1 )
		return Token();

	int pos = _tokenRX.indexIn( _source, _pos );
	if( pos == -1 )
		return Token();
	return Token ( _source.mid( pos, _tokenRX.matchedLength() ), pos );
}

void Tokenizer::SkipTo( int pos )
{
	_pos = pos;
}

void Tokenizer::Skip( const QString& text, bool onlyOneLine )
{
	if( onlyOneLine ) {
		int pos = _source.indexOf( QRegExp( "(\\n)|" + text ) );
		if( pos == -1 )
			_pos = -1;
		else if( _source[ pos ] == '\n' )
			_pos = pos + 1;
		else
			_pos = pos + text.size();
	}
	else {
		int pos = _source.indexOf( text );
		if( pos == -1 )
			_pos = -1;
		else
			_pos = pos + text.size();
	}
}

void Tokenizer::SkipLine()
{
	_pos = _source.indexOf( '\n', _pos );
}
