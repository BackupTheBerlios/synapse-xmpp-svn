#ifndef GMAIL_NOTIFY_H
#define GMAIL_NOTIFY_H

#include <QObject>
#include "xmpp.h"
#include "coreinterface.h"

class PsiAccount;
class JT_GMailNotify;
class ContactViewItem;
class PsiCon;

class GMailNotify : public CoreInterface
{
        Q_INTERFACES(CoreInterface);
public:
	~GMailNotify();
	
	QString name();
	QString version();

	void setup(PsiAccount *pa, const XMPP::Jid& receiver, bool enable);
	void init();
	void reset();
	void setJid(const XMPP::Jid& receiver);
	
	bool isEvent(const XMPP::Jid &reciver);
	int countEvents();
	
	bool isEnabled();
	void setEnabled(bool enable);

	void changed();

	ContactViewItem *cvi();
	void setCvi(ContactViewItem *i);

	void setPsiCon(PsiCon* psi);
	void process(const XMPP::Jid&, const QString&);

public slots:
	void updated(const XMPP::Jid& _jid);
	void change(const XMPP::Jid& _jid);
	void set(bool state);

	void done();

private:
	JT_GMailNotify *task_;
	PsiAccount *pa_;
	ContactViewItem *cvi_;
	bool enabled_;
};
#endif
