//
// C++ Interface: jinglesessionmanager
//
// Description: 
//
//
// Author: Andrzej Wójcik <andrzej@hi-low.eu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef JINGLESESSIONMANAGER_H
#define JINGLESESSIONMANAGER_H
#define JINGLE_NS "http://www.xmpp.org/extensions/xep-0166.html#ns"

#include <qstring.h>
#include <qdom.h>
#include <q3ptrlist.h>
#include <QObject>
#include "xmpp_jid.h"
#include "xmpp_task.h"
#include "xmpp_xmlcommon.h"
#include "psiaccount.h"
#include "jinglevoicesession.h"
#include "transport.h"

using namespace XMPP;

class JingleSessionManager : public Task
{
	Q_OBJECT
public:
	JingleSessionManager(PsiAccount *pa, Task *t);
	~JingleSessionManager();
	bool take(const QDomElement &x);
	void onGo();

	QDomElement sessionHeader(const Jid &jid, const Jid &initiator, QString &sid);
	QDomElement sessionInitiate(const Jid &jid, const Jid &initiator, QString &sid);
	QDomElement sessionAccept(const Jid &jid, const Jid &initiator, QString &sid);
	QDomElement sessionTransportInfo(const Jid &jid, const Jid &initiator, QString &sid);
	QDomElement sessionTerminate(const Jid &jid, const Jid &initiator, QString &sid);

	QString generateSID();
	void registerSession(Session *sess);
	Session *getSession(QString sid);
	void unregisterSession(QString sid);

	Jid jid();

public slots:

	JingleVoiceSession *voiceSession();
	void sendStanza(const QDomElement &e);

private:
	JingleVoiceSession *voiceSession_;	
	Q3PtrList<Session> sessionList_;
	PsiAccount *pa_;
	Client *client_;
	Transport *transport_;
	Jid jid_;
	int sid_seed;
};

#endif
