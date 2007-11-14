include_directories(capabilities)

#/*ADD_DEFINITIONS("-DWIDGET_PLUGIN")*/
set(PWD capabilities)

set(capabilities_SRCS
	${PWD}/capsspec.cpp
	${PWD}/capsmanager.cpp
	${PWD}/capsregistry.cpp
)

set(capabilities_noMOC_SRCS
)

set(capabilities_H
	${PWD}/capsspec.h
	${PWD}/capsmanager.h
	${PWD}/capsregistry.h
)

#qt4_automoc(widgets_SRCS)
MY_AUTOMOC(capabilities_SRCS)
QT4_WRAP_CPP(capabilities_moc_SRCS ${capabilities_H})

set(capabilities_SRCS ${capabilities_SRCS} ${capabilities_moc_SRCS} ${capabilities_noMOC_SRCS})

#LINK_DIRECTORIES( ../lib )

#add_library(widgets SHARED ${widgets_SRCS})

#TARGET_LINK_LIBRARIES(widgets
#	${QT_LIBRARIES}
#	${QT_QT3SUPPORT_LIBRARY}
#	tools
#)
