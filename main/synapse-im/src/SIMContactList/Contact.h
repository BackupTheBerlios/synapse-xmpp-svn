#include "Item.h"
#include "Name.h"
#include "userlist.h"
#include "iconset.h"

#ifndef CONTACT_H
#define CONTACT_H

namespace SIMContactList {

class Contact : public Item {
public:
	Contact(const UserListItem &_u, PsiAccount *_pa, List *cl, Item *parent);
	~Contact();

	QString name();
	XMPP::Jid jid();
	QPixmap state();
	QPixmap pixmap();
	QPixmap avatar();
	QString description();
	const SIMContactList::Name &contactName();
	const QColor &textColor();

	bool alerting();
	void setAlertIcon(PsiIcon *icon);
	void setBlocked(bool);

	void setUserListItem(const UserListItem &_u);
	UserListItem *u();

	XMPP::Status status();

	void showContextMenu(const QPoint&);
	QString toolTip();

	static Item *updateParent(Contact *item, SIMContactList::List *contactList);
	void updateOptions();

private:
	UserListItem u_;
	SIMContactList::Name contactName_;
	PsiIcon *alertIcon_;
	bool blocked_;
};

};

#endif
