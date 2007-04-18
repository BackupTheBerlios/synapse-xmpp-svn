# NOTE: All files are relative to the win32 subdir

#QCADIR=..\..\qca-2.0-beta1
#QCA_OPENSSLDIR=..\..\qca-openssl
ASPELLDIR=C:\aspell
QTDIR=C:\Qt\4.2.2
OPENSSL=C:\OpenSSL\lib\MinGW
MINGWDIR=c:\MingW
WINDOWSDIR=C:\Windows

TARGET_DIR=synapse-im

.PHONY: all
all:
	cd .. && $(MAKE)
	-mkdir $(TARGET_DIR)
	copy ..\src\release\synapse-im.exe $(TARGET_DIR)
	rem copy $(QCADIR)\lib\qca2.dll $(TARGET_DIR)
	copy $(QTDIR)\bin\QtCore4.dll $(TARGET_DIR)
	copy $(QTDIR)\bin\QtGui4.dll $(TARGET_DIR)
	copy $(QTDIR)\bin\QtNetwork4.dll $(TARGET_DIR)
	copy $(QTDIR)\bin\QtXml4.dll $(TARGET_DIR)
	copy $(QTDIR)\bin\Qt3Support4.dll $(TARGET_DIR)
	copy $(QTDIR)\bin\QtSql4.dll $(TARGET_DIR)
	copy $(MINGWDIR)\bin\mingwm10.dll $(TARGET_DIR)
	copy ..\src\tools\idle\win32\idleui.dll $(TARGET_DIR)
	xcopy /i /e /q /y ..\certs $(TARGET_DIR)\certs
	xcopy /i /e /q /y ..\iconsets $(TARGET_DIR)\iconsets
	xcopy /i /e /q /y ..\sound $(TARGET_DIR)\sound
	copy ..\COPYING $(TARGET_DIR)
	rem -mkdir $(TARGET_DIR)\crypto
	rem copy $(QCA_OPENSSLDIR)\release\qca-openssl.dll $(TARGET_DIR)\crypto
	copy $(WINDOWSDIR)\system32\libeay32.dll $(TARGET_DIR)
	copy $(WINDOWSDIR)\system32\ssleay32.dll $(TARGET_DIR)
	xcopy /i /e /q /y $(QTDIR)\plugins\imageformats $(TARGET_DIR)\imageformats
	-mkdir $(TARGET_DIR)\sqldrivers
	copy $(QTDIR)\plugins\sqldrivers\qsqlite.dll $(TARGET_DIR)\sqldrivers