#project(synapse-im)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -g0 -pipe -w -fomit-frame-pointer")
#TEMPORARY

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "/usr/local")
endif(NOT CMAKE_INSTALL_PREFIX)

ADD_DEFINITIONS("-DUSE_XEP0091")
#ADD_DEFINITIONS("-DUSE_XEP0022")

if(uVme)
    ADD_DEFINITIONS("-DuVme")
endif(uVme)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

set(EXECUTABLE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE STRING "Directory where program will installed")

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")

set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib CACHE STRING "Directory where lib will installed" )

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

cmake_minimum_required(VERSION 2.4.0)

project(synapse-im)

find_package(Qt4 REQUIRED)

if(!QT_QTXML_FOUND)
    MESSAGE(FATAL_ERROR "Qt4 is builded without XML support!")
endif(!QT_QTXML_FOUND)
if(!QT_QTSQL_FOUND)
    MESSAGE(FATAL_ERROR "Qt4 is builded without SQL support!")
endif(!QT_QTSQL_FOUND)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    ADD_DEFINITIONS(-DHAVE_OPENSSL)
    MESSAGE(STATUS "Found OpenSSL")
endif(OPENSSL_FOUND)

find_package(Sasl2)

if(QT_QTDBUS_FOUND)
    MESSAGE(STATUS "Found DBus support in Qt4 -> enabling DBus support")
#    ADD_DEFINITIONS(-D)
else(QT_QTDBUS_FOUND)
    MESSAGE(STATUS "No DBus support in Qt4 -> disabling DBus support")
endif(QT_QTDBUS_FOUND)

if(QT_QTSVG_FOUND)
    MESSAGE(STATUS "Found SVG support in Qt4 -> enabling Whiteboard support")
else(QT_QTSVG_FOUND)
    MESSAGE(STATUS "No SVG support in Qt4 -> disabling Whiteboard support")
endif(QT_QTSVG_FOUND)

find_program(QCA_GPG_EXECUTABLE NAMES gpg)
if(QCA_GPG_EXECUTABLE)
    MESSAGE(STATUS "Found GPG -> enabling support for GPG")
endif(QCA_GPG_EXECUTABLE)

find_package(Speex)
if(SPEEX_FOUND)
    MESSAGE(STATUS "Found Speex -> enabling Jingle support")
else(SPEEX_FOUND)
    MESSAGE(STATUS "Speex not found -> disabling Jingle support")
endif(SPEEX_FOUND)

find_package(ASPELL)
if(ASPELL_FOUND)
    MESSAGE(STATUS "Found ASpell -> enabling Spellchecking support")
else(ASPELL_FOUND)
    MESSAGE(STATUS "ASpell not found -> disabling Spellchecking support")
endif(ASPELL_FOUND)
#TEMPORARY
if(!WIN32)
    ADD_DEFINITIONS("-DSYNAPSE_IM_DATADIR=\\\"${CMAKE_INSTALL_PREFIX}/share/synapse-im/\\\"")
endif(!WIN32)

#find_package(Avahi)
#if(QT_QTDBUS_FOUND)
#    if(AVAHI_FOUND)
#	MESSAGE(STATUS "Found Avahi -> enabling Link-Local support")
#    else(AVAHI_FOUND)
#	MESSAGE(STATUS "Avahi not found -> disabling Link-Local support")
#    endif(AVAHI_FOUND)
#endif(QT_QTDBUS_FOUND)

add_definitions(-DQT_NO_DEBUG)

add_subdirectory(third-party/qca)

if(SPEEX_FOUND)
add_subdirectory(third-party/jrtplib)
endif(SPEEX_FOUND)

add_subdirectory(iris)

add_subdirectory(src)

set(data_SRCS
    certs
    iconsets
    sound
    options
)
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

INSTALL(DIRECTORY ${data_SRCS}  DESTINATION share/synapse-im
	PATTERN "SVN" EXCLUDE)