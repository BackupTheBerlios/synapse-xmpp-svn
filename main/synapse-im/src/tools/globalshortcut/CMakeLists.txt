include_directories(tools/globalshortcut)

set(globalshortcut_noMOC_SRCS
    tools/globalshortcut/globalshortcutmanager.cpp
)

if (WIN32)
set(globalshortcut_noMOC_SRCS ${globalshortcut_noMOC_SRCS}
    tools/globalshortcut/globalshortcutmanager_win.cpp
)
else (WIN32)
set(globalshortcut_noMOC_SRCS ${globalshortcut_noMOC_SRCS}
    tools/globalshortcut/globalshortcutmanager_x11.cpp
)
endif (WIN32)

set(globalshortcut_H
#    tools/globalshortcut/globalshortcutmanager.h
    tools/globalshortcut/globalshortcuttrigger.h
)
