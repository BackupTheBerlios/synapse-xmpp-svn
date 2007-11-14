include_directories(protocol)

set(PWD protocol)

#/*ADD_DEFINITIONS("-DWIDGET_PLUGIN")*/

set(protocol_SRCS
)

set(protocol_noMOC_SRCS
)

set(protocol_H
    ${PWD}/discoinfoquerier.h
)

#qt4_automoc(widgets_SRCS)
MY_AUTOMOC(protocol_SRCS)
QT4_WRAP_CPP(protocol_moc_SRCS ${protocol_H})

set(protocol_SRCS ${protocol_SRCS} ${protocol_moc_SRCS} ${protocol_noMOC_SRCS})

#LINK_DIRECTORIES( ../lib )

#add_library(widgets SHARED ${widgets_SRCS})

#TARGET_LINK_LIBRARIES(widgets
#	${QT_LIBRARIES}
#	${QT_QT3SUPPORT_LIBRARY}
#	tools
#)
