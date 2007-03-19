# qconf

PREFIX = /usr
BINDIR = /usr/bin
DATADIR = /usr/share

DEFINES += OSSL_097 HAVE_OPENSSL QCA_SYSTEMSTORE_PATH=\\\"/etc/ssl/certs/ca-certificates.crt\\\" HAVE_XSS HAVE_ASPELL HAVE_DNOTIFY HAVE_GETHOSTBYNAME_R HAVE_SPEEX HAVE_ORTP HAVE_ALSA_ASOUNDLIB_H __ALSA_ENABLED__ HAVE_GLIB HAVE_CONFIG
INCLUDEPATH += /usr/include/speex   /usr/include/glib-2.0 /usr/lib/glib-2.0/include
LIBS += -lssl -lcrypto -lz -lXss -laspell -lexpat -lspeex -lortp -lasound -lglib-2.0 -lgmodule-2.0 -lgthread-2.0
CONFIG += qca-static
CONFIG += google_ft google_voice
CONFIG += jingle
DEFINES += HAVE_DBUS
CONFIG += qdbus
CONFIG += dbus
CONFIG += release
PREFIX=/usr
DATADIR=/usr/share/psi

