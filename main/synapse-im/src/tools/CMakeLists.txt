include(tools/trayicon/CMakeLists.txt)
include(tools/zip/CMakeLists.txt)
include(tools/iconset/CMakeLists.txt)
include(tools/idle/CMakeLists.txt)
include(tools/systemwatch/CMakeLists.txt)
include(tools/optionstree/CMakeLists.txt)
include(tools/globalshortcut/CMakeLists.txt)
include(tools/spellchecker/CMakeLists.txt)
include(tools/advwidget/CMakeLists.txt)
include(tools/contactlist/CMakeLists.txt)
include(tools/grepshortcutkeydlg/CMakeLists.txt)
include(tools/tunecontroller/CMakeLists.txt)

set(tools_SRCS 
	${trayicon_SRCS} 
	${zip_SRCS}
	${iconset_SRCS}
	${idle_SRCS}
	${systemwatch_SRCS}
	${optionstree_SRCS}
	${globalshortcut_SRCS}
	${spellchecker_SRCS}
	${advwidget_SRCS}
	${contactlist_SRCS}
	${grepshortcutkeydlg_SRCS}
	${tunecontroller_SRCS}
)

set(tools_UIS
	${trayicon_UIS} 
	${zip_UIS}
	${iconset_UIS}
	${idle_UIS}
	${systemwatch_UIS}
	${optionstree_UIS}
	${globalshortcut_UIS}
	${spellchecker_UIS}
	${advwidget_UIS}
	${contactlist_UIS}
	${grepshortcutkeydlg_UIS}
	${tunecontroller_UIS}
)

set(tools_H
	${trayicon_H} 
	${zip_H}
	${iconset_H}
	${idle_H}
	${systemwatch_H}
	${optionstree_H}
	${globalshortcut_H}
	${spellchecker_H}
	${advwidget_H}
	${contactlist_H}
	${grepshortcutkeydlg_H}
	${tunecontroller_H}
)

MY_AUTOMOC(tools_SRCS)
QT4_WRAP_CPP(tools_moc_SRCS ${tools_H})
QT4_WRAP_UI(tools_UIS_H ${tools_UIS})

set(tools_SRCS ${tools_SRCS} ${tools_moc_SRCS})

