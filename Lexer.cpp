#include "Lexer.h"

#include <QMap>

Lexer::Lexer( const QString& text ) :
	_text   ( text ),

	_next( _text.data() ),
	_nextTokenBegin( nullptr ),
	_nextTokenType( TK_ERROR ),

	_begin  ( _text.data() ),
	_end    ( _text.data() + text.size() ),

	_lineNumber( 0 )
{
	Next();
}

bool Lexer::HasNext() const
{
	return _next < _end;
}

int Lexer::Next()
{
	_currentToken.Begin = _nextTokenBegin;
	_currentToken.End = _next;
	_currentToken.Type = _nextTokenType;
	_currentToken.LineNumber = _lineNumber;

	_nextTokenType = AdvanceToken();

	return _currentToken.Type;
}

int Lexer::AdvanceToken()
{
	while( HasNext() ) {
		_nextTokenBegin = _next;
		switch ( _next->unicode() ) {
		case L'\n': case L'\r': {  /* line breaks */
			SkipNewLine();
			break;
		}
		case L' ': case L'\f': case L'\t': case L'\v': {  /* spaces */
			++_next;
			break;
		}
		case L'-': {  /* '-' or '--' (comment) */
			++_next;
			if( !HasNext() || _next->unicode() != L'-' )
				return L'-';
			/* else is a comment */
			++_next;
			if( !HasNext() )
				return TK_END_OF_FILE;

			if( _next->unicode() == L'[' ) {  /* long comment? */
				int count = SkipMultiLineSeparator();
				if( count >= 0 ) {
					SkipMultiLineContent( count );  /* skip long comment */
					break;
				}
			}
			/* else short comment */
			while( !CurrIsNewline() ) {
				++_next;  /* skip until end of line (or end of file) */
				if( !HasNext() )
					break;
			}
			++_next;
			break;
		}
		case L'[': {  /* long string or simply '[' */
			int count = SkipMultiLineSeparator();
			if( count >= 0 ) {
				if( SkipMultiLineContent( count ) ) {
					return TK_STRING;
				}
				return TK_ERROR;
			}
			else if( count == -1 ) {
				return L'[';
			}
			else {
				return TK_ERROR;
			}
		}
		case L'=': {
			++_next;
			if( _next->unicode() != L'=' ) {
				return L'=';
			}
			else {
				++_next;
				return TK_EQUAL;
			}
		}
		case L'<': {
			++_next;
			if( _next->unicode() != L'=' ) {
				return L'<';
			}
			else {
				++_next;
				return TK_LESS_OR_EQUAL;
			}
		}
		case L'>': {
			++_next;
			if( _next->unicode() != L'=' )
				return L'>';
			else {
				++_next;
				return TK_GREAT_OR_EQUAL;
			}
		}
		case L'~': {
			++_next;
			if( _next->unicode() != L'=' )
				return L'~';
			else {
				++_next;
				return TK_NOT_EQUAL;
			}
		}
		case L':': {
			++_next;
			if ( _next->unicode() != L':')
				return L':';
			else {
				++_next;
				return TK_DOUBLE_COLON;
			}
		}
		case L'"': case L'\'': {  /* short literal strings */
			if( ReadString() )
				return TK_STRING;
			else
				return TK_ERROR;
		}
		case L'.': {  /* '.', '..', '...', or number */
			++_next;
			if( !HasNext() )
				return L'.';

			if( _next->unicode() == L'.' ) {
				++_next;
				if( HasNext() && _next->unicode() == L'.' ) {
					++_next;
					return TK_DOTS;   /* '...' */
				}
				else
					return TK_CONCAT;   /* '..' */
			}
			else if( !_next->isDigit() )
				return L'.';

			/* else go through */
		}
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': {
			SkipNumber();
			return TK_NUMBER;
		}
		default: {
			if( CurrentIsAlpha() ) {  /* identifier or reserved word? */
				do {
					++_next;
					if( !HasNext() )
						break;
				} while( CurrentIsAlphaOrNumber() );

				int keyword = CurrentKeyword();
				if( keyword > 0 )  /* reserved word? */
					return keyword;
				else {
					return TK_NAME;
				}
			}
			else {  /* single-char tokens ( + - / * = ^ % ) */
				int c = _next->unicode();
				++_next;
				return c;
			}
		}
		}
	}

	return TK_END_OF_FILE;
}

const QString Lexer::CurrentString() const
{
	return QString::fromRawData( _currentToken.Begin, _currentToken.End - _currentToken.Begin ) + "";
}

int Lexer::CurrentTokenType() const
{
	return _currentToken.Type;
}

int Lexer::CurrentLine() const
{
	return _currentToken.LineNumber;
}

int Lexer::CurrentPos() const
{
	return _currentToken.Begin - _begin;
}

int Lexer::PeekNextTokenType() const
{
	return _nextTokenType;
}

