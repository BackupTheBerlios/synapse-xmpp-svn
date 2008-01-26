include_directories(archiving)

ADD_DEFINITIONS(-DXEP-0136)

set(DIR archiving)

set(archiving_SRCS
	${DIR}/archivedlg.cpp
	${DIR}/getcollectionlisttask.cpp
	${DIR}/getcollectiontask.cpp
)

set(archiving_H
	${DIR}/archivedlg.h
	${DIR}/getcollectionlisttask.h
	${DIR}/getcollectiontask.h
)

set(archiving_UIS
	${DIR}/archivedlg.ui
)

QT4_WRAP_CPP(archiving_moc_SRCS ${archiving_H})

QT4_WRAP_UI(archiving_UIS_H ${archiving_UIS})

MY_AUTOMOC(archiving_SRCS)


