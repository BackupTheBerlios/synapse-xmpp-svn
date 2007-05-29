include_directories(tools/spellchecker)

set(spellchecker_SRCS
	tools/spellchecker/spellchecker.cpp
	tools/spellchecker/spellhighlighter.cpp
)

set(spellchecker_H
	tools/spellchecker/spellchecker.h
	tools/spellchecker/spellhighlighter.h
)


#if (WIN32)
#set(synapse-im_SRCS ${synapse-im_SRCS}
#    tools/spellchecker/systemwatch_win.cpp
#)
#else (WIN32)
#set(synapse-im_SRCS ${synapse-im_SRCS}
#    tools/spellchecker/systemwatch_unix.cpp
#)
#endif (WIN32)