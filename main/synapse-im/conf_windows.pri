# Windows build settings
CONFIG += release
CONFIG += qca-static
DEFINES += HAVE_SNARL
CONFIG += whiteboarding
CONFIG += jingle google_voice
QT += sql

# OpenSSL
qca-static {
	DEFINES += HAVE_OPENSSL
	DEFINES += OSSL_097
	OPENSSL_PREFIX = C:\OpenSSL
	INCLUDEPATH += $$OPENSSL_PREFIX/include
	LIBS += -L$$OPENSSL_PREFIX/lib/MinGW
}

# SASL
#qca-static {
#	CYRUSSASL_PREFIX = /local
#	INCLUDEPATH += $$CYRUSSASL_PREFIX/include
#	LIBS += $$CYRUSSASL_PREFIX/lib/libsasl.lib
#}

# ASpell
#DEFINES += HAVE_ASPELL
contains(DEFINES, HAVE_ASPELL) {
	ASPELL_PREFIX = ../../../aspell
	INCLUDEPATH += "$$ASPELL_PREFIX/include"
	LIBS += -L"$$ASPELL_PREFIX/lib"
	LIBS += -laspell-15
}
