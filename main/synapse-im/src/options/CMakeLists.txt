
#set(QT_USE_OPENGL TRUE)

include(${QT_USE_FILE})
ADD_DEFINITIONS(-DQT3_SUPPORT)

#include_directories(${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES(
    ${QT_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}
    ${QT_QTSQL_INCLUDE_DIR}
    ${QT_QTXML_INCLUDE_DIR}
    ${QT_QT3SUPPORT_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    options
    ../third-party/qca/qca/include/QtCrypto
    ../cutestuff/legacy
    ../cutestuff/network
    ../cutestuff/util
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/synapse-im.dir
)

set(options_SRCS
	options/optionstab.cpp
	options/optionsdlg.cpp 
	options/opt_application.cpp
	options/opt_chat.cpp
	options/opt_events.cpp
	options/opt_status.cpp
	options/opt_appearance.cpp
	options/opt_iconset.cpp
	options/opt_groupchat.cpp
	options/opt_sound.cpp
	options/opt_toolbars.cpp
	options/opt_advanced.cpp
	options/opt_shortcuts.cpp

)

set(options_H
	options/optionstab.h
	options/optionsdlg.h 
	options/opt_application.h
	options/opt_chat.h
	options/opt_events.h
	options/opt_status.h
	options/opt_appearance.h
	options/opt_iconset.h
	options/opt_groupchat.h
	options/opt_sound.h
	options/opt_toolbars.h
	options/opt_advanced.h
	options/opt_shortcuts.h
)

set(options_UIS
	options/opt_application.ui
	options/opt_chat.ui
	options/opt_events.ui
	options/opt_status.ui
	options/opt_appearance.ui
	options/opt_appearance_misc.ui
	options/opt_sound.ui
	options/opt_advanced.ui
	options/opt_lookfeel_toolbars.ui
	options/ui_positiontoolbar.ui
	options/ui_isdetails.ui
	options/opt_iconset_emo.ui
	options/opt_iconset_system.ui
	options/opt_iconset_roster.ui
	options/opt_general_groupchat.ui
	options/opt_shortcuts.ui
)

MY_AUTOMOC(options_SRCS)
QT4_WRAP_CPP(options_moc_SRCS ${options_H})
QT4_WRAP_UI(options_UIS_H ${options_UIS})

set(options_SRCS ${options_SRCS} ${options_moc_SRCS})
