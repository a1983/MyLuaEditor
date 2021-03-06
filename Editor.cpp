#include "Editor.h"

#include <QPainter>
#include <QTextBlock>

#include "LineNumberArea.h"

Editor::Editor( QWidget* parent) :
	QPlainTextEdit( parent )
{
	lineNumberArea = new LineNumberArea( this );

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	updateLineNumberAreaWidth( 0 );
	highlightCurrentLine();
}

void Editor::setLineNumberForeground( const QColor& color )
{
	_lineNumberForeground = color;
}

void Editor::setLineNumberBackground( const QColor& color )
{
	_lineNumberBackground = color;
}

void Editor::setLineNumberFont( const QFont& font )
{
	lineNumberArea->setFont( font );
}

void Editor::setCurrentLineFormat( const QTextCharFormat& format )
{
	_currentLineFormat = format;
}

int Editor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax( 1, blockCount() );
	while( max >= 10 ) {
		max /= 10;
		++digits;
	}

	int space = 3 + fontMetrics().width( QLatin1Char( '9' ) ) * digits;

	return space;
}

void Editor::updateLineNumberAreaWidth( int /* newBlockCount */ )
{
	setViewportMargins( lineNumberAreaWidth(), 0, 0, 0 );
}

void Editor::updateLineNumberArea( const QRect& rect, int dy )
{
	if( dy )
		lineNumberArea->scroll( 0, dy );
	else
		lineNumberArea->update( 0, rect.y(), lineNumberArea->width(), rect.height() );

	if( rect.contains(viewport()->rect()) )
		updateLineNumberAreaWidth(0);
}

void Editor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void Editor::highlightCurrentLine()
{
	QList< QTextEdit::ExtraSelection > extraSelections;

	if( !isReadOnly() ) {
		QTextEdit::ExtraSelection selection;

		selection.format = _currentLineFormat;
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append( selection );
	}

	setExtraSelections( extraSelections );
}

void Editor::lineNumberAreaPaintEvent( QPaintEvent* event )
{
	QPainter painter( lineNumberArea );
	painter.fillRect( event->rect(), _lineNumberBackground );

	int currentBlockNumber = textCursor().block().blockNumber();

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = ( int )blockBoundingGeometry( block ).translated( contentOffset() ).top();
	int bottom = top + (int) blockBoundingRect( block ).height();
	int height = fontMetrics().height();
	int width = lineNumberArea->width() - 2;

	painter.setPen( _lineNumberForeground );


	while( block.isValid() && top <= event->rect().bottom() ) {
		if( block.isVisible() && bottom >= event->rect().top() ) {
			QString number = QString::number( blockNumber + 1 );
			if( blockNumber == currentBlockNumber ) {
				QFont font = lineNumberArea->font();
				font.setBold( true );
				painter.setFont( font );
				painter.drawText( 0, top, width, height,
								  Qt::AlignRight | Qt::AlignVCenter, number );
				font.setBold( false );
				painter.setFont( font );
			} else {
				painter.drawText( 0, top, width, height,
								  Qt::AlignRight | Qt::AlignVCenter, number );
			}
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect( block ).height();
		++blockNumber;
	}
}
