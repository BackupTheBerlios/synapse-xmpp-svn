//
// C++ Implementation: jinglesessionmanager
//
// Description: 
//
//
// Author: Andrzej Wójcik <andrzej@hi-low.eu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "jinglesessionmanager.h"
#include "jinglevoicesession.h"
#include "xmpp.h"
#include "xmpp_xmlcommon.h"
#include "psiaccount.h"
#include <qstring.h>
#include <qdom.h>


JingleSessionManager::JingleSessionManager(PsiAccount *pa, Task *t)
: Task(t), pa_(pa), client_(pa->client()), jid_(pa->jid()), sid_seed(0xaaaa)
{
	voiceSession_ = new JingleVoiceSession(pa,this);
	transport_ = NULL;

	connect(voiceSession_, SIGNAL(send(const QDomElement&)), this, SLOT(sendStanza(const QDomElement&)));
	Task::go(false);
}

JingleSessionManager::~JingleSessionManager()
{
	delete voiceSession_;
}

bool JingleSessionManager::take(const QDomElement &e)
{
	if(e.tagName() != "iq")
		return false;

	if(e.attribute("type") != "error")
	{
	bool found = false;
	QDomElement jt = findSubTag(e, "jingle", &found);
	if(found && jt.attribute("xmlns") == JINGLE_NS) {
		QDomElement content = findSubTag(jt,"content",&found);
		if(found && jt.attribute("action") == "session-initiate") {
			VoiceSession *sess = new VoiceSession(jt.attribute("sid"));
			sess->jid = Jid(e.attribute("from"));
			sess->initiator = Jid(e.attribute("from"));
			registerSession(sess);
			if(voiceSession_->setPayload(jt.attribute("sid"),content)) {
				setSuccess();
			} else
				setError(e);
		} else if (found && jt.attribute("action") == "session-accept") {
				voiceSession_->setPayload(jt.attribute("sid"),content);
				voiceSession_->start(jt.attribute("sid"));
				setSuccess();
		} else if(jt.attribute("action") == "transport-info") {
			if(voiceSession_->setTransport(jt.attribute("sid"),content)) {
				setSuccess();
			} else
				setError(e);
		} else if (jt.attribute("action") == "session-terminate") {
				voiceSession_->stop(jt.attribute("sid"));
				setSuccess();
		}
		return true;
	}
	}
	return false;
}

void JingleSessionManager::onGo()
{
}

QString JingleSessionManager::generateSID()
{
	QString sid;
	sid.sprintf("a%x", sid_seed);
	sid_seed += 0x10;
	return sid;
}

void JingleSessionManager::registerSession(Session *sess)
{
	sessionList_.append(sess);
}

Session *JingleSessionManager::getSession(QString sid)
{
	Q3PtrListIterator<Session> it(sessionList_);
	for(Session *i; (i = it.current()); ++it) {
		printf("%s : %s\n",i->sid.ascii(), sid.ascii());
		if(i->sid == sid)
			return i;
	}
	return NULL;
}

void JingleSessionManager::unregisterSession(QString sid)
{
	Session *sess = getSession(sid);
	printf("removeRef\n");
	sessionList_.removeRef(sess);
	printf("delete\n");
	delete sess;
}

QDomElement JingleSessionManager::sessionHeader(const Jid &j, const Jid &initiator, QString &sid)
{
	QDomElement iq = createIQ(doc(), "set", j.full(), id());
	QDomElement jt = doc()->createElement("jingle");
	jt.setAttribute("xmlns", JINGLE_NS);
	jt.setAttribute("initiator",initiator.full());
	jt.setAttribute("sid", sid);
	iq.appendChild(jt);
	printf("sH:%s\n",iq.text().ascii());
	return iq;
}

QDomElement JingleSessionManager::sessionInitiate(const Jid &j, const Jid &initiator, QString &sid)
{
	if(transport_)
		delete transport_;
	transport_ = new Transport();
	printf("getStunInfo()\n");
	transport_->getStunInfo();
	QDomElement iq = sessionHeader(j, initiator, sid);
	bool found;
	QDomElement jt = findSubTag(iq, "jingle", &found);
	jt.setAttribute("action","session-initiate");
	printf("sI:%s\n",iq.text().ascii());
	return iq;
}

QDomElement JingleSessionManager::sessionAccept(const Jid &j, const Jid &initiator, QString &sid)
{
	QDomElement iq = sessionHeader(j, initiator, sid);
	bool found;
	QDomElement jt = findSubTag(iq, "jingle", &found);
	jt.setAttribute("action","session-accept");
	return iq;
}

QDomElement JingleSessionManager::sessionTransportInfo(const Jid &j, const Jid &initiator, QString &sid)
{
	QDomElement iq = sessionHeader(j, initiator, sid);
	bool found;
	QDomElement jt = findSubTag(iq, "jingle", &found);
	jt.setAttribute("action","transport-info");
	return iq;
}

QDomElement JingleSessionManager::sessionTerminate(const Jid &j, const Jid &initiator, QString &sid)
{
	QDomElement iq = sessionHeader(j, initiator, sid);
	bool found;
	QDomElement jt = findSubTag(iq, "jingle", &found);
	jt.setAttribute("action","session-terminate");
	return iq;
}

JingleVoiceSession *JingleSessionManager::voiceSession()
{
	return voiceSession_;
}

void JingleSessionManager::sendStanza(const QDomElement &e)
{
	printf("sendStanza()\n");
	QString s = e.text();
	client_->send(e);
}

Jid JingleSessionManager::jid()
{
	return jid_;
}
