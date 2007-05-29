include_directories(irisnet)

include(irisnet/jdns/CMakeLists.txt)

set(iris_SRCS ${iris_SRCS}
	irisnet/jdnsshared.cpp
	irisnet/irisnetplugin.cpp
	irisnet/irisnetglobal.cpp
	irisnet/processquit.cpp
	irisnet/netinterface.cpp
	irisnet/netnames.cpp
#	irisnet/netinterface_unix.cpp
	irisnet/netnames_jdns.cpp
)

qt4_generate_moc(irisnet/netinterface_unix.cpp ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/iris.dir/irisnet/netinterface_unix.moc)
qt4_generate_moc(irisnet/jdnsshared.cpp ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/iris.dir/irisnet/jdnsshared.moc)

include(../iris/irisnet/legacy/CMakeLists.txt)