#include "Parser.h"

#include <QDebug>
#include <QSet>

QSet< ushort > _operators = QSet< ushort >() << L'=' << L'#' << L':'
                                             << L'[' << L']' << L'(' << L')' << L'{' << L'}'
                                             << L'+' << L'-' << L'*' << L'/' << L'^' << L'%'
                                             << L',' << L';';

QSet< QString > _keywords = QSet< QString >() << "and" << "break" << "do" << "else" << "elseif"
                                              << "end" << "false" << "in" << "not" << "function"
                                              << "for" << "local" << "if" << "then" << "repeat"
                                              << "nil" << "until" << "or" << "true" << "return" << "while";


Parser::Parser( QString text ) :
	_text( text ),
	p( _text.data() )
{
}

void Parser::SkipToEndOfLine() {
	while( p->unicode() != L'\n' ) {
		if( p->isNull() )
			return;
		++p;
	}

	// past end of line
	++p;
}

void Parser::EndOfString( const ushort& quote ) {
	++p;
	while( p->unicode() != L'\n' && p->unicode() != quote ) {
		if( p->isNull() )
			return;
		++p;
	}

	// past end of line
	++p;
}

bool Parser::NotEscaped( QChar* current ) {
	QChar* begin = _text.data();
	int isEscaped = 0;
	while( current > begin ) {
		--current;
		if( current->unicode() != L'\\' )
			break;

		isEscaped ^= 1;
	}
	return isEscaped;
}

bool Parser::TryParseSpace() {
	QChar* a = p;
	while( a->isSpace() ) {
		++a;
	}

	if( a > p ) {
		p = a;
		return true;
	}

	return false;
}

bool Parser::TryParseComment() {
	QChar* a = p;
	if( a->unicode() != L'-' )
		return false;

	++a;
	if( a->unicode() != L'-' )
		return false;

	p += 2;

	if( !TryParseMultiLineComment() )
		SkipToEndOfLine();

	return true;
}

bool Parser::TryParseMultiLineComment() {
	// Multiline
	QChar* a = p;
	if( a->unicode() != L'[' )
		return false;

	int level = 0;
	++a;
	while( a->unicode() == L'=' ) {
		if( a->isNull() )
			return false;
		++a;
		++level;
	}

	if( a->unicode() != L'[' )
		return false;

	// Find end of multiline
	while( true ) {
		++a;
		while( a->unicode() != L']' ) {
			if( a->isNull() ) {
				// Multiline till end of text
				p = a;
				return true;
			}
			++a;
		}

		// Check level
		int checkLevel = level;
		++a;
		while( checkLevel && a->unicode() == L'=' ) {
			++a;
			--checkLevel;
		}

		// Check end
		if( !checkLevel && a->unicode() == L']' ) {
			++a;
			break;
		}
	}

	p = a;
	return true;
}

bool Parser::TryParseMultiLineString()
{
	// Multiline
	QChar* a = p;
	if( a->unicode() != L'[' )
		return false;

	int level = 0;
	++a;
	while( a->unicode() == L'=' ) {
		if( a->isNull() )
			return false;
		++a;
		++level;
	}

	if( a->unicode() != L'[' )
		return false;

	// Find end of multiline
	while( true ) {
		++a;
		while( a->unicode() != L']' && NotEscaped( a ) ) {
			if( a->isNull() ) {
				// Multiline till end of text
				p = a;
				return true;
			}
			++a;
		}

		// Check level
		int checkLevel = level;
		++a;
		while( checkLevel && a->unicode() == L'=' ) {
			++a;
			--checkLevel;
		}

		// Check end
		if( !checkLevel && a->unicode() == L']' ) {
			++a;
			break;
		}
	}

	p = a;
	return true;
}

bool Parser::TryParseLexems()
{
	QChar* a = p;

	while( a->isLetterOrNumber() || a->unicode() == L'_' ) {
		if( a->isNull() )
			break;
		++a;
	}

	if( a > p ) {
		p = a;
		return true;
	}

	return false;
}

bool Parser::TryParseOperator()
{
	if( _operators.contains( p->unicode() ) ) {
		++p;
		return true;
	}

    switch( p->unicode() ) {
    case( L'.' ) : {
        ++p;
        if( p->unicode() == L'.' ) {
            ++p;
            if( p->unicode() == L'.' )
                ++p;
        }
        return true;
    }
    case( L'<' ) :
    case( L'>' ) :
    case( L'=' ) :
    case( L'~' ) : {
        ++p;
        if( p->unicode() == L'=' )
            ++p;
        return true;
    }
    }

	return false;
}

bool Parser::TryParseString()
{
	if( p->unicode() == L'"' || p->unicode() == L'\'' ) {
		EndOfString( p->unicode() );
		return true;
	}

	return TryParseMultiLineString();
}

void Parser::Parse() {
	while( !p->isNull() ) {

		QChar* begin = p;

		TokenType type;

		if( TryParseSpace() ) {
            // type = Spacing;
            continue;
		}
		else if( TryParseComment() ) {
            type = Comment;
		}
		else if( TryParseString() ) {
			type = String;
		}
		else if( TryParseLexems() ) {
			type = Lexems;
		}
		else if( TryParseOperator() ) {
			type = Operator;
		}
		else {
			qWarning() << "Wrong symbol" << QString( *p );
			return;
		}

		Token token = { begin, p, type };
		_tokens.append( token );

//      qDebug() << token.DebugString().toLocal8Bit().data();
	}

//  qDebug() << _tokens.size();
}

QList<Parser::Token> Parser::Tokens() const {
	return _tokens;
}

const QString Parser::Token::ToString() const
{
    return QString::fromRawData( this->Begin, this->End - this->Begin );
}

bool Parser::Token::operator ==( const QString& string ) const {
    const QChar* lhs = string.data();
    const QChar* rhs = this->Begin;
    for( int i = 0; i < string.size(); ++i ) {
        if( lhs->unicode() != rhs->unicode() ) {
            return false;
        }

        ++lhs;
        ++rhs;
    }

    return true;
}

bool Parser::Token::Is( const QChar& c ) const
{
    return this->Begin->unicode() == c.unicode();
}

QString Parser::Token::DebugString() const {
	QString result;

    switch( this->Type ) {
        case Unknown : {
            result = "Unknown : ";
            break;
        }
        case Spacing : {
            return "Spacing";
        }
        case Comment : {
            result = "Comment : ";
            break;
        }
        case Lexems : {
            result = "Lexema  : ";
            break;
        }
        case String : {
            result = "String  : ";
            break;
        }
        case Operator : {
            result = "Operator: ";
            break;
        }
	}

    return result.append( this->ToString() );
}
