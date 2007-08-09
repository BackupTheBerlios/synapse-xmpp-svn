include_directories(SIMContactList)

set(simcontactlist_noMOC_SRCS
	SIMContactList/SIMContactListAccount.cpp
	SIMContactList/SIMContactListContact.cpp
	SIMContactList/SIMContactList.cpp
	SIMContactList/SIMContactListGroup.cpp
	SIMContactList/SIMContactListItem.cpp
	SIMContactList/SIMContactListModel.cpp
	SIMContactList/SIMContactListView.cpp
	SIMContactList/SIMContactDelegate.cpp
	SIMContactList/SIMContactName.cpp
)

set(simcontactlist_H
	SIMContactList/SIMContactListAccount.h
	SIMContactList/SIMContactListContact.h
	SIMContactList/SIMContactList.h
	SIMContactList/SIMContactListGroup.h
	SIMContactList/SIMContactListItem.h
	SIMContactList/SIMContactListModel.h
	SIMContactList/SIMContactListView.h
	SIMContactList/SIMContactDelegate.h
	SIMContactList/SIMContactName.h
)

MY_AUTOMOC(simcontactlist_SRCS)

QT4_WRAP_CPP(simcontactlist_moc_SRCS ${simcontactlist_H})
#QT4_WRAP_UI(tools_UIS_H ${tools_UIS})

set(simcontactlist_SRCS ${simcontactlist_SRCS} ${simcontactlist_moc_SRCS} ${simcontactlist_noMOC_SRCS})