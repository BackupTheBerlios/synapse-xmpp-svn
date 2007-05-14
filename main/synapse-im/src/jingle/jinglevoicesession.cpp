//
// C++ Implementation: jinglevoicesession
//
// Description: 
//
//
// Author: Andrzej Wójcik <andrzej@hi-low.eu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "jinglevoicesession.h"
#include "jinglesessionmanager.h"
#include "xmpp.h"
#include "xmpp_xmlcommon.h"
#include "voicecodec.h"
#include <qstring.h>
#include <qvariant.h>
#include <qdom.h>

JingleVoiceSession::JingleVoiceSession(PsiAccount *pa, JingleSessionManager *jsm)
: VoiceCaller(pa), jsm_(jsm)
{
}

JingleVoiceSession::~JingleVoiceSession()
{
}

void JingleVoiceSession::initialize()
{
}

void JingleVoiceSession::deinitialize()
{
}

QString JingleVoiceSession::call(const Jid &jid)
{
	printf("call()\n");
	QString sid = jsm_->generateSID();
	printf("SID\n");
	QDomElement s = jsm_->sessionInitiate(jid, jsm_->jid(), sid);
	printf("Init\n");

	VoiceSession *sess = new VoiceSession(sid);
	sess->jid = jid.full();
	sess->initiator = jsm_->jid();
	jsm_->registerSession(sess);
	printf("reg\n");

	bool found;
	QDomElement jt = findSubTag(s, "jingle", &found);
	QDomElement content = doc()->createElement("content");
	content.setAttribute("name","Audio-Content");
	content.setAttribute("profile","RTP/AVP");
	jt.appendChild(content);
// create description with codecs
	QDomElement desc = doc()->createElement("description");
	desc.setAttribute("xmlns",JINGLE_VOICE_NS);

	Q3ValueList<int>::iterator i = CodecsManager::instance()->payloads().begin();
	printf("..seaching in codecs..\n");
	for (i = CodecsManager::instance()->payloads().begin(); i != CodecsManager::instance()->payloads().end(); ++i) {
		VoiceCodecFactory *vcf = CodecsManager::instance()->codecFactory((*i));
		if(vcf == NULL)
			break;
		printf("id= %d\n", (*i));
		QDomElement payload = doc()->createElement("payload-type");
		payload.setAttribute("id", (*i));
		payload.setAttribute("name", vcf->name());
		payload.setAttribute("clockrate", 8000);
		desc.appendChild(payload);
	}

	content.appendChild(desc);
// create transport list
	QDomElement trans = doc()->createElement("transport");
	trans.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0176.html#ns-udp");
	content.appendChild(trans);
// send it
	printf("send()\n");
	send(s);
	sendTransportInfo(jid, sid);	
	return sid;
}

void JingleVoiceSession::accept(const Jid &jid, QString sid)
{
	start(sid);
	sendPayload(sid);
}

void JingleVoiceSession::reject(const Jid &jid, QString sid)
{
	stop(sid);
}

void JingleVoiceSession::terminate(const Jid &jid, QString sid)
{
	stop(sid);
}


bool JingleVoiceSession::setPayload(QString sid,QDomElement &e)
{
	VoiceSession *s = (VoiceSession*) jsm_->getSession(sid);
	if(!s)
		printf("no session!!\n");
	bool found = false;
	QDomElement desc = findSubTag(e, "description", &found);
	if(found && (desc.attribute("xmlns") == JINGLE_VOICE_NS )) {
		for(QDomNode n = desc.firstChild(); !n.isNull(); n = n.nextSibling())
		{
			QDomElement a = n.toElement();
			if(a.isNull())
				continue;
			if(a.tagName() == "payload-type") {
				Q3ValueList<int>::iterator i = CodecsManager::instance()->payloads().begin();
				printf("..seaching in codecs..\n");
				for (i = CodecsManager::instance()->payloads().begin(); i != CodecsManager::instance()->payloads().end(); i++) {
					VoiceCodecFactory *vcf = CodecsManager::instance()->codecFactory((*i));
					if(vcf == NULL)
						break;
      					printf("id= %s %d\n", a.attribute("id").ascii(), (*i));
      					if ((QVariant(a.attribute("id")).toInt() == (*i)) && a.attribute("name") == vcf->name()) {
						s->codec = QVariant(a.attribute("id")).toInt();
          					printf("..founded. (%d)\n", s->codec);
          					return true;
				        }
				}
			}
		}
	}
	return false;
}

