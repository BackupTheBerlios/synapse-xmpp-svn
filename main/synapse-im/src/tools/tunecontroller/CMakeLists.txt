include_directories(tools/tunecontroller)

set(tunecontroller_noMOC_SRCS
	tools/tunecontroller/pollingtunecontroller.cpp
	tools/tunecontroller/filetunecontroller.cpp
	tools/tunecontroller/combinedtunecontroller.cpp
	tools/tunecontroller/tunecontrollermanager.cpp
)

set(tunecontroller_H
#	tools/tunecontroller/tune.h
	tools/tunecontroller/tunecontroller.h
	tools/tunecontroller/pollingtunecontroller.h
#	tools/tunecontroller/filetunecontroller.h
#	tools/tunecontroller/combinedtunecontroller.h
#	tools/tunecontroller/tunecontrollerplugin.h
#	tools/tunecontroller/tunecontrollermanager.h
#	tools/tunecontroller/tunecontrollerinterface.h
)

if (WIN32)
add_definitions(-DTC_WINAMP)
set(tunecontroller_noMOC_SRCS ${tunecontroller_noMOC_SRCS}
	tools/tunecontroller/plugins/winamp/winampcontroller.cpp
	tools/tunecontroller/plugins/winamp/winampplugin.cpp
)
set(tunecontroller_H ${tunecontroller_H}
	tools/tunecontroller/plugins/winamp/winampcontroller.h 
)
else (WIN32)
add_definitions(-DTC_PSIFILE)
set(tunecontroller_SRCS ${tunecontroller_SRCS}
	tools/tunecontroller/plugins/psifile/psifilecontroller.cpp
	tools/tunecontroller/plugins/psifile/psifileplugin.cpp
)
set(tunecontroller_H ${tunecontroller_H}
	tools/tunecontroller/plugins/psifile/psifilecontroller.h
)
endif (WIN32)