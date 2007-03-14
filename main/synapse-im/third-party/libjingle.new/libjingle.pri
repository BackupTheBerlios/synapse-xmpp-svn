include(../../conf.pri)
SUBDIR += $$PWD/libjingle
INCLUDEPATH += $$PWD/libjingle
LIBS += -L$$PWD/libjingle -ljingle_psi -ljingle_xmpp_psi
