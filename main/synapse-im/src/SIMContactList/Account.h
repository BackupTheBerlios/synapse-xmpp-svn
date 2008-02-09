#include "Item.h"
#include "userlist.h"
#include "iconset.h"

#ifndef ACCOUNT_H
#define ACCOUNT_H

namespace SIMContactList {

class Group;
class Contact;
class Meta;

class Account : public Item {
// 	Q_OBJECT
public:
	Account(PsiAccount *pa, List *cl, Item *parent);
	~Account();

	const QString &name();
	const QPixmap &pixmap();

	void setState(int _state);

	void showContextMenu(const QPoint&);
	QString toolTip();

	void setAlert(const XMPP::Jid &j, PsiIcon *icon);

	void updateEntry(const UserListItem &u);
	void removeEntry(const XMPP::Jid &j);

	Item *updateParent();

private:
	SIMContactList::Group *ensureGroup(const QString &group_name);
	SIMContactList::Meta *ensureMeta(const QString  &meta_name, SIMContactList::Group *group);

	int state_;
};

};

#endif
