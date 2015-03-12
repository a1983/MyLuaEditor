#include "highlighter.h"

#include <QApplication>
#include <QTextDocument>

#include <QStringRef>

enum {
	MaxMultiLine = 255
};

enum BlockState {
	BlockState_Default = -1,
	BlockState_MultilineComment = 0,
	BlockState_MaxMultilineComment = BlockState_MultilineComment + MaxMultiLine,
	BlockState_MultilineText = BlockState_MaxMultilineComment + 1,
	BlockState_MaxMultilineText = BlockState_MultilineText + MaxMultiLine
};

Highlighter::Highlighter( QTextDocument* parent )
	: QSyntaxHighlighter( parent )
{
	_symbols = QRegExp( "(\\w+)|(\\S)" );

	_keywordFormat.setForeground( QBrush( "#93C763" ) );
	_keywords.insert( QStringRef( new QString( "local" ) ) );
	_keywords.insert( QStringRef( new QString( "function" ) ) );
	_keywords.insert( QStringRef( new QString( "end" ) ) );
	_keywords.insert( QStringRef( new QString( "return" ) ) );
	_keywords.insert( QStringRef( new QString( "if" ) ) );
	_keywords.insert( QStringRef( new QString( "then" ) ) );
	_keywords.insert( QStringRef( new QString( "else" ) ) );
	_keywords.insert( QStringRef( new QString( "elseif" ) ) );
	_keywords.insert( QStringRef( new QString( "for" ) ) );
	_keywords.insert( QStringRef( new QString( "in" ) ) );
	_keywords.insert( QStringRef( new QString( "do" ) ) );
	_keywords.insert( QStringRef( new QString( "break" ) ) );
	_keywords.insert( QStringRef( new QString( "and" ) ) );
	_keywords.insert( QStringRef( new QString( "not" ) ) );
	_keywords.insert( QStringRef( new QString( "or" ) ) );
	_keywords.insert( QStringRef( new QString( "true" ) ) );
	_keywords.insert( QStringRef( new QString( "nil" ) ) );
	_keywords.insert( QStringRef( new QString( "false" ) ) );
    _keywords.insert( QStringRef( new QString( "while" ) ) );
	_keywords.insert( QStringRef( new QString( "repeat" ) ) );
	_keywords.insert( QStringRef( new QString( "until" ) ) );

	_commentFormat.setForeground( QBrush( "#7D8C93" ) );
	_textFormat.setForeground( QBrush( "#EC7600" ) );

    _commentChar = new QString( "-" );
    _multilineBeginChar = new QString( "[" );
	_multilineBegin = QRegExp( "\\[=*\\[" );
	_multilineEnd = QString( "\\]={%1}\\]" );

	_quot = new QString( "\"" );
	_apos = new QString( "'" );
	_escape = new QString( "\\" );
}

void Highlighter::highlightBlock( const QString& text )
{
	int pos = 0;

	int state = previousBlockState();
	if( state > BlockState_Default ) {
		int level = state;
		if( state > BlockState_MaxMultilineComment ) {
			level -= BlockState_MultilineText;
        }

		QRegExp end( _multilineEnd.arg( level ) );
		int endPos = end.indexIn( text );
		if( endPos == -1 ) {
			setFormat( 0, text.size(), state > BlockState_MaxMultilineComment ? _textFormat : _commentFormat );
			setCurrentBlockState( state );
			return;
		}
		else {
			pos = endPos + end.matchedLength();
			setFormat( 0, pos, state > BlockState_MaxMultilineComment ? _textFormat : _commentFormat );
			++pos;
		}
	}

	while( ( pos = _symbols.indexIn( text, pos ) ) != -1 ) {
		QStringRef symbol = text.midRef( pos, _symbols.matchedLength() );
		if( symbol == _commentChar ) {
			int advance = pos + 1;
			if( advance < text.size() && text.midRef( advance, 1 ) == _commentChar ) {
				// Comments
				++advance;
				if( _multilineBegin.indexIn( text, advance ) == advance ) {
					int level = _multilineBegin.matchedLength() - 2;
					// ToDo: check max level
					QRegExp end( _multilineEnd.arg( level ) );
					advance += _multilineBegin.matchedLength();
					int endPos = end.indexIn( text, advance );
					if( endPos == -1 ) {
						// start multiline comment
						setFormat( pos, text.size() - pos, _commentFormat );
						setCurrentBlockState( level );
						return;
					}
					else {
						// single line comment, begin and end in one line
						endPos += end.matchedLength();
						setFormat( pos, endPos - pos, _commentFormat );
						pos = endPos;
						continue;
					}
				}
				else {
					// single line comment till end line
					setFormat( pos, text.size() - pos, _commentFormat );
					break;
				}
			}
		}
		else if( symbol == _multilineBeginChar ) {
			if( _multilineBegin.indexIn( text, pos ) == pos ) {
				// Text
				int level = _multilineBegin.matchedLength() - 2;
				// ToDo: check max level
				QRegExp end( _multilineEnd.arg( level ) );
				int advance = pos + _multilineBegin.matchedLength();
				int endPos = end.indexIn( text, advance );
				if( endPos == -1 ) {
					// start multiline text
					setFormat( pos, text.size() - pos, _textFormat );
					setCurrentBlockState( level + BlockState_MultilineText );
					return;
				}
				else {
					// single line text, begin and end in one line
					endPos += end.matchedLength();
					setFormat( pos, endPos - pos, _textFormat );
					pos = endPos;
					continue;
				}
			}
		}
		else if( symbol == _quot || symbol == _apos ) {
			// String
			int endPos = pos + 1;
			while( ( endPos = text.indexOf( symbol.toString(), endPos ) ) != -1) {
				// Check escape symbol
				int testPos = endPos - 1;
				int escape = 0;
				while( testPos > pos && text.midRef( testPos, 1 ) == _escape ) {
					--testPos;
					++escape;
				}
				if( ( escape & 1 ) == 0 )
					break;

				++endPos;
			}

			if( endPos == -1 ) {
				// Not closed string
				setFormat( pos, text.size() - pos, _textFormat );
				break;
			}
			else {
				// closed string
				setFormat( pos, endPos - pos + 1, _textFormat );
				pos = endPos + 1;
				continue;
			}
		}
		else if( _keywords.contains( symbol ) ) {
			setFormat( pos, _symbols.matchedLength(), _keywordFormat );
		}

		pos += _symbols.matchedLength();
	};

	if( currentBlockState() != BlockState_Default )
		setCurrentBlockState( BlockState_Default );
}

