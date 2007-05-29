include_directories(tools/trayicon)
set(trayicon_SRCS tools/trayicon/trayicon.cpp )

if (WIN32)
set(trayicon_SRCS ${trayicon_SRCS} tools/trayicon/trayicon_win.cpp )
else (WIN32)
set(trayicon_SRCS ${trayicon_SRCS} tools/trayicon/trayicon_x11.cpp )
endif (WIN32)

set(trayicon_H tools/trayicon/trayicon.h )