/*
** skip a sequence '[=*[' or ']=*]' and return its number of '='s or
** -1 if sequence is malformed
*/
int Lexer::SkipMultiLineSeparator() {
	int count = 0;
	const ushort bracket = _next->unicode();
	++_next;
	while( _next->unicode() == L'=' ) {
		++_next;
		++count;
	}
	return ( _next->unicode() == bracket ) ? count : ( -count ) - 1;
}

bool Lexer::SkipMultiLineContent( int count ) {
	++_next;             /* skip 2nd `[' */
	if( CurrIsNewline() )   /* string starts with a newline? */
		SkipNewLine();      /* skip it */

	while( HasNext() ) {
		switch( _next->unicode() ) {
		case L']': {
			if( SkipMultiLineSeparator() == count ) {
				++_next;  /* skip 2nd `]' */
				return true;
			}
			break;
		}
		case L'\n': case L'\r': {
			SkipNewLine();
			break;
		}
		}

		++_next;
	}

	return false;
}

bool Lexer::CurrIsNewline() const
{
	const ushort v = _next->unicode();
	return v == L'\r' || v == L'\n';
}

void Lexer::SkipNewLine()
{
	++_lineNumber;
	++_next;
}

bool Lexer::ReadString()
{
	const ushort quote = _next->unicode();
	++_next;
	while( HasNext() ) {
		if( _next->unicode() == quote ) {
			++_next;
			return true;
		}

		switch( _next->unicode() ) {
		case L'\n':
		case L'\r':
			//            lexerror(ls, "unfinished string", TK_STRING);
			return false;
		case L'\\': {  /* escape sequences */
			++_next;
			if( !HasNext() )
				return false;

			switch( _next->unicode() ) {
			case L'a': case L'b':
			case L'f': case L'n':
			case L'r': case L't':
			case L'v':
			case L'\\': case L'\"': case L'\'':
				break; /* escape sequences */
			case L'x':
				if( !SkipHexEscapeSequence() )
					return false;
				break;
			case L'\n': case L'\r':
				SkipNewLine(); break; /* escape sequences */
			case L'z': { /* zap following span of spaces */
				++_next; /* skip the 'z' */
				while( HasNext() && _next->isSpace() ) {
					if( CurrIsNewline() )
						SkipNewLine();
					else
						++_next;
				}
				break; /* escape sequences */
			}
			default: {
				if( !_next->unicode() ) {
					// escerror(ls, &ls->current, 1, "invalid escape sequence");
					return false;
				}
				/* digital escape \ddd */
				if( !SkipDecimalEscapeSequence() )
					return false;
				break; /* escape sequences */
			}
			}// /* escape sequences */
		}
		default:
			++_next;
		}
	}
	return false;
}

bool isHex( const QChar* c ) {
	if( c->isDigit() )
		return true;

	if( c->unicode() >= L'a' && c->unicode() <= L'f' )
		return true;
	if( c->unicode() >= L'A' && c->unicode() <= L'F' )
		return true;

	return false;
}

bool Lexer::SkipDecimalEscapeSequence()
{
	int r = 0;
	for( int i = 0; i < 3 && _next->isDigit(); ++i ) {  /* read up to 3 digits */
		r = 10 * r + _next->unicode() - L'0';
		++_next;
		if( !HasNext() )
			break;
	}

	if( r > UCHAR_MAX ) {
//        escerror(ls, c, i, "decimal escape too large");
		return false;
	}

	return true;
}

bool Lexer::SkipHexEscapeSequence()
{
	for( int i = 1; i < 3; ++i ) {  /* read two hexa digits */
		++_next;
		if( !HasNext() || !isHex( _next ) ) {
			// escerror(ls, c, i + 1, "hexadecimal digit expected");
			return false;
		}
	}
	return true;
}

void Lexer::SkipNumber()
{
	/* LUA_NUMBER */
	do {
		++_next;
		if( !HasNext() )
			break;

		const ushort symbol = _next->unicode();
		if( symbol == L'E' || symbol == L'e' ) { /* exponent part? */
			++_next;
			if( !HasNext() ) {
				break;
			}
			if( symbol == L'+' || symbol == L'-' ) {  /* optional exponent sign */
				++_next;
				if( !HasNext() )
					break;
			}
		}
	} while( _next->isNumber() || _next->unicode() == L'.' );
}

bool Lexer::CurrentIsAlpha() const
{
	return _next->isLetter() || _next->unicode() == L'_';
}

bool Lexer::CurrentIsAlphaOrNumber() const
{
	return CurrentIsAlpha() || _next->isDigit();
}

int Lexer::CurrentKeyword() const
{
	static QMap< QString, TokenType > _keywords;
	if( _keywords.isEmpty() ) {
		_keywords.insert( "return", TK_RETURN );
		_keywords.insert( "nil", TK_NIL );
	}

	return _keywords.value( LexerString(), TK_ERROR );
}

const QString Lexer::LexerString() const
{
	return QString::fromRawData( _nextTokenBegin, _next - _nextTokenBegin );
}
