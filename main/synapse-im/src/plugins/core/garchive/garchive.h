#ifndef GARCHIVE_H
#define GARCHIVE_H

#include <QObject>
#include <QList>
#include "xmpp.h"
#include "xmpp_jid.h"
#include "coreinterface.h"

class PsiAccount;
class JT_GArchive;
class ContactViewItem;

class GArchive : public CoreInterface
{
	Q_INTERFACES(CoreInterface);
public:
	~GArchive();

	QString name();
	QString version();

	void setup(PsiAccount *_pa,const XMPP::Jid& _jid, bool enabled);
	void init();
	void reset();
	void setJid(const XMPP::Jid& receiver);

	bool isEvent(const XMPP::Jid& _jid);
	int countEvents();

	void setEnabled(bool);
	bool isEnabled();

	void changed();

	ContactViewItem *cvi();
	void setCvi(ContactViewItem *i);

public slots:
	void otrChanged(const XMPP::Jid& _jid);
	void changeOtr(const XMPP::Jid& _jid);
	void setSave(bool state);

private:
	PsiAccount *pa_;
	JT_GArchive *task_;
	XMPP::Jid receiver_;
	bool save_;
};

#endif
