#ifndef PARSER_H
#define PARSER_H

#include <QList>
#include <QString>

class Parser
{
public:
	enum TokenType {
		Unknown,
		Spacing,
		Comment,
		Lexems,
		String,
		Operator
	};

	struct Token {
		QChar* Begin;
		QChar* End;

		TokenType Type;

        const QString ToString() const;
        bool operator ==( const QString& string ) const;
        bool Is( const QChar& c ) const;

        QString DebugString() const;
	};

public:
	Parser( QString text );

	void Parse();

	QList< Token > Tokens() const;

private:
	void SkipToEndOfLine();
	void EndOfString( const ushort& quote );

	bool NotEscaped( QChar* current );

	bool TryParseSpace();
	bool TryParseComment();
	bool TryParseMultiLineComment();
	bool TryParseString();
	bool TryParseMultiLineString();
	bool TryParseLexems();
	bool TryParseOperator();
	bool TryParseBracket();

private:
	QString _text;
	QChar* p;

	QList< Token > _tokens;
};

#endif // PARSER_H
