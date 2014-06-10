#include "tokenizer2.h"

Tokenizer2::Tokenizer2()
{
	source =
			"local a = \t700 function Minus( a, b ) return a - b end --Just comment\n"
			"local function b() return \"line\" .. 'line\\0' .. [=[aas\\]]]=]] end";
	source = source.repeated( 100000 );
	c = source.data();
	b = c;
	e = c + source.size();
}

bool Tokenizer2::HasNext() const
{
	return c != e;
}

QStringRef Tokenizer2::operator ()()
{
	switch( c->unicode() ) {
	case L'-' : {
		// Comment ?
		QChar* start = c;
		QChar* advance = start + 1;
		if( advance->unicode() != L'-' )
			break;

		c += 2;
		while( c != e && c->unicode() != L'\n' ) {
			++c;
		}

		return QStringRef( &source, start - b, c - start );
	}
	case L'[' : {
		// multiline string?
		QChar* start = c;
		int level = 0;
		QChar* advance = start + 1;
		for( ; advance != e; ++advance ) {
			if( advance->unicode() != L'=' )
				break;
			++level;
		}
		if( advance->unicode() != L'[' )
			break;

		++advance;
		while( advance != e ) {
			if( advance->unicode() != L']' ) {
				++advance;
				continue;
			}

			QChar* regress = advance - 1;
			while( regress->unicode() == L'\\' ) {
				--regress;
			}
			if( ( ( advance - regress ) & 1 ) == 0 ) {
				++advance;
				continue;
			}

			int checkLevel = level;
			while( ++advance, advance != e ) {
				if( advance->unicode() != L'=' )
					break;
				--checkLevel;
			}

			if( advance->unicode() == L']' ) {
				if( !checkLevel ) {
					++advance;
					break;
				}
			}
			else
				++advance;
		}

		c = advance;
		return QStringRef( &source, start - b, c - start );
	}
	case L'.' : {
		// . .. ... ?
		QChar* start = c;
		QChar* advance = start + 1;
		if( advance->unicode() != L'-' )
			break;

		c += 2;
		while( c != e && c->unicode() == L'.' ) {
			++c;
		}

		return QStringRef( &source, start - b, c - start );
	}
	default: {
		// Alpha
		if( c->isLetter() ) {
			QChar* start = c;
			for( ; c != e; ++c ) {
				if( !c->isLetterOrNumber() ) {
					break;
				}
			}
			return QStringRef( &source, start - b, c - start );
		}
		// Number
		else if( c->isDigit() ) {
			QChar* start = c;
			for( ; c != e; ++c ) {
				if( !c->isNumber() ) {
					break;
				}
			}
			return QStringRef( &source, start - b, c - start );
		}
		else if( c->isSpace() ) {
			QChar* start = c;
			for( ; c != e; ++c ) {
				if( !c->isSpace() ) {
					break;
				}
			}
			return QStringRef( &source, start - b, c - start );
		}
	}
	}

	QStringRef result( &source, c - b, 1 );
	++c;
	return result;
}
