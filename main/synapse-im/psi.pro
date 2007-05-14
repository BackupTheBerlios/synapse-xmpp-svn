TEMPLATE = subdirs

include(conf.pri)
windows:include(conf_windows.pri)

jingle {
	CONFIG += google_voice
	SUBDIRS += third-party/jrtplib
#	SUBDIRS += third-party/libjingle.new/libjingle
#	SUBDIRS += third-party/libjingle.new/libjingle/talk/xmpp
}

qca-static {
	SUBDIRS += third-party/qca
}

SUBDIRS += \
	src

unix {
	# unittest
	QMAKE_EXTRA_TARGETS += check
	check.commands += cd unittest && make check && cd ..
}

