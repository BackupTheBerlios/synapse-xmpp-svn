#SET(CDIR "plugins/generic/chess")
SET(CDIR ".")
include_directories(${CDIR})

set(p_chess_SRCS
    ${CDIR}/gameboard.cpp
    ${CDIR}/gamesocket.cpp
    ${CDIR}/mainwindow.cpp
    ${CDIR}/chessplugin.cpp
)

set(p_chess_H
    ../../../psiplugin.h
    ${CDIR}/gameboard.h
    ${CDIR}/gamesocket.h
    ${CDIR}/mainwindow.h
)

MY_AUTOMOC(p_chess_SRCS)

QT4_WRAP_CPP(p_chess_moc ${p_chess_H})

ADD_LIBRARY(plugin_chess SHARED ${p_chess_SRCS} ${p_chess_moc} ${p_chess_H})

TARGET_LINK_LIBRARIES(plugin_chess
    ${QT_QT3SUPPORT_LIBRARY}
)

INSTALL(TARGETS plugin_chess DESTINATION "${CMAKE_INSTALL_PREFIX}/share/synapse-im/plugins" )