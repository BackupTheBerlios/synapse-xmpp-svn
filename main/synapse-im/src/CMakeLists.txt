
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
ADD_DEFINITIONS(-DQT_SHARED)

INCLUDE_DIRECTORIES(
    ${QT_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}
    ${QT_QTSQL_INCLUDE_DIR}
    ${QT_QTXML_INCLUDE_DIR}
    ${QT_QT3SUPPORT_INCLUDE_DIR}
    ${QT_QTDBUS_INCLUDE_DIR}
    ${QT_QTSVG_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ../third-party/qca/include/QtCrypto
    ../cutestuff/legacy
    ../cutestuff/network
    ../cutestuff/util
    jingle
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/s-im.dir
)

add_definitions(-DUSE_PEP)

if(PLUGINS)
    ADD_DEFINITIONS("-DPSI_PLUGINS")
endif(PLUGINS)

include(SIMContactList/CMakeLists.txt)

include(tools/CMakeLists.txt)

include(widgets/CMakeLists.txt)

include(options/CMakeLists.txt)

include(../iris/CMakeLinks.txt)

include(privacy/CMakeLists.txt)

include(archiving/CMakeLists.txt)

include(capabilities/CMakeLists.txt)
include(irisprotocol/CMakeLists.txt)
include(utilities/CMakeLists.txt)
include(tabs/CMakeLists.txt)
include(protocol/CMakeLists.txt)

if(NOT WIN32)
add_definitions(-DSYNAPSE_IM_DATADIR=\\\"${CMAKE_INSTALL_PREFIX}/share/synapse-im\\\")
endif(NOT WIN32)

#DBus
if(QT_QTDBUS_FOUND)
#//???????????
#ADD_DEFINITIONS(-DHAVE_DBUS)
ADD_DEFINITIONS(-DUSE_DBUS)
set(rpc_SRCS
	dbus.cpp
	activeprofiles_dbus.cpp
)
set(rpc_H
	dbus.h
)
#if(AVAHI_FOUND)
#endif(AVAHI_FOUND)
else (QT_QTDBUS_FOUND)
	if(WIN32)
		set(rpc_SRCS
			activeprofiles_win.cpp
		)
	endif(WIN32)
endif(QT_QTDBUS_FOUND)

ADD_DEFINITIONS(-DLINKLOCAL)

#Whiteboarding
if(QT_QTSVG_FOUND)
ADD_DEFINITIONS(-DWHITEBOARDING)
set(wb_noMOC_SRCS
    sxe/sxemanager.cpp
    sxe/sxesession.cpp
    sxe/sxeedit.cpp
    sxe/sxenewedit.cpp
    sxe/sxeremoveedit.cpp
    sxe/sxerecordedit.cpp
    sxe/sxerecord.cpp
    whiteboarding/wbmanager.cpp
    whiteboarding/wbdlg.cpp
    whiteboarding/wbwidget.cpp
    whiteboarding/wbscene.cpp
    whiteboarding/wbitem.cpp
    whiteboarding/wbnewitem.cpp
    whiteboarding/wbnewpath.cpp
    whiteboarding/wbnewimage.cpp
)
set(wb_H
    sxe/sxemanager.h
    sxe/sxesession.h
    sxe/sxeedit.h
    sxe/sxenewedit.h
    sxe/sxeremoveedit.h
    sxe/sxerecordedit.h
    sxe/sxerecord.h
    whiteboarding/wbmanager.h
    whiteboarding/wbdlg.h
    whiteboarding/wbwidget.h
    whiteboarding/wbscene.h
    whiteboarding/wbitem.h
    whiteboarding/wbnewitem.h
    whiteboarding/wbnewpath.h
    whiteboarding/wbnewimage.h
)
endif(QT_QTSVG_FOUND)

# JINGLE
if(SPEEX_FOUND)
include(jingle/CMakeLists.txt)
ADD_DEFINITIONS(-DHAVE_JINGLE)
ADD_DEFINITIONS(-DPOSIX)
ADD_DEFINITIONS(-Djingle)
ADD_DEFINITIONS(-Djingle_voice)
endif(SPEEX_FOUND)

#Main files
set(synapse-im_noMOC_SRCS
	aboutdlg.cpp
	accountadddlg.cpp
	accountlabel.cpp
	accountmodifydlg.cpp
	accountregdlg.cpp
	accountscombobox.cpp
	adduserdlg.cpp
	ahcexecutetask.cpp
	ahcformdlg.cpp
	ahcommand.cpp
	ahcommanddlg.cpp
	ahcommandserver.cpp
	alertable.cpp
	antievil.cpp
	applicationinfo.cpp
	bookmarkmanager.cpp
	certutil.cpp
	changepwdlg.cpp
	chateditproxy.cpp
	chatsplitter.cpp
	common.cpp
	conferencebookmark.cpp
	desktoputil.cpp
	entitytimetask.cpp
	gcuserview.cpp
	geolocation.cpp
	geolocationdlg.cpp
	historydb.cpp
	historydlg.cpp
	infodlg.cpp
	jidutil.cpp
	lastactivitytask.cpp
	main.cpp
	mainwin.cpp
	miniclient.cpp
	mood.cpp
	moodcatalog.cpp
	mooddlg.cpp
	msgmle.cpp
	mucaffiliationsmodel.cpp
	mucaffiliationsproxymodel.cpp
	mucaffiliationsview.cpp
	mucconfigdlg.cpp
	mucjoindlg.cpp
	mucmanager.cpp
	passphrasedlg.cpp
	pepmanager.cpp
	pgpkeydlg.cpp
	pgptransaction.cpp
	pgputil.cpp
	physicallocation.cpp
	pongserver.cpp
	profiledlg.cpp
	proxy.cpp
	psi_profiles.cpp
	psiapplication.cpp
	psicontactlist.cpp
	psievent.cpp
	psioptions.cpp
	psitoolbar.cpp
	psitrayicon.cpp
	pubsubsubscription.cpp
	rc.cpp
	resourcemenu.cpp
	rosteritemexchangetask.cpp
	rtparse.cpp
	serverinfomanager.cpp
	serverlistquerier.cpp
	shortcutmanager.cpp
	showtextdlg.cpp
	sslcertdlg.cpp
	statusdlg.cpp
	statuscombobox.cpp
	statuspreset.cpp
	systeminfo.cpp
	psichatdlg.cpp
	chattoolbox.cpp
	textutil.cpp
	timeserver.cpp
	tipdlg.cpp
	translationmanager.cpp
	urlbookmark.cpp
	userlist.cpp
	varlist.cpp
	vcardfactory.cpp
	voicecalldlg.cpp
	bookmarkmanagedlg.cpp
	${wb_noMOC_SRCS}
	xdata_widget.cpp
	xmlconsole.cpp
)

set(synapse-im_SRCS
	${rpc_SRCS}
	${wb_SRCS}
	pixmaputil.cpp
	mainwin_p.cpp
	accountmanagedlg.cpp
	eventdlg.cpp
	chatdlg.cpp
	searchdlg.cpp
	groupchatdlg.cpp
	registrationdlg.cpp
	ahcservermanager.cpp
	discodlg.cpp
	alerticon.cpp
	psipopup.cpp
	activeprofiles.cpp
	psioptionseditor.cpp
	filetransdlg.cpp
	avatars.cpp
	actionlist.cpp
	psiactionlist.cpp
	httpauthmanager.cpp
	psiiconset.cpp
	psicon.cpp
	psiaccount.cpp
)

set(synapse-im_H
	${rpc_H}
	${wb_H}
	aboutdlg.h
	accountadddlg.h
	accountlabel.h
	accountmanagedlg.h
	accountmodifydlg.h
	accountregdlg.h
	accountscombobox.h
	activeprofiles.h
	adduserdlg.h
	ahcformdlg.h
	ahcommanddlg.h
	alertable.h
	alerticon.h
	antievil.h
	avatars.h
	bookmarkmanager.h
	changepwdlg.h
	chatdlg.h
	chateditproxy.h
	chatsplitter.h
	discodlg.h
	eventdlg.h
	filetransdlg.h
	gcuserview.h
	groupchatdlg.h
	mucaffiliationsmodel.h
	mucaffiliationsproxymodel.h
	mucaffiliationsview.h
	mucconfigdlg.h
	mucjoindlg.h
	mucmanager.h
	historydb.h
	historydlg.h
	httpauthmanager.h
	infodlg.h
	main.h
	mainwin.h
	mainwin_p.h
	miniclient.h
	msgmle.h
	mooddlg.h
	pepmanager.h
	passphrasedlg.h
	pgpkeydlg.h
	pgptransaction.h
	pgputil.h
	profiledlg.h
	proxy.h
	psiaccount.h
	psiactions.h
	psiapplication.h
	psichatdlg.h
	chattoolbox.h
	psicon.h
	psicontactlist.h
	psiiconset.h
	psievent.h
	psioptions.h
	psioptionseditor.h
	psipopup.h
	pongserver.h
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
	timeserver.h
	tipdlg.h
	translationmanager.h
	vcardfactory.h
	voicecalldlg.h
	voicecaller.h
	xdata_widget.h
	xmlconsole.h
	bookmarkmanagedlg.h
	statuscombobox.h
	geolocationdlg.h
)

set(res_RC
    ../psi.qrc
    ../iconsets.qrc
)

if(PLUGINS)
#    ADD_DEFINITIONS("-DPSI_PLUGINS")
    set(synapse-im_SRCS ${synapse-im_SRCS}
	pluginmanager.cpp
	)
    set(synapse-im_H ${synapse-im_H}
	pluginmanager.h
	)
    ADD_SUBDIRECTORY(plugins)
endif(PLUGINS)

MY_AUTOMOC(synapse-im_SRCS)

QT4_WRAP_CPP(synapse-im_moc_SRCS ${synapse-im_H})

QT4_ADD_RESOURCES(res_SRCS ${res_RC})

set(res_SRCS ${res_SRCS} ../options/default.xml)


set(synapse-im_UIS
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
	bookmarkmanage.ui
	disco.ui
	tip.ui
	geolocation.ui
	filetrans.ui
	mood.ui
	voicecall.ui
	chatdlg.ui
	chat/chatcontact.ui
	groupchatdlg.ui
	optioneditor.ui
	options/ui_isdetails.ui
	options/ui_options.ui
	options/ui_positiontoolbar.ui
)

QT4_WRAP_UI(synapse-im_UIS_H ${synapse-im_UIS})

if(WIN32)
set(CMAKE_EXE_LINKER_FLAGS "-mwindows")
endif(WIN32)

if(SPEEX_FOUND)
set(synapse-im_SRCS ${synapse-im_SRCS} ${jingle_SRCS})
endif(SPEEX_FOUND)

add_executable(synapse-im
	${synapse-im_SRCS}
	${synapse-im_noMOC_SRCS}
        ${tools_SRCS}
	${widgets_SRCS}
	${options_SRCS}
	${options_noMOC_SRCS}
	${simcontactlist_SRCS}
	${privacy_SRCS}
	${privacy_moc_SRCS}
	${privacy_UIS_H}
	${archiving_SRCS}
	${archiving_moc_SRCS}
	${archiving_UIS_H}
	${capabilities_SRCS}
	${capabilities_moc_SRCS}
	${utilities_SRCS}
	${utilities_moc_SRCS}
	${tabs_SRCS}
	${tabs_moc_SRCS}
	${irisprotocol_SRCS}
	${irisprotocol_moc_SRCS}
	${protocol_SRCS}
	${protocol_moc_SRCS}
#	${jingle_SRCS}
	${synapse-im_UIS_H}
	${options_UIS_H}
	${tools_UIS_H}
	${synapse-im_moc_SRCS}
	${res_SRCS}
)

LINK_DIRECTORIES( ../lib )

TARGET_LINK_LIBRARIES( synapse-im
    iris
    qca
    ${QT_LIBRARIES}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTSQL_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QT3SUPPORT_LIBRARY}
)
if(WIN32)
TARGET_LINK_LIBRARIES( synapse-im
	gdi32
	User32
)
endif(WIN32)

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
#    jingle
    jrtp
    speex
)
endif(SPEEX_FOUND)

if(ASPELL_FOUND)
TARGET_LINK_LIBRARIES( synapse-im
    ${ASPELL_LIBRARIES}
)
endif(ASPELL_FOUND)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(${QT_NO_DEBUG})

INSTALL(TARGETS synapse-im DESTINATION ${EXECUTABLE_INSTALL_DIR})


