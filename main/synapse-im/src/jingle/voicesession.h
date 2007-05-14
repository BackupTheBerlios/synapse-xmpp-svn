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
		ms_->start(ntohl(inet_addr(ip.ascii())),strtol(port.ascii(),&endPtr,10),codec);
	}

	void stop() {
		ms_->stop();
	}
};

#endif
