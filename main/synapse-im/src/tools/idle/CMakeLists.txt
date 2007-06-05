include_directories(tools/idle)

set(idle_noMOC_SRCS
    tools/idle/idle.cpp
)

if (WIN32)
set(idle_noMOC_SRCS ${idle_noMOC_SRCS}
    tools/idle/idle_win.cpp
)
else (WIN32)
set(idle_noMOC_SRCS ${idle_noMOC_SRCS}
    tools/idle/idle_x11.cpp
)
endif (WIN32)

set(idle_H
    tools/idle/idle.h
)
