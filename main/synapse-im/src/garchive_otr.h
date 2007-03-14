#ifndef GARCHIVE_OTR_H
#define GARCHIVE_OTR_H
#include <QObject>
#include <QList>
#include "xmpp.h"

class PsiAccount;

namespace Otr {

class OtrItem
{
public:
	OtrItem(const XMPP::Jid& _jid, bool _on = false);

	XMPP::Jid& jid();
	void setJid(const XMPP::Jid& _jid);
	
	void setOn(bool on);
	bool on();
	
private:
	XMPP::Jid jid_;
	bool on_;
};

class OtrList : public QList<OtrItem>
{
public:
	OtrList();
	~OtrList();
	
	OtrList::Iterator find(const XMPP::Jid& jid);
	
	bool isOn(const XMPP::Jid& jid);
	void setOn(const XMPP::Jid& jid, bool _on = false);
};

}

#endif