bool JingleVoiceSession::setTransport(QString sid,QDomElement &e)
{
	VoiceSession *s = (VoiceSession*) jsm_->getSession(sid);
	if(s) {
		bool found = false;
		QDomElement trans = findSubTag(e, "transport", &found);
		if(found) {
			QDomElement candidate = findSubTag(trans, "candidate", &found);
			if(found) {
				s->ip = candidate.attribute("ip");
				s->port = candidate.attribute("port");
				s->protocol = candidate.attribute("protocol");
			}
		}
		emit incoming(s->jid,sid);
	}
}

void JingleVoiceSession::sendPayload(QString sid)
{
	VoiceSession *sess = (VoiceSession*) jsm_->getSession(sid);
	if(!sess)
		return;
	QDomElement s = jsm_->sessionAccept(sess->jid, sess->initiator, sid);
	bool found;
	QDomElement jt = findSubTag(s, "jingle", &found);
	QDomElement content = doc()->createElement("content");
	content.setAttribute("name","Audio-Content");
	content.setAttribute("profile","RTP/AVP");
	jt.appendChild(content);
// create description with codecs
	QDomElement desc = doc()->createElement("description");
	desc.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0167.html#ns");

	VoiceCodecFactory *vcf = CodecsManager::instance()->codecFactory(sess->codec);
	if(vcf == NULL)
		return;
	printf("id= %d\n", sess->codec);
	QDomElement payload = doc()->createElement("payload-type");
	payload.setAttribute("id", sess->codec);
	payload.setAttribute("name", vcf->name());
	payload.setAttribute("clockrate", (int)(vcf->bandwidth() * 1000));
	desc.appendChild(payload);

	content.appendChild(desc);
// create transport list
	QDomElement trans = doc()->createElement("transport");
	trans.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0176.html#ns-udp");
	content.appendChild(trans);
	QDomElement candidate = doc()->createElement("candidate");
	candidate = sess->transportInfo(candidate);
	trans.appendChild(candidate);
	send(s);
}

void JingleVoiceSession::sendTransportInfo(const Jid &jid, QString sid)
{
	VoiceSession *sess = (VoiceSession*) jsm_->getSession(sid);
	if(!sess)
		return;
	QDomElement s = jsm_->sessionTransportInfo(jid, sess->initiator, sid);
	bool found;
	QDomElement jt = findSubTag(s, "jingle", &found);
	QDomElement content = doc()->createElement("content");
	content.setAttribute("name","Audio-Content");
	content.setAttribute("profile","RTP/AVP");
	jt.appendChild(content);
	QDomElement desc = doc()->createElement("description");
	desc.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0167.html#ns");
	content.appendChild(desc);
	QDomElement trans = doc()->createElement("transport");
	trans.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0176.html#ns-udp");
	content.appendChild(trans);
	QDomElement candidate = doc()->createElement("candidate");
	candidate = sess->transportInfo(candidate);
	trans.appendChild(candidate);
	send(s);
}

void JingleVoiceSession::start(QString sid)
{
	VoiceSession *sess = (VoiceSession*) jsm_->getSession(sid);
	if(!sess)
		return;
	accepted(sess->jid);
	sess->start();
}

void JingleVoiceSession::stop(QString sid)
{
	VoiceSession *sess = (VoiceSession*) jsm_->getSession(sid);
	if(sess == NULL)
	{
		printf("No session\n");
		return;
	}
	sess->stop();
	terminated(sess->jid);
	send(jsm_->sessionTerminate(sess->jid, sess->initiator, sess->sid));
	jsm_->unregisterSession(sid);
}

