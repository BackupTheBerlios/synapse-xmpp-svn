#include "xmpp_jid.h"
#include <QObject>
class PsiAccount;
class ContactViewItem;
#ifndef COREINTERFACE_H
#define COREINTERFACE_H

class CoreInterface :  public QObject {
	Q_OBJECT
public:
	virtual ~CoreInterface(){};
	virtual QString name() = 0;
	virtual QString version() = 0;

	virtual void setup(PsiAccount *pa, const XMPP::Jid& receiver, bool enable) = 0;
	virtual void init() = 0;
	virtual void reset() = 0;
	virtual void setJid(const XMPP::Jid& receiver) = 0;

	virtual bool isEnabled() = 0;
	virtual void setEnabled(bool enable) = 0;

	virtual bool isEvent(const XMPP::Jid& _jid) = 0;
	virtual int countEvents() = 0;

	virtual void changed() = 0;

	virtual ContactViewItem *cvi() = 0;
	virtual void setCvi(ContactViewItem *i) = 0;


};

#endif
