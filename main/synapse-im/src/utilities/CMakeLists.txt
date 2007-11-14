include_directories(utilities)

#/*ADD_DEFINITIONS("-DWIDGET_PLUGIN")*/
set(PWD utilities)

set(utilities_SRCS
	${PWD}/iodeviceopener.cpp
)

set(utilities_noMOC_SRCS
)

set(utilities_H
	${PWD}/iodeviceopener.h
	${PWD}/maybe.h
)

#qt4_automoc(widgets_SRCS)
MY_AUTOMOC(utilities_SRCS)
QT4_WRAP_CPP(utilities_moc_SRCS ${utilities_H})

set(utilities_SRCS ${utilities_SRCS} ${utilities_moc_SRCS} ${utilities_noMOC_SRCS})

#LINK_DIRECTORIES( ../lib )

#add_library(widgets SHARED ${widgets_SRCS})

#TARGET_LINK_LIBRARIES(widgets
#	${QT_LIBRARIES}
#	${QT_QT3SUPPORT_LIBRARY}
#	tools
#)
