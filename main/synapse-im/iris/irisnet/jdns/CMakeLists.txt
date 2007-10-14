include_directories(irisnet/jdns)
set(DIR irisnet/jdns)

set(jdns_SRCS ${jdns_SRCS}
	${DIR}/jdns_util.c
	${DIR}/jdns_packet.c
	${DIR}/jdns_mdnsd.c
	${DIR}/jdns_sys.c
	${DIR}/jdns.c
	${DIR}/qjdns_sock.cpp
	${DIR}/qjdns.cpp
)

set(jdns_H ${jdns_H}
	${DIR}/qjdns_sock.h
	${DIR}/qjdns.h
)

qt4_generate_moc(irisnet/netinterface_unix.cpp ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/iris.dir/irisnet/netinterface_unix.moc)
qt4_generate_moc(irisnet/jdnsshared.cpp ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/iris.dir/irisnet/jdnsshared.moc)
