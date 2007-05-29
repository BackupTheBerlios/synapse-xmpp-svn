PROJECT(jrtp)

MACRO(MY_AUTOMOC _srcsList)
  QT4_GET_MOC_INC_DIRS(_moc_INCS)
  FOREACH (_current_FILE ${${_srcsList}})
    GET_FILENAME_COMPONENT(_abs_FILE ${_current_FILE} ABSOLUTE)
    GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)
    SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)
    SET(extra_moc_argument)
    if(WIN32)
	SET(extra_moc_argument -DWIN32)
    endif(WIN32)
    ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
                       COMMAND ${QT_MOC_EXECUTABLE}
		       ARGS ${extra_moc_argument} ${_moc_INCS} -o ${_moc} ${_abs_FILE}
		       DEPENDS ${_current_FILE}
    )
    LIST(APPEND ${_srcsList} ${_moc})
  ENDFOREACH (_current_FILE)
ENDMACRO(MY_AUTOMOC)

include(${QT_USE_FILE})
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT3_SUPPORT)

include_directories(../jrtplib)

set(jrtp_H
	rtcpapppacket.h
	rtcpbyepacket.h
	rtcpcompoundpacket.h
	rtcpcompoundpacketbuilder.h
	rtcppacket.h
	rtcppacketbuilder.h
	rtcprrpacket.h
	rtcpscheduler.h
	rtcpsdesinfo.h
	rtcpsdespacket.h
	rtcpsrpacket.h
	rtcpunknownpacket.h
	rtpaddress.h
	rtpcollisionlist.h
	rtpconfig.h
	rtpdebug.h
	rtpdefines.h
	rtperrors.h
	rtphashtable.h
	rtpinternalsourcedata.h
	rtpipv4address.h
	rtpipv4destination.h
	rtpipv6address.h
	rtpipv6destination.h
	rtpkeyhashtable.h
	rtplibraryversion.h
	rtppacket.h
	rtppacketbuilder.h
	rtppollthread.h
	rtprandom.h
	rtprawpacket.h
	rtpsession.h
	rtpsessionparams.h
	rtpsessionsources.h
	rtpsourcedata.h
	rtpsources.h
	rtpstructs.h
	rtptimeutilities.h
	rtptransmitter.h
	rtptypes.h
	rtptypes_unix.h
	rtptypes_win.h
	rtpudpv4transmitter.h
	rtpudpv6transmitter.h
)

set(jrtp_SRCS
	rtcpapppacket.cpp
	rtcpbyepacket.cpp
	rtcpcompoundpacket.cpp
	rtcpcompoundpacketbuilder.cpp
	rtcppacket.cpp
	rtcppacketbuilder.cpp
	rtcprrpacket.cpp
	rtcpscheduler.cpp
	rtcpsdesinfo.cpp
	rtcpsdespacket.cpp
	rtcpsrpacket.cpp
	rtpcollisionlist.cpp
	rtpdebug.cpp
	rtperrors.cpp
	rtpinternalsourcedata.cpp
	rtpipv4address.cpp
	rtpipv6address.cpp
	rtplibraryversion.cpp
	rtppacket.cpp
	rtppacketbuilder.cpp
	rtppollthread.cpp
	rtprandom.cpp
	rtpsession.cpp
	rtpsessionparams.cpp
	rtpsessionsources.cpp
	rtpsourcedata.cpp
	rtpsources.cpp
	rtptimeutilities.cpp
	rtpudpv4transmitter.cpp
	rtpudpv6transmitter.cpp
)

QT4_WRAP_CPP(jrtp_moc_SRCS ${jrtp_H})
MY_AUTOMOC(jrtp_SRCS)

add_library(jrtp SHARED ${jrtp_SRCS} ${jrtp_moc_SRCS})

target_link_libraries(jrtp
	${QT_LIBRARIES}
	${QT_QTNETWORK_LIBRARIES}
)

set_target_properties(jrtp PROPERTIES VERSION 0.3.0 SOVERSION 3 )

INSTALL(TARGETS jrtp DESTINATION ${LIB_INSTALL_DIR} )
