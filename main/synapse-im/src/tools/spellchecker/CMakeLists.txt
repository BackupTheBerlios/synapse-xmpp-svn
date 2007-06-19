include_directories(tools/spellchecker)

if(ASPELL_FOUND)
ADD_DEFINITIONS("-DHAVE_ASPELL")
INCLUDE_DIRECTORIES( ${ASPELL_INCLUDE_DIR} )
set(aspell_noMOC_SRCS
	tools/spellchecker/aspellchecker.cpp
)

endif(ASPELL_FOUND)

set(spellchecker_noMOC_SRCS
	tools/spellchecker/spellchecker.cpp
	tools/spellchecker/spellhighlighter.cpp
	${aspell_noMOC_SRCS}
)

set(spellchecker_H
#	tools/spellchecker/spellchecker.h
#	tools/spellchecker/spellhighlighter.h
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