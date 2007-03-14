#ifndef GMAIL_NOTIFY_H
#define GMAIL_NOTIFY_H

#include <QObject>
#include "xmpp.h"

class PsiAccount;
class JT_GMailNotify;
class ContactViewItem;

class GMailNotify : public QObject
{
	Q_OBJECT
public:
	GMailNotify(PsiAccount *pa, const XMPP::Jid& receiver, bool enable);
	~GMailNotify();
	
	void init();
	void setJid(const XMPP::Jid& receiver);
	
	bool isNewMail();
	int newMailCount();
	QPixmap newMailIcon();
	
	bool isEnabled();
	void setEnabled(bool enable);
	ContactViewItem *cvi();
	void setCvi(ContactViewItem *i);
	
public slots:
	void done();
	
private:
	JT_GMailNotify *task_;
	PsiAccount *pa_;
	ContactViewItem *cvi_;
	bool enabled_;
};

#endif
