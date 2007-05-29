
MACRO(MY_AUTOMOC _srcsList)
  QT4_GET_MOC_INC_DIRS(_moc_INCS)
  FOREACH (_current_FILE ${${_srcsList}})
    GET_FILENAME_COMPONENT(_abs_FILE ${_current_FILE} ABSOLUTE)
    GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)
    SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)
    SET(extra_moc_argument)
    if(WIN32)
	SET(extra_moc_argument -DWIN32)
    endif(WIN32)
    ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
                       COMMAND ${QT_MOC_EXECUTABLE}
		       ARGS ${extra_moc_argument} ${_moc_INCS} -o ${_moc} ${_abs_FILE}
		       DEPENDS ${_current_FILE}
    )
    LIST(APPEND ${_srcsList} ${_moc})
  ENDFOREACH (_current_FILE)
ENDMACRO(MY_AUTOMOC)

include(${QT_USE_FILE})
ADD_DEFINITIONS(-DQT3_SUPPORT)

INCLUDE_DIRECTORIES(
    ${QT_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}
    ${QT_QTSQL_INCLUDE_DIR}
    ${QT_QTXML_INCLUDE_DIR}
    ${QT_QT3SUPPORT_INCLUDE_DIR}
    ${QT_QTDBUS_INCLUDE_DIR}

    ${CMAKE_CURRENT_BINARY_DIR}
    ../third-party/qca/include/QtCrypto
    ../cutestuff/legacy
    ../cutestuff/network
    ../cutestuff/util
    jingle
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/synapse-im.dir
)

include(tools/CMakeLists.txt)

add_definitions(-DUSE_PEP)

include(options/CMakeLists.txt)

include(widgets/CMakeLists.txt)

include(../iris/CMakeLinks.txt)

#DBus
if(QT_QTDBUS_FOUND)
ADD_DEFINITIONS(-DHAVE_DBUS)
set(dbus_SRCS
	dbus.cpp
)
set(dbus_H
	dbus.h
)
endif(QT_QTDBUS_FOUND)

# JINGLE
if(SPEEX_FOUND)
include(jingle/CMakeLists.txt)
ADD_DEFINITIONS(-DHAVE_JINGLE)
ADD_DEFINITIONS(-DPOSIX)
ADD_DEFINITIONS(-Djingle)
ADD_DEFINITIONS(-Djingle_voice)
endif(SPEEX_FOUND)

set(synapse-im_SRCS
	${dbus_SRCS}
	main.cpp
	varlist.cpp
	jidutil.cpp
	showtextdlg.cpp
	psi_profiles.cpp
	profiledlg.cpp
	aboutdlg.cpp
	desktoputil.cpp
	textutil.cpp
	pixmaputil.cpp
	accountscombobox.cpp
	accountlabel.cpp
	psievent.cpp
	xmlconsole.cpp
	contactview.cpp
	psiiconset.cpp
	applicationinfo.cpp
	pgpkeydlg.cpp
	pgputil.cpp
	pgptransaction.cpp
	serverinfomanager.cpp
	userlist.cpp
	mainwin.cpp
	mainwin_p.cpp
	psitrayicon.cpp
	rtparse.cpp
	systeminfo.cpp
	common.cpp
	proxy.cpp
	miniclient.cpp
	accountmanagedlg.cpp
	accountadddlg.cpp
	accountregdlg.cpp
	accountmodifydlg.cpp
	changepwdlg.cpp
	msgmle.cpp
	statusdlg.cpp
	eventdlg.cpp
	chatdlg.cpp
	chatsplitter.cpp
	chateditproxy.cpp
	tipdlg.cpp
	tabdlg.cpp
	adduserdlg.cpp
	groupchatdlg.cpp
	gcuserview.cpp
	infodlg.cpp
	translationmanager.cpp
	certutil.cpp
	historydb.cpp
	historydlg.cpp
	searchdlg.cpp
	registrationdlg.cpp
	psitoolbar.cpp
	passphrasedlg.cpp
	vcardfactory.cpp
	sslcertdlg.cpp
#	qwextend.cpp
	discodlg.cpp
	capsspec.cpp
	capsregistry.cpp
	capsmanager.cpp
	alerticon.cpp
	alertable.cpp
	psipopup.cpp
	psiapplication.cpp
	filetransdlg.cpp
	avatars.cpp
	actionlist.cpp
	psiactionlist.cpp
	xdata_widget.cpp
	lastactivitytask.cpp
	statuspreset.cpp
	mucmanager.cpp
	mucjoindlg.cpp
	mucconfigdlg.cpp
	mucaffiliationsmodel.cpp
	mucaffiliationsproxymodel.cpp
	mucaffiliationsview.cpp
	rosteritemexchangetask.cpp
	mood.cpp
	moodcatalog.cpp
	mooddlg.cpp
	geolocation.cpp
	physicallocation.cpp
	urlbookmark.cpp
	conferencebookmark.cpp
	bookmarkmanager.cpp
	pepmanager.cpp
	pubsubsubscription.cpp
	rc.cpp
	gmail_notify.cpp
	httpauthmanager.cpp
 	privacylistitem.cpp
 	privacylist.cpp
 	privacylistmodel.cpp
 	privacylistblockedmodel.cpp
 	privacymanager.cpp
 	privacydlg.cpp
 	privacyruledlg.cpp
	garchive_otr.cpp
	garchive.cpp
	ahcommand.cpp
	ahcommandserver.cpp
 	ahcommanddlg.cpp
	ahcformdlg.cpp
	ahcexecutetask.cpp
 	ahcservermanager.cpp
	serverlistquerier.cpp
	psioptions.cpp
	voicecalldlg.cpp
	resourcemenu.cpp
	shortcutmanager.cpp
	psicontactlist.cpp
	psicon.cpp
	psiaccount.cpp
)

