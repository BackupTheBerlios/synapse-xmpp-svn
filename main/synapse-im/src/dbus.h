
#ifndef PSI_DBUS_H
#define PSI_DBUS_H

#include "psicon.h"

#define PSIDBUSNAME "org.psi-im.Psi"
#define PSIDBUSMAINIF "org.psi_im.Psi.Main"
bool dbusInit(const QString profile);

void addPsiConAdapter(PsiCon *psicon);

#endif

/*
//
// C++ Interface: dbus
//
// Description: 
//
//
// Author: root <root@mainframe>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DBUS_H
#define DBUS_H
#include <QtCore/QObject>
#include <QDBusConnection>
#include "mainwin.h"
#include "filetransdlg.h"
class MainWin;
class DBus: public QObject
{
	Q_OBJECT
//	Q_CLASSINFO("D-Bus Interface", "org.Synapse-IM")
public:
	DBus(MainWin *tmp);
	~DBus();
	
	static DBus* instance();

	MainWin *main_win;
	FileRequestDlg *frdlg;
	QString lastStatus;

private:
	static DBus* instance_;
	QDBusConnection *con;
	QDBusConnection *con1;

public Q_SLOTS:
	Q_SCRIPTABLE void sendNewMessageTo(const QString&,const QDBusMessage &);
	Q_SCRIPTABLE void sendFile(const QString& filename,const QDBusMessage &);
	Q_SCRIPTABLE void gameingStart(const QDBusMessage &);
	Q_SCRIPTABLE void gameingStop(const QDBusMessage &);
	Q_SCRIPTABLE void setStatus(const QString& show, const QString& text, bool available, const QDBusMessage &);
// signals:
// 	void sendNewMessageTo(const QString& to);
// 	void sendFile(const QString& filename);
// 	void gameingStart();
// 	void gameingStop();
// 	void setStatus(const QString& show, const QString& text, bool available = true);
};
#endif
*/