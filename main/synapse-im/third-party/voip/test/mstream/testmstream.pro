TEMPLATE = app

SOURCES = testmstream.cpp
TINS_CPP = ../..
CONFIG += debug
CONFIG -= release

DEFINES += TINS

INCLUDEPATH += $$TINS_CPP $$TINS_CPP/jrtplib $$TINS_CPP/portaudio

LIBS += -L$$TINS_CPP/jrtplib
LIBS += -L$$TINS_CPP/jrtplib -ljrtp 
unix:LIBS += -lspeex
win32:LIBS += -llibspeex

HEADERS += \
    $$TINS_CPP/voicecodec.h \
    $$TINS_CPP/codecs/pcmucodec.h \
    $$TINS_CPP/codecs/speexcodec.h \
    $$TINS_CPP/sdp.h \
    $$TINS_CPP/ringbuffer.h \
    $$TINS_CPP/mediastream.h 

SOURCES += \
    $$TINS_CPP/voicecodec.cpp \
    $$TINS_CPP/codecs/pcmucodec.cpp \
    $$TINS_CPP/codecs/speexcodec.cpp \
    $$TINS_CPP/sdp.cpp \
    $$TINS_CPP/udp.cpp \
    $$TINS_CPP/ringbuffer.cpp \
    $$TINS_CPP/mediastream.cpp

#portaudio

HEADERS += \
    $$TINS_CPP/portaudio/pa_host.h \
    $$TINS_CPP/portaudio/pa_trace.h \
    $$TINS_CPP/portaudio/portaudio.h

unix: HEADERS += $$TINS_CPP/portaudio/pa_unix.h
    
SOURCES += \
    $$TINS_CPP/portaudio/pa_convert.c \
    $$TINS_CPP/portaudio/pa_lib.c \
    $$TINS_CPP/portaudio/pa_trace.c 

unix: SOURCES += $$TINS_CPP/portaudio/pa_unix.c $$TINS_CPP/portaudio/pa_unix_oss.c
win32:SOURCES += $$TINS_CPP/portaudio/pa_win_wmme.c
    
win32:QMAKE_CFLAGS	+= -GR -GX -DWIN32
win32:QMAKE_CXXFLAGS	+= -GR -GX -DWIN32
win32:CONFIG += console

