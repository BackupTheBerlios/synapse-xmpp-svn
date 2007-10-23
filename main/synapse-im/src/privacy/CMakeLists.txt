include_directories(privacy)

set(privacy_SRCS
	privacy/privacylistitem.cpp
	privacy/privacylist.cpp
	privacy/privacylistmodel.cpp
	privacy/privacylistblockedmodel.cpp
	privacy/psiprivacymanager.cpp
	privacy/privacydlg.cpp
	privacy/privacyruledlg.cpp
)

set(privacy_H
	privacy/privacylistitem.h
	privacy/privacylist.h
	privacy/privacylistmodel.h
	privacy/privacylistblockedmodel.h
	privacy/privacymanager.h
	privacy/psiprivacymanager.h
	privacy/privacydlg.h
	privacy/privacyruledlg.h
)

set(privacy_UIS
	privacy/privacy.ui
	privacy/privacyrule.ui
)

QT4_WRAP_CPP(privacy_moc_SRCS ${privacy_H})

QT4_WRAP_UI(privacy_UIS_H ${privacy_UIS})

MY_AUTOMOC(privacy_SRCS)


