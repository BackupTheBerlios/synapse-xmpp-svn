
include(../third-party/jrtplib)

VOIP_CPP = ../third-party/voip

#tins:{
#    DEFINES += TINS
#    HEADERS += $$VOIP_CPP/tins.h
#    SOURCES += $$VOIP_CPP/tins.cpp
#}

jingle:{
    DEFINES += JINGLE
}

INCLUDEPATH += $$VOIP_CPP ../third-party/jrtplib ../third-party/libjingle.new/libjingle/talk/session/phone/ $$VOIP_CPP/portaudio

unix:LIBS += -L../third-party/jrtplib -ljrtp 
# -L../iLBC -liLBC
unix:LIBS += -lspeex
unix:LIBS += -lssl

#win32:LIBS += -ljthread

win32 {
  INCLUDEPATH += C:\Speex\libspeex\include
  LIBS += C:\Speex\libspeex\libspeex.lib
  LIBS += ..\jrtplib\jrtp.lib
#  LIBS += ..\iLBC\iLBC.lib
}

HEADERS += \
#    $$VOIP_CPP/callslog.h \
    $$VOIP_CPP/voicecodec.h \
    $$VOIP_CPP/codecs/pcmucodec.h \
    $$VOIP_CPP/codecs/speexcodec.h \
#    $$VOIP_CPP/codecs/ilbccodec.h \
#    $$VOIP_CPP/sdp.h \
#    $$VOIP_CPP/udp.h \
#    $$VOIP_CPP/stun.h \
    $$VOIP_CPP/ringbuffer.h \
    $$VOIP_CPP/mediastream.h \
#    $$VOIP_CPP/jabbinmediaengine.h \

SOURCES += \
#    $$VOIP_CPP/callslog.cpp \
    $$VOIP_CPP/voicecodec.cpp \
    $$VOIP_CPP/codecs/pcmucodec.cpp \
    $$VOIP_CPP/codecs/speexcodec.cpp \
#    $$VOIP_CPP/codecs/ilbccodec.cpp \
#    $$VOIP_CPP/sdp.cpp \
#    $$VOIP_CPP/udp.cpp \
#    $$VOIP_CPP/stun.cpp \
    $$VOIP_CPP/ringbuffer.cpp \
    $$VOIP_CPP/mediastream.cpp \
#    $$VOIP_CPP/jabbinmediaengine.cpp \

#portaudio

HEADERS += \
    $$VOIP_CPP/portaudio/pa_host.h \
    $$VOIP_CPP/portaudio/pa_trace.h \
    $$VOIP_CPP/portaudio/portaudio.h

unix: HEADERS += $$VOIP_CPP/portaudio/pa_unix.h
    
SOURCES += \
    $$VOIP_CPP/portaudio/pa_convert.c \
    $$VOIP_CPP/portaudio/pa_lib.c \
    $$VOIP_CPP/portaudio/pa_trace.c 

unix: SOURCES += $$VOIP_CPP/portaudio/pa_unix.c $$VOIP_CPP/portaudio/pa_unix_oss.c
win32:SOURCES += $$VOIP_CPP/portaudio/pa_win_wmme.c
    
#INTERFACES += $$VOIP_CPP/callslogdialogbase.ui

#The following line was inserted by qt3to4
QT += qt3support 
#The following line was inserted by qt3to4
QT +=  
