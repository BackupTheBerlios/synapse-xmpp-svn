include_directories(tools/systemwatch)

set(systemwatch_SRCS
    tools/systemwatch/systemwatch.cpp
)

set(systemwatch_H
    tools/systemwatch/systemwatch.h
)

if (WIN32)
set(systemwatch_SRCS ${systemwatch_SRCS}
    tools/systemwatch/systemwatch_win.cpp
)
set(systemwatch_H ${systemwatch_H}
    tools/systemwatch/systemwatch_win.h
)
else (WIN32)
set(systemwatch_SRCS ${systemwatch_SRCS}
    tools/systemwatch/systemwatch_unix.cpp
)
set(systemwatch_H ${systemwatch_H}
    tools/systemwatch/systemwatch_unix.h
)
endif (WIN32)