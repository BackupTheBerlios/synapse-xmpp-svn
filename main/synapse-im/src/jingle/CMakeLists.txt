# Portaudio
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	../third-party/jrtplib
)
set(portaudio_SRCS
	jingle/portaudio/pa_convert.c
	jingle/portaudio/pa_lib.c
	jingle/portaudio/pa_trace.c
)

set(portaudio_H
	jingle/portaudio/pa_host.h
	jingle/portaudio/pa_trace.h
	jingle/portaudio/portaudio.h
)

if(WIN32)
	set(portaudio_SRCS ${portaudio_SRCS}
		jingle/portaudio/pa_win_wmme.c
	)
else(WIN32)
	set(portaudio_SRCS ${portaudio_SRCS}
		jingle/portaudio/pa_unix.c
		jingle/portaudio/pa_unix_oss.c
	)
	set(portaudio_H ${portaudio_H}
		jingle/portaudio/pa_unix.h
	)
endif(WIN32)

#JINGLE
set(jingle_SRCS
	jingle/jinglesessionmanager.cpp
	jingle/jinglevoicesession.cpp
	jingle/stun.cpp
	jingle/udp.cpp
	jingle/transport.cpp

	jingle/voicecodec.cpp
	jingle/ringbuffer.cpp
	jingle/mediastream.cpp

	${portaudio_SRCS}

# Codecs
	jingle/codecs/pcmucodec.cpp
	jingle/codecs/speexcodec.cpp
)

set(jingle_H
	jingle/jinglesessionmanager.h
	jingle/jinglevoicesession.h
	jingle/stun.h
	jingle/udp.h
	jingle/transport.h

	jingle/voicecodec.h
	jingle/ringbuffer.h
	jingle/mediastream.h

	${portaudio_H}

# Codecs
	jingle/codecs/pcmucodec.h
	jingle/codecs/speexcodec.h

)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

MY_AUTOMOC(jingle_SRCS)
QT4_WRAP_CPP(jingle_moc_SRCS ${jingle_H})

set(jingle_SRCS ${jingle_SRCS} ${jingle_moc_SRCS})


