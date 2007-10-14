#SET(CDIR "plugins/generic/otr")
SET(CDIR ".")
include_directories(${CDIR})

set(p_otr_SRCS
    ${CDIR}/psi-otr.cpp
    ${CDIR}/otrconnection.cpp
    ${CDIR}/configdlg.cpp
    ${CDIR}/fingerprintwidget.cpp
    ${CDIR}/privkeytablemodel.cpp
    ${CDIR}/privkeywidget.cpp
    ${CDIR}/configotrwidget.cpp
)

set(p_otr_H
    ../../../psiplugin.h
    ${CDIR}/psi-otr.h
    ${CDIR}/otrconnection.h
    ${CDIR}/configdlg.h
    ${CDIR}/fingerprintwidget.h
    ${CDIR}/fprint.h
    ${CDIR}/privkeytablemodel.h
    ${CDIR}/privkeywidget.h
    ${CDIR}/configotrwidget.h
)

MY_AUTOMOC(p_otr_SRCS)

QT4_WRAP_CPP(p_otr_moc ${p_otr_H})

ADD_LIBRARY(plugin_otr SHARED ${p_otr_SRCS} ${p_otr_moc} ${p_otr_H})

TARGET_LINK_LIBRARIES(plugin_otr
    ${QT_QTXML_LIBRARY}
    otr
)