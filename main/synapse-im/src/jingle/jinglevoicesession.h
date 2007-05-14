//
// C++ Interface: jinglevoicesession
//
// Description: 
//
//
// Author: Andrzej Wójcik <andrzej@hi-low.eu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qstring.h>
#include <qdom.h>
#include <QObject>
#include "voicesession.h"
#include "voicecaller.h"
#include "psiaccount.h"
#include "codec.h"
#include "xmpp_client.h"
#include "xmpp_jid.h"
#include "xmpp_task.h"
#include "xmpp_xmlcommon.h"

#ifndef JINGLEVOICESESSION_H
#define JINGLEVOICESESSION_H

#define JINGLE_VOICE_NS "http://www.xmpp.org/extensions/xep-0167.html#ns"

using namespace XMPP;
class JingleSessionManager;

class JingleVoiceSession : public VoiceCaller
{
	Q_OBJECT
public:
	JingleVoiceSession(PsiAccount *pa, JingleSessionManager *jsm);
	~JingleVoiceSession();

	void initialize();
	void deinitialize();

	QString call(const Jid &jid);
	void accept(const Jid &jid, QString);
	void reject(const Jid &jid, QString);
	void terminate(const Jid &jid, QString);

	bool setPayload(QString sid,QDomElement &e);
	bool setTransport(QString sid,QDomElement &e);
	void sendPayload(QString sid);
	void sendTransportInfo(const Jid &jid, QString sid);

	void start(QString sid);
	void stop(QString sid);

	QDomDocument *doc()
	{
		return account()->client()->rootTask()->doc();
	};

signals:
	void incoming(const Jid &jid, QString sid);
	void send(const QDomElement &e);
	
	QString generateSID();

private:
	QList<Codec> codecs_;
	JingleSessionManager *jsm_;
};

#endif
