#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>

class Editor;

class LineNumberArea : public QWidget
{
public:
	LineNumberArea( Editor *editor );

	QSize sizeHint() const;

protected:
	void paintEvent( QPaintEvent* event );

private:
	Editor* _editor;
};
#endif // LINENUMBERAREA_H
