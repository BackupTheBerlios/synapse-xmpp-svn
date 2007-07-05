#project(synapse-im)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_CXX_FLAGS "-Os -pipe -w -s")

set(CMAKE_INSTALL_PREFIX "/usr")

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
)

INSTALL(DIRECTORY ${data_SRCS}  DESTINATION share/synapse-im
	PATTERN "SVN" EXCLUDE)