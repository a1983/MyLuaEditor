#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>
#include <QTextCharFormat>

class Editor : public QPlainTextEdit
{
	Q_OBJECT

public:
	Editor( QWidget* parent = 0 );

public:
	void setLineNumberForeground( const QColor& color );
	void setLineNumberBackground( const QColor& color );
	void setLineNumberFont( const QFont& font );
	void setCurrentLineFormat( const QTextCharFormat& format );

public:
	void lineNumberAreaPaintEvent( QPaintEvent *event );
	int lineNumberAreaWidth();

protected:
	void resizeEvent( QResizeEvent *event );

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect &, int);

private:
	QWidget *lineNumberArea;

	QTextCharFormat _currentLineFormat;
	QColor _lineNumberForeground;
	QColor _lineNumberBackground;
};

#endif // EDITOR_H
