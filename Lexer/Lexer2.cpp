#include "Lexer2.h"

#include <QMap>

Lexer2::Lexer2()
{

}

Lexer2::Lexer2( const QString* source )
{
	_state.Begin = _state.Current = _state.Previos = source->data();
	_state.End = _state.Begin + source->size();
    _state.LineNumber = 1;

    // SkipSheBang();
    if( _state.Current->unicode() == L'#' ) {
        ++_state.Current;
        if( _state.Current->unicode() == L'!' ) {
            ++_state.Current;
            while( HasNext() ) {
                if( CurrIsNewline() ) {
                    SkipNewLine();
                    _state.Type = Next();
                    return;
                }
                ++_state.Current;
            }
            _state.Previos = _state.Current = _state.End;
            _state.Type = TT_END_OF_FILE;
        }
        else {
            _state.Previos = _state.Current = _state.End;
            _state.Type = TT_ERROR;
        }
    }
    else {
        _state.Type = Next();
    }
}

Lexer2::Lexer2( const LexerState& state ) :
	_state( state )
{
}

bool Lexer2::HasNext() const
{
	return _state.Current < _state.End;
}

TokenType Lexer2::Next()
{
	while( HasNext() ) {
		_state.Previos = _state.Current;
		switch ( _state.Current->unicode() ) {
		case L'\n': case L'\r': {  /* line breaks */
			SkipNewLine();
			break;
		}
		case L' ': case L'\f': case L'\t': case L'\v': {  /* spaces */
			++_state.Current;
			break;
		}
		case L'-': {  /* '-' or '--' (comment) */
			++_state.Current;
			if( _state.Current->unicode() != L'-' ) {
				return _state.Type = TT_MINUS;;
			}
			/* else is a comment */
			++_state.Current;
			if( !HasNext() ) {
				return _state.Type = TT_END_OF_FILE;
			}

			if( _state.Current->unicode() == L'[' ) {  /* long comment? */
				int count = SkipMultiLineSeparator();
				if( count >= 0 ) {
					SkipMultiLineContent( count );  /* skip long comment */
					break;
				}
			}
			/* else short comment */
			while( !CurrIsNewline() ) {
				++_state.Current;  /* skip until end of line (or end of file) */
				if( !HasNext() )
					break;
			}
            SkipNewLine();
			break;
		}
		case L'[': {  /* long string or simply '[' */
			int count = SkipMultiLineSeparator();
			if( count >= 0 ) {
				if( SkipMultiLineContent( count ) ) {
					return _state.Type = TT_STRING;
				}
				return _state.Type = TT_ERROR;
			}
			else if( count == -1 ) {
				return  _state.Type = TT_LEFT_SQUARE;
			}
			else {
				return _state.Type = TT_ERROR;
			}
		}
		case L'=': {
			++_state.Current;
			if( _state.Current->unicode() != L'=' ) {
				return _state.Type = TT_ASSIGN;
			}
			else {
				++_state.Current;
				return _state.Type = TT_EQUAL;
			}
		}
		case L'<': {
			++_state.Current;
			if( _state.Current->unicode() != L'=' ) {
				return _state.Type = TT_LESS;
			}
			else {
				++_state.Current;
				return _state.Type = TT_LESS_OR_EQUAL;
			}
		}
		case L'>': {
			++_state.Current;
			if( _state.Current->unicode() != L'=' )
				return _state.Type = TT_GREAT;
			else {
				++_state.Current;
				return _state.Type = TT_GREAT_OR_EQUAL;
			}
		}
		case L'~': {
			++_state.Current;
			if( _state.Current->unicode() != L'=' )
				return _state.Type = TT_TILDA;
			else {
				++_state.Current;
				return _state.Type = TT_NOT_EQUAL;
			}
		}
		case L':': {
			++_state.Current;
			if ( _state.Current->unicode() != L':')
				return _state.Type = TT_COLON;
			else {
				++_state.Current;
				return _state.Type = TT_DOUBLE_COLON;
			}
		}
		case L'"': case L'\'': {  /* short literal strings */
			if( ReadString() )
				return _state.Type = TT_STRING;
			else
				return _state.Type = TT_ERROR;
		}
		case L'.': {  /* '.', '..', '...', or number */
			++_state.Current;
			if( _state.Current->unicode() == L'.' ) {
				++_state.Current;
				if( _state.Current->unicode() == L'.' ) {
					++_state.Current;
					return _state.Type = TT_DOTS;   /* '...' */
				}
				else
					return _state.Type = TT_CONCAT;   /* '..' */
			}
			else if( !_state.Current->isDigit() )
				return _state.Type = TT_POINT;

			/* else go through */
		}
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': {
			SkipNumber();
			return _state.Type = TT_NUMBER;
		}
		default: {
			if( CurrentIsAlpha() ) {  /* identifier or reserved word? */
				do {
					++_state.Current;
					if( !HasNext() )
						break;
				} while( CurrentIsAlphaOrNumber() );

				TokenType keyword = CurrentKeyword();
				if( keyword > 0 ) { /* reserved word? */
					return _state.Type = keyword;
				}
				else {
					return _state.Type = TT_NAME;
				}
			}
			else {  /* single-char tokens */
				int c = _state.Current->unicode();
				++_state.Current;
				switch( c ) {
				case L'+' : return _state.Type = TT_PLUS;
				case L'/' : return _state.Type = TT_SLASH;
				case L'*' : return _state.Type = TT_MAGNIFY;
				case L'^' : return _state.Type = TT_CARET;
				case L'%' : return _state.Type = TT_PERCENT;
				case L']' : return _state.Type = TT_RIGHT_SQUARE;
				case L'(' : return _state.Type = TT_LEFT_BRACKET;
				case L')' : return _state.Type = TT_RIGHT_BRACKET;
				case L'{' : return _state.Type = TT_LEFT_CURLY;
				case L'}' : return _state.Type = TT_RIGHT_CURLY;
				case L';' : return _state.Type = TT_SEMICOLON;
				case L',' : return _state.Type = TT_COMMA;
				case L'#' : return _state.Type = TT_NUMBER_SIGN;
				default:
					return _state.Type = TT_ERROR;
				}
			}
		}
		}
	}

	_state.Previos = _state.Current;
    return _state.Type = TT_END_OF_FILE;
}

