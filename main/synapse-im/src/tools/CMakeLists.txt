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
	${advwidget_SRCS}
	${globalshortcut_SRCS}
	${grepshortcutkeydlg_SRCS}
	${zip_SRCS}
	${iconset_SRCS}
	${idle_SRCS}
	${systemwatch_SRCS}
	${optionstree_SRCS}
	${spellchecker_SRCS}
	${contactlist_SRCS}
	${tunecontroller_SRCS}
)
set(tools_noMOC_SRCS 
	${advwidget_noMOC_SRCS}
	${globalshortcut_noMOC_SRCS}
	${grepshortcutkeydlg_noMOC_SRCS}
	${zip_noMOC_SRCS}
	${iconset_noMOC_SRCS}
	${idle_noMOC_SRCS}
	${systemwatch_noMOC_SRCS}
	${optionstree_noMOC_SRCS}
	${spellchecker_noMOC_SRCS}
	${contactlist_noMOC_SRCS}
	${tunecontroller_noMOC_SRCS}
)

set(tools_UIS
	${advwidget_UIS}
	${globalshortcut_UIS}
	${grepshortcutkeydlg_UIS}
)
set(tools1_UIS
	${zip_UIS}
	${iconset_UIS}
	${idle_UIS}
	${systemwatch_UIS}
	${optionstree_UIS}
	${spellchecker_UIS}
	${contactlist_UIS}
	${tunecontroller_UIS}
)

set(tools_H
	${advwidget_H}
	${globalshortcut_H}
	${grepshortcutkeydlg_H}
	${zip_H}
	${iconset_H}
	${idle_H}
	${systemwatch_H}
	${optionstree_H}
	${spellchecker_H}
	${contactlist_H}
	${tunecontroller_H}
)

MY_AUTOMOC(tools_SRCS)
QT4_WRAP_CPP(tools_moc_SRCS ${tools_H})
QT4_WRAP_UI(tools_UIS_H ${tools_UIS})

set(tools_SRCS ${tools_SRCS} ${tools_moc_SRCS} ${tools_noMOC_SRCS})
