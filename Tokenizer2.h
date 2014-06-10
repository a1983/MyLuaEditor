#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <QString>

class Tokenizer2
{
public:
    Tokenizer2();

	bool HasNext() const;

	QStringRef operator()();

private:
	QChar* c;
	QChar* b;
	QChar* e;
	QString source;
};

#endif // TOKENIZER_H
