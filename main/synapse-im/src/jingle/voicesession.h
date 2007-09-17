#ifndef VOICESESSION_H
#define VOICESESSION_H
#include "session.h"
#include <QString>
#include "mediastream.h"
class VoiceSession : public Session
{
public:
	VoiceSession(QString _sid) { sid = _sid; ms_ = new MediaStream(); }
	~VoiceSession(){ delete ms_;};
	int codec;
	MediaStream *ms_;

	void start() {
		char* endPtr=NULL;
		for(QList<Transport::Params>::Iterator addr = tpl.begin(); addr != tpl.end(); addr) 
		{
			if(ms_->start(ntohl(inet_addr((*addr).ip.ascii())),strtol((*addr).port.ascii(),&endPtr,10), transport_->firewallPort(), codec))
				break;
		}
	}

	void stop() {
		ms_->stop();
	}
};

#endif
