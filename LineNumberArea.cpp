#include "LineNumberArea.h"

#include "Editor.h"

LineNumberArea::LineNumberArea( Editor *editor ) :
	QWidget( editor ),
	_editor( editor )
{
}

QSize LineNumberArea::sizeHint() const
{
	return QSize( _editor->lineNumberAreaWidth(), 0 );
}

void LineNumberArea::paintEvent(QPaintEvent* event)
{
	_editor->lineNumberAreaPaintEvent( event );
}
