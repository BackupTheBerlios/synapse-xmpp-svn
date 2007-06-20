#ifndef SESSION_H
#define SESSION_H

#include <QString>
#include "transport.h"
#include "xmpp_jid.h"
class Session
{
public:
	Session() { transport_ = new Transport(); };
	~Session(){ if(transport_) delete transport_; };

	QDomElement transportInfo(QDomElement &candidate) { return transport_->info(candidate); };

	QString sid;
	XMPP::Jid jid;
	XMPP::Jid initiator;
	QList<Transport::Params> tpl;
	Transport *transport_;	
};
#endif