bool Lexer2::NextIf( TokenType type )
{
    if( _state.Type == type ) {
        Next();
        return true;
    }
    return false;
}

const QString Lexer2::CurrentLineText() const
{
    const QChar* previous = _state.Previos;
    while( previous > _state.Begin ) {
        --previous;
        if( previous->unicode() == '\n' )
            break;
    }

    const QChar* end = _state.Current;
    while( end != _state.End ) {
        ++end;
        if( end->unicode() == '\n' )
            break;
    }

    return QString::fromRawData( previous, end - previous + 1 );
}

const QString Lexer2::CurrentString() const
{
	return QString::fromRawData( _state.Previos, _state.Current - _state.Previos );
}

TokenType Lexer2::CurrentType() const
{
	return _state.Type;
}

int Lexer2::CurrentLine() const
{
	return _state.LineNumber;
}

int Lexer2::TokenBeginPos() const
{
    return _state.Previos - _state.Begin;
}

int Lexer2::TokenEndPos() const
{
    return _state.Current - _state.Begin;
}

int Lexer2::TokenSize() const
{
    return _state.Current - _state.Previos;
}

bool Lexer2::Is( TokenType type ) const
{
    return _state.Type == type;
}

/*
** skip a sequence '[=*[' or ']=*]' and return its number of '='s or
** -1 if sequence is malformed
*/
int Lexer2::SkipMultiLineSeparator() {
	int count = 0;
	const ushort bracket = _state.Current->unicode();
	++_state.Current;
	while( _state.Current->unicode() == L'=' ) {
		++_state.Current;
		++count;
	}
	return ( _state.Current->unicode() == bracket ) ? count : ( -count ) - 1;
}

bool Lexer2::SkipMultiLineContent( int count ) {
    ++_state.Current;       /* skip 2nd `[' */
	if( CurrIsNewline() )   /* string starts with a newline? */
		SkipNewLine();      /* skip it */

	while( HasNext() ) {
		switch( _state.Current->unicode() ) {
		case L']': {
			if( SkipMultiLineSeparator() == count ) {
				++_state.Current;  /* skip 2nd `]' */
				return true;
			}
			break;
		}
		case L'\n': case L'\r': {
			SkipNewLine();
			break;
		}
		default:
			++_state.Current;
		}
	}

	return false;
}

bool Lexer2::CurrIsNewline() const
{
	const ushort v = _state.Current->unicode();
	return v == L'\r' || v == L'\n';
}

void Lexer2::SkipNewLine()
{
	++_state.LineNumber;
	++_state.Current;
}

