#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QTextLayout>
#include <QTreeView>
#include <QVBoxLayout>

#include "Editor.h"

#include "Model/CodeModel2.h"
#include "Parser/AstParser2.h"

MainWindow::MainWindow( QWidget* parent )
	: QMainWindow( parent )
{
	setupFileMenu();
	setupHelpMenu();
	setupEditor();
	setupOutline();

	setCentralWidget( editor );
	setWindowTitle( tr( "Editor" ) );
}

void MainWindow::about()
{
	QMessageBox::about( this, tr( "About Editor" ),
				tr( "<p>The <b>Editor Lua Work</b>.</p>" ) );
}

void MainWindow::newFile()
{
	editor->clear();
}

void MainWindow::openFile(const QString &path)
{
	QString fileName = path;

	if( fileName.isNull() )
		fileName = QFileDialog::getOpenFileName( this,
												 tr( "Open File" ), "", "Lua Files (*.lua)" );
	if( !fileName.isEmpty() ) {
		QFile file( fileName );
		if( file.open( QFile::ReadOnly | QFile::Text ) ) {
			editor->setPlainText( file.readAll() );

			CodeModel2* model = qobject_cast< CodeModel2* >( treeView->model() );
			model->RebuildModel( editor->toPlainText() );
		}
	}
}

void MainWindow::setupEditor()
{
	QFont font;
	font.setFamily( "Consolas" );
	font.setFixedPitch( true );
	font.setPointSize( 12 );

	editor = new Editor;
	editor->setFont( font );
	editor->setStyleSheet( "QPlainTextEdit { color: #E0E2E4; background: #293134 }" );

	highlighter = new Highlighter( editor->document() );

	QFile file("mainwindow.h");
	if( file.open(QFile::ReadOnly | QFile::Text) )
		editor->setPlainText( file.readAll() );

	editor->setTabStopWidth( 36 );

	QTextCharFormat currentLineFormat;
	currentLineFormat.setBackground( QColor( "#2F393C" ) );
	currentLineFormat.setProperty( QTextFormat::FullWidthSelection, true );
	editor->setCurrentLineFormat( currentLineFormat );

	editor->setLineNumberForeground( QColor( "#81969A" ) );
	editor->setLineNumberBackground( QColor( "#293134" ).lighter( 130 ) );
	editor->setLineNumberFont( font );

	editor->setWordWrapMode( QTextOption::NoWrap );
}


void MainWindow::setupFileMenu()
{
	QMenu* fileMenu = new QMenu( tr( "&File" ), this);
	menuBar()->addMenu(fileMenu);

	fileMenu->addAction( tr( "&New" ), this, SLOT( newFile() ),			QKeySequence::New );
	fileMenu->addAction( tr( "&Open..." ), this, SLOT( openFile() ),	QKeySequence::Open );
	fileMenu->addAction( tr( "E&xit" ), qApp, SLOT( quit() ),			QKeySequence::Quit );
}

void MainWindow::setupHelpMenu()
{
	QMenu* helpMenu = new QMenu( tr( "&Help" ), this);
	menuBar()->addMenu( helpMenu );

	helpMenu->addAction( tr("&About"), this, SLOT( about() ) );
	helpMenu->addAction( tr("About &Qt"), qApp, SLOT( aboutQt() ) );
}

void MainWindow::setupOutline()
{
	QDockWidget* dock = new QDockWidget( "Outline", this );
	addDockWidget( Qt::LeftDockWidgetArea, dock, Qt::Vertical );

	treeView = new QTreeView( this );
	treeView->setHeaderHidden( true );
	CodeModel2* model = new CodeModel2();
	treeView->setModel( model );

	dock->setWidget( treeView );
}
