#include "SIMContactListItem.h"
#include "SIMContactName.h"
#include "userlist.h"
#include "iconset.h"

#ifndef SIMCONTACTLISTCONTACT_H
#define SIMCONTACTLISTCONTACT_H

class SIMContactListContact : public SIMContactListItem {
public:
	SIMContactListContact(const UserListItem &_u, PsiAccount *_pa, SIMContactList *cl, SIMContactListItem *parent);
	~SIMContactListContact();

	QString name();
	XMPP::Jid jid();
	QPixmap state();
	QPixmap pixmap();
	QPixmap avatar();
	const QString &description();
	const SIMContactName &contactName();
	const QColor &textColor();

	bool alerting();
	void setAlertIcon(PsiIcon *icon);

	void setUserListItem(const UserListItem &_u);
	UserListItem *u();

	XMPP::Status status();

	void showContextMenu(const QPoint&);
	QString toolTip();

	static SIMContactListItem *updateParent(SIMContactListContact *item, SIMContactList *contactList);

private:
	UserListItem u_;
	SIMContactName contactName_;
	PsiIcon *alertIcon_;
};

#endif
