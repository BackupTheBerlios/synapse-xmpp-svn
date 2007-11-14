include_directories(irisprotocol)

set(PWD irisprotocol)

#/*ADD_DEFINITIONS("-DWIDGET_PLUGIN")*/

set(irisprotocol_SRCS
    ${PWD}/iris_discoinfoquerier.cpp
)

set(irisprotocol_noMOC_SRCS
)

set(irisprotocol_H
    ${PWD}/iris_discoinfoquerier.h
)

#qt4_automoc(widgets_SRCS)
MY_AUTOMOC(irisprotocol_SRCS)
QT4_WRAP_CPP(irisprotocol_moc_SRCS ${irisprotocol_H})

set(irisprotocol_SRCS ${irisprotocol_SRCS} ${irisprotocol_moc_SRCS} ${irisprotocol_noMOC_SRCS})

#LINK_DIRECTORIES( ../lib )

#add_library(widgets SHARED ${widgets_SRCS})

#TARGET_LINK_LIBRARIES(widgets
#	${QT_LIBRARIES}
#	${QT_QT3SUPPORT_LIBRARY}
#	tools
#)
