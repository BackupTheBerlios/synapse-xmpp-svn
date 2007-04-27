# qconf

PREFIX = /usr
BINDIR = /usr/bin
DATADIR = /usr/share

DEFINES += OSSL_097 HAVE_OPENSSL HAVE_CYRUSSASL QCA_SYSTEMSTORE_PATH=\\\"/etc/ssl/certs/ca-certificates.crt\\\" HAVE_XSS HAVE_ASPELL HAVE_DNOTIFY HAVE_GETHOSTBYNAME_R HAVE_SPEEX HAVE_ORTP HAVE_CONFIG
INCLUDEPATH += /usr/include/speex 
LIBS += -lssl -lcrypto -lsasl2 -lz -lXss -laspell -lexpat -lspeex -lortp
CONFIG += qca-static
CONFIG += google_ft google_voice
CONFIG += jingle
CONFIG += release
PREFIX=/usr
DATADIR=/usr/share/synapse-im

