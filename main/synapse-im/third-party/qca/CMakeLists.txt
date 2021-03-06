PROJECT(QCA)

OPTION(BUILD_TESTS "Create test" ON)

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Directory where lib will install")

set(PKGCONFIG_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig/" CACHE STRING "Base directory for pkgconfig files")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" )

#find_package(Qt4 REQUIRED)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

if (CMAKE_COMPILER_IS_GNUCXX)
   if (CMAKE_SYSTEM_NAME MATCHES Linux)
     add_definitions (-D_BSD_SOURCE)
     set ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-long-long -ansi -Wundef -Wcast-align -Werror-implicit-function-declaration -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-common")
     set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -fno-check-new -fno-common")
   endif (CMAKE_SYSTEM_NAME MATCHES Linux)
endif (CMAKE_COMPILER_IS_GNUCXX)

add_definitions (${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(include/QtCrypto/ ${QT_INCLUDES})

# Always include srcdir and builddir in include path
# This saves typing ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY} in about every subdir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# put the include dirs which are in the source or build tree
# before all other include dirs, so the headers in the sources
# are prefered over the already installed ones
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

SET(qca_INCLUDEDIR "${CMAKE_CURRENT_SOURCE_DIR}/include" )
SET(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib${LIB_SUFFIX}" )

set(qca_PLUGINSDIR "${QT_PLUGINS_DIR}/crypto/")
#MESSAGE(STATUS "Plugins will be installed to " ${qca_PLUGINSDIR})

include(QcaMacro)

#MESSAGE(STATUS "Checking for certstore..")
# fixme add OR mac
IF( WIN32 )
# USE BUILTIN
ELSE ( WIN32 )
  IF ( ENV{QC_CERTSTORE_PATH} )
    IF(EXISTS ENV{QC_CERTSTORE_PATH})
      SET( qca_CERTSTORE $ENV{QC_CERTSTORE_PATH})
    ELSE(EXISTS ENV{QC_CERTSTORE_PATH})
		  # path to try
    ENDIF(EXISTS ENV{QC_CERTSTORE_PATH})
  ELSE( ENV{QC_CERTSTORE_PATH} )
    SET( toTry
      "/etc/ssl/certs/ca-certificates.crt"
      "/usr/share/ssl/cert.pem"
      "/usr/share/ssl/certs/ca-bundle.crt"
      "/etc/pki/tls/cert.pem"
      "/usr/share/curl/curl-ca-bundle.crt"
      "/etc/ssl/certs/ca-certificates.crt"
    )
    FOREACH (_current_try ${toTry})
      IF(EXISTS ${_current_try})
        SET( qca_CERTSTORE ${_current_try})
      ENDIF(EXISTS ${_current_try})
    ENDFOREACH (_current_try)
  ENDIF( ENV{QC_CERTSTORE_PATH} )
ENDIF(WIN32)

#IF (qca_CERTSTORE)
#  MESSAGE(STATUS "Found system certstore")
IF (!qca_CERTSTORE)
  MESSAGE(STATUS "Using built in certstore.")
  SET( qca_CERTSTORE "${CMAKE_CURRENT_SOURCE_DIR}/certs/rootcerts.pem")
  # note that INSTALL_FILES targets are relative to the current installation prefix...
  INSTALL_FILES( "/certs" FILES "${CMAKE_CURRENT_SOURCE_DIR}/certs/rootcerts.pem" )
ENDIF (!qca_CERTSTORE)
ADD_DEFINITIONS( -DQCA_SYSTEMSTORE_PATH="\\\"${qca_CERTSTORE}\\\"" )
  MESSAGE(STATUS "certstore path: " ${qca_CERTSTORE})


SET( private_HEADERS ${qca_INCLUDEDIR}/QtCrypto/qca_plugin.h ${qca_INCLUDEDIR}/QtCrypto/qca_systemstore.h )

SET( public_HEADERS 
  ${qca_INCLUDEDIR}/QtCrypto/qca.h
  ${qca_INCLUDEDIR}/QtCrypto/qcaprovider.h
  ${qca_INCLUDEDIR}/QtCrypto/QtCrypto
  ${qca_INCLUDEDIR}/QtCrypto/qca_export.h
	${qca_INCLUDEDIR}/QtCrypto/qca_support.h
	${qca_INCLUDEDIR}/QtCrypto/qca_tools.h
	${qca_INCLUDEDIR}/QtCrypto/qca_core.h
	${qca_INCLUDEDIR}/QtCrypto/qca_textfilter.h
	${qca_INCLUDEDIR}/QtCrypto/qca_basic.h
	${qca_INCLUDEDIR}/QtCrypto/qca_publickey.h
	${qca_INCLUDEDIR}/QtCrypto/qca_cert.h
	${qca_INCLUDEDIR}/QtCrypto/qca_keystore.h
	${qca_INCLUDEDIR}/QtCrypto/qca_securelayer.h
	${qca_INCLUDEDIR}/QtCrypto/qca_securemessage.h
	${qca_INCLUDEDIR}/QtCrypto/qpipe.h )

SET( qca_HEADERS ${private_HEADERS} ${public_HEADERS} )

#install public headers only
INSTALL(FILES ${public_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtCrypto")

INCLUDE_DIRECTORIES(${QT_QTCORE_INCLUDE_DIR} "${qca_INCLUDEDIR}/QtCrypto")

if(NOT WIN32)
  CONFIGURE_FILE("qca.pc.cmake" "${CMAKE_BINARY_DIR}/qca.pc" @ONLY)
  INSTALL(FILES "${CMAKE_BINARY_DIR}/qca.pc" DESTINATION ${PKGCONFIG_INSTALL_PREFIX})
endif(NOT WIN32)

CONFIGURE_FILE("crypto.prf.cmake" "${CMAKE_BINARY_DIR}/crypto.prf" @ONLY)
INSTALL(FILES "${CMAKE_BINARY_DIR}/crypto.prf" DESTINATION ${CMAKE_INSTALL_PREFIX}/mkspecs/features)

add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(plugins)
#if(BUILD_TESTS)
#	ENABLE_TESTING()
#        add_subdirectory(unittest)
#        add_subdirectory(examples)
#endif(BUILD_TESTS)

