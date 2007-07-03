#ifndef AHC_PLUGIN_H
#define AHC_PLUGIN_H

#include <QObject>
#include "coreinterface.h"
#include "ahcommanddlg.h"
#include "ahcservermanager.h"
#include "xmpp_jid.h"
#include "rc.h"
class PsiAccount;
class PsiCon;
class ContactViewItem;

class AHCBox : public CoreInterface
{
	Q_INTERFACES(CoreInterface);
public:
	~AHCBox();

	QString name();
	QString version();

	void setup(PsiAccount *pa, const XMPP::Jid& receiver, bool enable);
	void init();
	void reset();
	void setJid(const XMPP::Jid& receiver);

	bool isEnabled();
	void setEnabled(bool enable);

	bool isEvent(const XMPP::Jid& _jid);
	int countEvents();

	void changed();

	ContactViewItem *cvi();
	void setCvi(ContactViewItem *i);

	void setPsiCon(PsiCon *psi);
	void process(const XMPP::Jid&, const QString&);

public slots:
	void updated(const XMPP::Jid& _jid);
	void change(const XMPP::Jid& _jid);
	void set(bool state);

private:
	PsiAccount *pa_;
	PsiCon *psi_;
	AHCServerManager *ahcManager;
 	RCSetStatusServer *rcSetStatusServer;
 	RCForwardServer *rcForwardServer;
 	RCSetOptionsServer *rcSetOptionsServer;
};

#endif
