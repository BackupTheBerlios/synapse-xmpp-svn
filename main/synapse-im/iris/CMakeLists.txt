project(iris)

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

include_directories(
../iris/include
../iris/xmpp-core
../iris/xmpp-im
../iris/jabber
../iris/libidn
${QT_INCLUDE_DIR}
${QT_CORE_INCLUDE_DIR}
${QT_QTNETWORK_INCLUDE_DIR}
${QT_QTSQL_INCLUDE_DIR}
${QT_QTXML_INCLUDE_DIR}
${QT_QT3SUPPORT_INCLUDE_DIR}
../src/tools/zip/minizip/win32
${CMAKE_CURRENT_BINARY_DIR}
#../third-party/qca/qca/include/QtCrypto
#${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/iris.dir/irisnet
${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/iris.dir/xmpp-core
${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/iris.dir/xmpp-im
${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/iris.dir/jabber
)

if (irisnet)
    include(../iris/irisnet/CMakeLists.txt)
endif (irisnet)

include(../cutestuff/CMakeLists.txt)

set(idn_SRCS
	../iris/libidn/profiles.c
	#$$LIBIDN_BASE/toutf8.c \
	../iris/libidn/rfc3454.c
	../iris/libidn/nfkc.c
	../iris/libidn/stringprep.c
)

set(iris_H
	../iris/xmpp-core/securestream.h
	../iris/xmpp-core/parser.h
	../iris/xmpp-core/xmlprotocol.h
	../iris/xmpp-core/protocol.h
	../iris/xmpp-core/compressionhandler.h
	../iris/xmpp-core/compress.h
	../iris/xmpp-core/td.h
	../iris/xmpp-im/xmpp_tasks.h
	../iris/xmpp-im/xmpp_discoinfotask.h
	../iris/xmpp-im/xmpp_xmlcommon.h
	../iris/xmpp-im/xmpp_vcard.h
	../iris/jabber/s5b.h
	../iris/jabber/xmpp_ibb.h
	../iris/jabber/filetransfer.h 
	../iris/include/xmpp.h
	../iris/include/xmpp_jid.h
	../iris/include/xmpp_url.h
	../iris/include/xmpp_chatstate.h
	../iris/include/xmpp_client.h
	../iris/include/xmpp_clientstream.h
	../iris/include/xmpp_stanza.h
	../iris/include/xmpp_stream.h
	../iris/include/xmpp_address.h
	../iris/include/xmpp_htmlelement.h
	../iris/include/xmpp_muc.h
	../iris/include/xmpp_message.h
	../iris/include/xmpp_pubsubitem.h
	../iris/include/xmpp_resource.h
	../iris/include/xmpp_roster.h
	../iris/include/xmpp_rosterx.h
	../iris/include/xmpp_xdata.h
	../iris/include/xmpp_rosteritem.h
	../iris/include/xmpp_liveroster.h
	../iris/include/xmpp_liverosteritem.h
	../iris/include/xmpp_resourcelist.h
	../iris/include/xmpp_task.h
	../iris/include/xmpp_httpauthrequest.h
	../iris/include/xmpp_status.h
	../iris/include/xmpp_features.h
	../iris/include/xmpp_agentitem.h
	../iris/include/xmpp_discoitem.h
	../iris/include/im.h
)

set(iris_SRCS ${cutestuff_SRCS} ${idn_SRCS}
	../iris/xmpp-core/securestream.cpp
	../iris/xmpp-core/connector.cpp
	../iris/jabber/s5b.cpp
	../iris/xmpp-im/xmpp_features.cpp
	../iris/xmpp-core/tlshandler.cpp
	../iris/xmpp-core/jid.cpp
	../iris/xmpp-core/parser.cpp
	../iris/xmpp-core/xmlprotocol.cpp
	../iris/xmpp-core/protocol.cpp
	../iris/xmpp-core/compress.cpp
	../iris/xmpp-core/compressionhandler.cpp
	../iris/xmpp-core/stream.cpp
	../iris/xmpp-core/simplesasl.cpp
	../iris/xmpp-core/xmpp_stanza.cpp
	../iris/xmpp-im/types.cpp
	../iris/xmpp-im/client.cpp
	../iris/xmpp-im/xmpp_discoitem.cpp
	../iris/xmpp-im/xmpp_discoinfotask.cpp
	../iris/xmpp-im/xmpp_xdata.cpp
	../iris/xmpp-im/xmpp_task.cpp
	../iris/xmpp-im/xmpp_tasks.cpp
	../iris/xmpp-im/xmpp_xmlcommon.cpp
	../iris/xmpp-im/xmpp_vcard.cpp
	../iris/jabber/xmpp_ibb.cpp
	../iris/jabber/filetransfer.cpp
)

qt4_wrap_cpp(iris_MOC_SRCS ${iris_H})
qt4_wrap_cpp(cutestuff_MOC_SRCS ${cutestuff_H})

MY_AUTOMOC(iris_SRCS)

#add_library(iris ${iris_SRCS} ${iris_mocFix_SRCS})
#add_library(iris SHARED ${iris_SRCS} ${iris_MOC_SRCS} ${cutestuff_MOC_SRCS})

#set(iris_S ${iris_SRCS})

#set(iris_HS ${iris_H} ${cutestuff_H})

if(WIN32)
add_library(iris ${iris_SRCS} ${iris_MOC_SRCS} ${cutestuff_MOC_SRCS})
TARGET_LINK_LIBRARIES(iris
	${QT_LIBRARIES}
	${QT_QTNETWORK_LIBRARIES}
	${QT_QTXML_LIBRARIES}
	${QT_QT3SUPPORT_LIBRARIES}
	qca
	WSock32
)
else(WIN32)
add_library(iris SHARED ${iris_SRCS} ${iris_MOC_SRCS} ${cutestuff_MOC_SRCS})
target_link_libraries(iris 
	${QT_LIBRARIES}
	${QT_QTNETWORK_LIBRARIES}
	${QT_QTXML_LIBRARIES}
	${QT_QT3SUPPORT_LIBRARIES}
	qca
)
endif(WIN32)

set_target_properties(iris PROPERTIES VERSION 0.3.0 SOVERSION 3 )

INSTALL(TARGETS iris DESTINATION ${LIB_INSTALL_DIR} )