bool Lexer2::ReadString()
{
	const ushort quote = _state.Current->unicode();
	++_state.Current;
	while( HasNext() ) {
		if( _state.Current->unicode() == quote ) {
			++_state.Current;
			return true;
		}

		switch( _state.Current->unicode() ) {
		case L'\n':
		case L'\r':
			//            lexerror(ls, "unfinished string", TT_STRING);
			return false;
		case L'\\': {  /* escape sequences */
			++_state.Current;
			if( !HasNext() )
				return false;

			switch( _state.Current->unicode() ) {
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
				++_state.LineNumber;
				break;
			case L'z': { /* zap following span of spaces */
				++_state.Current; /* skip the 'z' */
				while( HasNext() && _state.Current->isSpace() ) {
					if( CurrIsNewline() )
						SkipNewLine();
					else
						++_state.Current;
				}
				break; /* escape sequences */
			}
			default: {
				if( !_state.Current->unicode() ) {
					// escerror(ls, &ls->current, 1, "invalid escape sequence");
					return false;
				}
				/* digital escape \ddd */
                if( !SkipDecimalEscapeSequence() ) {
					return false;
                }
                continue;
			}
			}// /* escape sequences */
		}
		default:
			++_state.Current;
		}
	}
	return false;
}

bool isHexDigit( const QChar* c ) {
	if( c->isDigit() )
		return true;

	if( c->unicode() >= L'a' && c->unicode() <= L'f' )
		return true;
	if( c->unicode() >= L'A' && c->unicode() <= L'F' )
		return true;

	return false;
}

bool Lexer2::SkipDecimalEscapeSequence()
{
	int r = 0;
	for( int i = 0; i < 3 && _state.Current->isDigit(); ++i ) {  /* read up to 3 digits */
		r = 10 * r + _state.Current->unicode() - L'0';
		++_state.Current;
		if( !HasNext() )
			break;
	}

	if( r > UCHAR_MAX ) {
//        escerror(ls, c, i, "decimal escape too large");
		return false;
	}

	return true;
}

bool Lexer2::SkipHexEscapeSequence()
{
	for( int i = 1; i < 3; ++i ) {  /* read two hexa digits */
		++_state.Current;
		if( !HasNext() || !isHexDigit( _state.Current ) ) {
			// escerror(ls, c, i + 1, "hexadecimal digit expected");
			return false;
		}
	}
	return true;
}

void Lexer2::SkipNumber()
{
	/* LUA_NUMBER */
    if( _state.Current->unicode() == L'0' ) {
        ++_state.Current;
        if( !HasNext() )
            return;
        if( _state.Current->unicode() == L'x' ) {
            do {
                ++_state.Current;
                if( !HasNext() )
                    return;
            } while( _state.Current->isLetter() || _state.Current->isDigit() );
        }
    }
    else if( _state.Current->isDigit() ) {
        ++_state.Current;
        if( !HasNext() )
            return;
    }

    while( _state.Current->isNumber() || _state.Current->unicode() == L'.' ) {
		++_state.Current;
		if( !HasNext() )
			break;

		const ushort symbol = _state.Current->unicode();
		if( symbol == L'E' || symbol == L'e' ) { /* exponent part? */
			++_state.Current;
			if( !HasNext() ) {
				break;
			}
			if( symbol == L'+' || symbol == L'-' ) {  /* optional exponent sign */
				++_state.Current;
				if( !HasNext() )
					break;
			}
		}
    }
}

bool Lexer2::CurrentIsAlpha() const
{
	return _state.Current->isLetter() || _state.Current->unicode() == L'_';
}

bool Lexer2::CurrentIsAlphaOrNumber() const
{
	return CurrentIsAlpha() || _state.Current->isDigit();
}

TokenType Lexer2::CurrentKeyword() const
{
	static QMap< QString, TokenType > _keywords;
	if( _keywords.isEmpty() ) {
		_keywords.insert( "nil",		TT_NIL );
		_keywords.insert( "true",		TT_TRUE );
		_keywords.insert( "false",		TT_FALSE );

		_keywords.insert( "not",		TT_NOT );
		_keywords.insert( "and",		TT_AND );
		_keywords.insert( "or",			TT_OR );

		_keywords.insert( "local",		TT_LOCAL );
		_keywords.insert( "function",	TT_FUNCTION );

		_keywords.insert( "do",			TT_DO );
		_keywords.insert( "while",		TT_WHILE );
		_keywords.insert( "repeat",		TT_REPEAT );
		_keywords.insert( "until",		TT_UNTIL );
		_keywords.insert( "if",			TT_IF );
		_keywords.insert( "then",		TT_THEN );
		_keywords.insert( "elseif",		TT_ELSEIF );
		_keywords.insert( "else",		TT_ELSE );
		_keywords.insert( "for",		TT_FOR );
		_keywords.insert( "in",			TT_IN );

		_keywords.insert( "return",		TT_RETURN );
		_keywords.insert( "break",		TT_BREAK );
		_keywords.insert( "end",		TT_END );
	}

	return _keywords.value( CurrentString(), TT_ERROR );
}
