#ifndef GARCHIVE_H
#define GARCHIVE_H

#include <QObject>
#include <QList>
#include "xmpp.h"
#include "xmpp_jid.h"

class PsiAccount;
class JT_GArchive;

class GArchive : public QObject
{
	Q_OBJECT
public:
	GArchive(PsiAccount *_pa,const XMPP::Jid& _jid, bool enabled);
	~GArchive();

	void reset();
	void enable();
	bool isEnabled();

	bool isOtrOn(const XMPP::Jid& _jid);

	void changeSave();
	
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
