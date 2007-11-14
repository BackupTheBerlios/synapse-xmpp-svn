include_directories(tabs)

#/*ADD_DEFINITIONS("-DWIDGET_PLUGIN")*/
set(PWD tabs)

set(tabs_SRCS
	${PWD}/tabbablewidget.cpp
	${PWD}/tabdlg.cpp
	${PWD}/tabmanager.cpp
)

set(tabs_noMOC_SRCS
)

set(tabs_H
	${PWD}/tabbablewidget.h
	${PWD}/tabdlg.h
	${PWD}/tabmanager.h
)

#qt4_automoc(widgets_SRCS)
MY_AUTOMOC(tabs_SRCS)
QT4_WRAP_CPP(tabs_moc_SRCS ${tabs_H})

set(tabs_SRCS ${tabs_SRCS} ${tabs_moc_SRCS} ${tabs_noMOC_SRCS})

#LINK_DIRECTORIES( ../lib )

#add_library(widgets SHARED ${widgets_SRCS})

#TARGET_LINK_LIBRARIES(widgets
#	${QT_LIBRARIES}
#	${QT_QT3SUPPORT_LIBRARY}
#	tools
#)
