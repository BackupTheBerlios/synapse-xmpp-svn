win:contains(DEFINED, HAVE_SNARL)
{
	INCLUDEPATH += $$PWD
	DEPENDPATH  += $$PWD

	HEADERS += $$PWD/SnarlInterface.h
	SOURCES += $$PWD/SnarlInterface.cpp
	#QMAKE_LFLAGS += -framework Growl -framework CoreFoundation
}