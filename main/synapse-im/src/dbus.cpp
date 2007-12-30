#include <QString>
#include <QVector>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusAbstractAdaptor>

#include "common.h"
#include "dbus.h"

#include "psicontactlist.h"
#include "psiaccount.h"

#define PSIDBUSIFACE "org.psi_im.Psi"



class PsiConAdapter : public QDBusAbstractAdaptor 
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.psi_im.Psi.Main")
//	Q_CLASSINFO("D-Bus Introspection", ...) 

public: 
	PsiConAdapter(PsiCon *psicon_);
	~PsiConAdapter();
public Q_SLOTS:
//	void openURI(QString uri);
	void raise();
/*Q_SIGNALS:
	void psi_pong();
*/
private: 
	PsiCon *psicon;
};



PsiConAdapter::PsiConAdapter(PsiCon *psicon_) : QDBusAbstractAdaptor(psicon_)
{
	psicon = psicon_;
}

PsiConAdapter::~PsiConAdapter()
{}

/*void PsiConAdapter::openURI(QString uri)
{
	psicon->doOpenUri(uri);
}*/

// FIXME libguniqueapp uses activate
void PsiConAdapter::raise()
{
	psicon->raiseMainwin();
}




void addPsiConAdapter(PsiCon *psicon)
{
	new PsiConAdapter(psicon);
	QDBusConnection::sessionBus().registerObject("/Main", psicon);
}


#include "dbus.moc"
/*
//
// C++ Implementation: dbus
//
// Description: 
//
//
// Author: root <root@mainframe>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <QObject>
#include <QDBusAbstractAdaptor>
#include "filetransdlg.h"
#include "psioptions.h"
#include "psicon.h"
#include "dbus.h"

class testa : public QObject
{
	Q_OBJECT
};

DBus::DBus(MainWin *tmp)
//: QDBusAbstractAdaptor(tmp)
{
	this->main_win = tmp;
	instance_ = this;
	this->frdlg = NULL;
	this->lastStatus = "";
// 	connect(this,SIGNAL(sendNewMessageTo(const QString &)),SLOT(sendNewMessageToSlot(const QString &)));
// 	connect(this,SIGNAL(sendFile(const QString& filename)),SLOT(sendFileSlot(const QString& filename)));
// 	connect(this,SIGNAL(gameingStart()),SLOT(gameingStartSlot()));
// 	connect(this,SIGNAL(gameingStop()),SLOT(gameingStopSlot()));
// 	connect(this,SIGNAL(setStatus(const QString& show, const QString& text, bool available)),SLOT(setStatusSlot(const QString& show, const QString& text, bool available)));
	QDBusConnection::sessionBus().registerService("org.Synapse-IM");
	QDBusConnection::sessionBus().registerObject("/IM",this,QDBusConnection::ExportAllSlots);
	//con1->registerObject("/Synapse-IM",this);
}

DBus::~DBus()
{
}

DBus* DBus::instance()
{
	return instance_;
}

void DBus::sendNewMessageTo(const QString& to,const QDBusMessage & msg)
{
    PsiCon *psicon = this->main_win->psiCon();
    psicon->doNewBlankMessage(to);
}

void DBus::sendFile(const QString& filename,const QDBusMessage & msg)
{
    if(frdlg)
    {
    QString file = QString(filename);
    this->frdlg->setFile(file);
    frdlg=NULL;
    }
}

void DBus::gameingStart(const QDBusMessage & msg)
{
    PsiCon *psicon = this->main_win->psiCon();
    lastStatus = psicon->lastStatusString;
    psicon->setGlobalStatus(Status("dnd","playing.. :)",0,true));
}

void DBus::gameingStop(const QDBusMessage & msg)
{
    PsiCon *psicon = this->main_win->psiCon();
    psicon->setGlobalStatus(Status("",lastStatus,0,true));
}

void DBus::setStatus(const QString& show, const QString& text, bool available,const QDBusMessage & msg)
{
    PsiCon *psicon = this->main_win->psiCon();
    psicon->setGlobalStatus(Status(show,text,0,available));
}

DBus* DBus::instance_ = NULL;
#include "dbus.moc"
*/