set(synapse-im_H
	${dbus_H}
	aboutdlg.h
	accountadddlg.h
	accountlabel.h
	accountmanagedlg.h
	accountmodifydlg.h
	accountregdlg.h
	accountscombobox.h
	adduserdlg.h
	ahcformdlg.h
	ahcommanddlg.h
	alertable.h
	alerticon.h
	avatars.h
	bookmarkmanager.h
	capsmanager.h
	capsregistry.h
	changepwdlg.h
	chatdlg.h
	chateditproxy.h
	chatsplitter.h
	contactview.h
	discodlg.h
	eventdlg.h
	filetransdlg.h
	garchive.h
	gcuserview.h
	gmail_notify.h
	groupchatdlg.h
	historydb.h
	historydlg.h
	httpauthmanager.h
	infodlg.h
	main.h
	mainwin.h
	mainwin_p.h
	miniclient.h
	mooddlg.h
	msgmle.h
	mucaffiliationsmodel.h
	mucaffiliationsproxymodel.h
	mucaffiliationsview.h
	mucconfigdlg.h
	mucjoindlg.h
	mucmanager.h
	passphrasedlg.h
	pepmanager.h
	pgpkeydlg.h
	pgptransaction.h
	pgputil.h
	privacydlg.h
	privacymanager.h
	privacyruledlg.h
	profiledlg.h
	proxy.h
	psiaccount.h
	psiapplication.h
	psicon.h
	psicontactlist.h
	psievent.h
	psioptions.h
	psipopup.h
	psitoolbar.h
	psitrayicon.h
	registrationdlg.h
	resourcemenu.h
	rosteritemexchangetask.h
	searchdlg.h
	serverinfomanager.h
	serverlistquerier.h
	showtextdlg.h
	sslcertdlg.h
	statusdlg.h
	tasklist.h
	tabdlg.h
	tipdlg.h
	translationmanager.h
	vcardfactory.h
	voicecalldlg.h
	voicecaller.h
	xdata_widget.h
	xmlconsole.h
)

set(res_RC
    ../psi.qrc
    ../iconsets.qrc
)

MY_AUTOMOC(synapse-im_SRCS)
#MY_AUTOMOC(tools_SRCS)
#MY_AUTOMOC(options_SRCS)
#MY_AUTOMOC(widgets_SRCS)
#MY_AUTOMOC(jingle_SRCS)

QT4_WRAP_CPP(synapse-im_moc_SRCS ${synapse-im_H})
#QT4_WRAP_CPP(tools_moc_SRCS ${tools_H})
#QT4_WRAP_CPP(options_moc_SRCS ${options_H})
#QT4_WRAP_CPP(widgets_moc_SRCS ${widgets_H})
#QT4_WRAP_CPP(jingle_moc_SRCS ${jingle_H})

QT4_ADD_RESOURCES(res_SRCS ${res_RC})

set(synapse-im_UIS
#	${tools_UIS}
#	${options_UIS}
	profileopen.ui
	profilemanage.ui
	profilenew.ui
	proxy.ui
	pgpkey.ui
	accountmanage.ui
	accountadd.ui
	accountreg.ui
	accountremove.ui
	accountmodify.ui
	historydlgui.ui
	changepw.ui
	addurl.ui
	adduser.ui
	mucjoin.ui
	info.ui
	search.ui
	about.ui
	passphrase.ui
	sslcert.ui
	mucconfig.ui
	xmlconsole.ui
	disco.ui
	tip.ui
	filetrans.ui
	privacy.ui
	privacyrule.ui
	mood.ui
	voicecall.ui
	chatdlg.ui
	chat/chatcontact.ui
	groupchatdlg.ui
	options/ui_isdetails.ui
	options/ui_options.ui
	options/ui_positiontoolbar.ui
)

QT4_WRAP_UI(synapse-im_UIS_H ${synapse-im_UIS})

add_executable(synapse-im 
	${synapse-im_SRCS}
	${tools_SRCS}
	${widgets_SRCS}
	${options_SRCS}
#	${iris_S}
	${jingle_SRCS}
	${synapse-im_UIS_H}
	${options_UIS_H}
	${tools_UIS_H}
#	${iris_moc_SRCS}
#	${tools_moc_SRCS}
#	${options_moc_SRCS}
	${synapse-im_moc_SRCS}
#	${widgets_moc_SRCS}
#	${jingle_moc_SRCS}
	${res_SRCS}
)

LINK_DIRECTORIES( ../lib )

TARGET_LINK_LIBRARIES( synapse-im
    ${QT_LIBRARIES}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTSQL_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QT3SUPPORT_LIBRARY}
    iris
    qca
)

if(QT_QTDBUS_FOUND)
TARGET_LINK_LIBRARIES( synapse-im
    ${QT_QTDBUS_LIBRARY}
)
endif(QT_QTDBUS_FOUND)

if(QT_QTSVG_FOUND)
TARGET_LINK_LIBRARIES( synapse-im
    ${QT_QTSVG_LIBRARY}
)
endif(QT_QTSVG_FOUND)

if(SPEEX_FOUND)
TARGET_LINK_LIBRARIES( synapse-im
    jrtp
    speex
)
endif(SPEEX_FOUND)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(${QT_NO_DEBUG})
ADD_DEFINITIONS(-DQT_SHARED)

INSTALL(TARGETS synapse-im DESTINATION ${EXECUTABLE_INSTALL_DIR})