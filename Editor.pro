
QT += widgets
QT += concurrent

DESTDIR = ${PWD}/../../Editor/bin

HEADERS +=              \
	$$PWD/*.h           \
	$$PWD/Data/*.h		\
	$$PWD/Lexer/*.h		\
	$$PWD/Parser/*.h	\

SOURCES +=              \
	$$PWD/*.cpp         \
	$$PWD/Data/*.cpp	\
	$$PWD/Lexer/*.cpp	\
	$$PWD/Parser/*.cpp	\
