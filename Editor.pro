
QT += widgets
QT += concurrent

win32-g++ : {
    QMAKE_CXXFLAGS += -std=c++11
}

DESTDIR = ../Editor/bin

HEADERS +=              \
	$$PWD/*.h           \
	$$PWD/Data/*.h		\
	$$PWD/Lexer/*.h		\
	$$PWD/Parser/*.h	\
	$$PWD/Model/*.h		\

SOURCES +=              \
	$$PWD/*.cpp         \
	$$PWD/Data/*.cpp	\
	$$PWD/Lexer/*.cpp	\
	$$PWD/Parser/*.cpp	\
	$$PWD/Model/*.cpp	\
