include_directories(widgets)

set(widgets_SRCS
	widgets/busywidget.cpp
	widgets/fancylabel.cpp
	widgets/iconselect.cpp
	widgets/iconwidget.cpp 
	widgets/psitextview.cpp
	widgets/psirichlabel.cpp
	widgets/urllabel.cpp
	widgets/urlobject.cpp
	widgets/iconaction.cpp
	widgets/fancypopup.cpp
	widgets/psirichtext.cpp
	widgets/psitooltip.cpp
	widgets/psitabwidget.cpp
	widgets/psitabbar.cpp
)

set(widgets_H
	widgets/stretchwidget.h
	widgets/busywidget.h
	widgets/fancylabel.h
	widgets/iconselect.h
	widgets/iconsetselect.h
	widgets/iconsetdisplay.h
	widgets/iconwidget.h
	widgets/iconbutton.h
	widgets/psitextview.h
	widgets/psirichlabel.h
	widgets/iconaction.h
	widgets/fancypopup.h
	widgets/urllabel.h
	widgets/urlobject.h
	widgets/updatingcombobox.h
	widgets/iconlabel.h
	widgets/icontoolbutton.h
	widgets/fancypopuplist.h
	widgets/psirichtext.h
	widgets/psitooltip.h
	widgets/psitabwidget.h
	widgets/psitabbar.h
)

#qt4_automoc(widgets_SRCS)
MY_AUTOMOC(widgets_SRCS)
QT4_WRAP_CPP(widgets_moc_SRCS ${widgets_H})

set(widgets_SRCS ${widgets_SRCS} ${widgets_moc_SRCS})
