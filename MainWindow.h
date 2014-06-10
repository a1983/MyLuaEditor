#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "highlighter.h"

#include <QMainWindow>

class Editor;
class QTreeView;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow( QWidget* parent = 0);

public slots:
	void about();
	void newFile();
	void openFile( const QString& path = QString() );

private:
	void setupEditor();
	void setupFileMenu();
	void setupHelpMenu();
    void setupOutline();

	Editor*				editor;
	Highlighter*		highlighter;
    QTreeView*          treeView;
};


#endif // MAINWINDOW_H
