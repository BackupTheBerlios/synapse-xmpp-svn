INCLUDE(CheckTypeSize)

#FIND_PACKAGE(OpenSSL)
#FIND_PACKAGE(Sasl2)
#FIND_PACKAGE(Nss)
#FIND_PACKAGE(Pkcs11Helper)
#FIND_PACKAGE(LibGcrypt)
#FIND_PACKAGE(Botan)

ADD_SUBDIRECTORY(qca-logger)
ADD_SUBDIRECTORY(qca-softstore)

IF (OPENSSL_FOUND)
  ADD_SUBDIRECTORY(qca-ossl)
#  MESSAGE(STATUS "OpenSSL plugin enabled")
ENDIF(OPENSSL_FOUND)

#IF (OPENSSL_FOUND AND PKCS11H_FOUND)
#  ADD_SUBDIRECTORY(qca-pkcs11)
#  MESSAGE(STATUS "PKCS#11 plugin enabled")
#ENDIF (OPENSSL_FOUND AND PKCS11H_FOUND)

FIND_PROGRAM(QCA_GPG_EXECUTABLE NAMES gpg)

IF ( QCA_GPG_EXECUTABLE )
#  MESSAGE(STATUS "GnuPG plugin enabled")
  ADD_SUBDIRECTORY(qca-gnupg)
ENDIF ( QCA_GPG_EXECUTABLE )

#IF(SASL2_FOUND)
#  MESSAGE(STATUS "SASL2 plugin enabled")
#  ADD_SUBDIRECTORY(qca-cyrus-sasl)
#ENDIF(SASL2_FOUND)

IF(BOTAN_FOUND)
#  MESSAGE(STATUS "Botan plugin enabled")
  ADD_SUBDIRECTORY(qca-botan)
ENDIF(BOTAN_FOUND)	

#IF(LIBGCRYPT_FOUND)
#	SET(remember_includes ${CMAKE_EXTRA_INCLUDE_FILES})
#	SET(remember_defines ${CMAKE_REQUIRED_FLAGS})
#	SET(CMAKE_EXTRA_INCLUDE_FILES gcrypt.h)
#	SET(CMAKE_REQUIRED_FLAGS ${LIBGCRYPT_CFLAGS})
#	CHECK_TYPE_SIZE(gcry_error_t GCRY_ERROR_T)
#	SET(CMAKE_REQUIRED_FLAGS ${remember_defines})
#	SET(CMAKE_EXTRA_INCLUDE_FILES ${remember_includes})
#	IF(HAVE_GCRY_ERROR_T)
#		MESSAGE(STATUS "libgcrypt plugin enabled")
#		ADD_SUBDIRECTORY(qca-gcrypt)
#	ELSE(HAVE_GCRY_ERROR_T)
#		MESSAGE(STATUS "libgcrypt seems to be too old")
#	ENDIF(HAVE_GCRY_ERROR_T)
#ENDIF(LIBGCRYPT_FOUND)

#IF(NSS_FOUND)
#	MESSAGE(STATUS "NSS plugin enabled")
#	ADD_SUBDIRECTORY(qca-nss)
#ENDIF(NSS_FOUND)	


