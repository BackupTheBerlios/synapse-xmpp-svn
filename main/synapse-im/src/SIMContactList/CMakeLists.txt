include_directories(SIMContactList)

set(simcontactlist_noMOC_SRCS
	SIMContactList/Account.cpp
	SIMContactList/Contact.cpp
#	SIMContactList/PubSub.cpp
	SIMContactList/Meta.cpp
	SIMContactList/List.cpp
	SIMContactList/Group.cpp
	SIMContactList/Item.cpp
	SIMContactList/Model.cpp
	SIMContactList/View.cpp
	SIMContactList/Delegate.cpp
	SIMContactList/Name.cpp
)

set(simcontactlist_H
	SIMContactList/Account.h
	SIMContactList/Contact.h
#	SIMContactList/PubSub.h
	SIMContactList/Meta.h
	SIMContactList/List.h
	SIMContactList/Group.h
	SIMContactList/Item.h
	SIMContactList/Model.h
	SIMContactList/View.h
	SIMContactList/Delegate.h
	SIMContactList/Name.h
)

MY_AUTOMOC(simcontactlist_SRCS)

QT4_WRAP_CPP(simcontactlist_moc_SRCS ${simcontactlist_H})
#QT4_WRAP_UI(tools_UIS_H ${tools_UIS})

set(simcontactlist_SRCS ${simcontactlist_SRCS} ${simcontactlist_moc_SRCS} ${simcontactlist_noMOC_SRCS})