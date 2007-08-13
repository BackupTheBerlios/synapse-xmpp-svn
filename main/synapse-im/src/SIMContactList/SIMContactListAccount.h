#include "SIMContactListItem.h"
#include "userlist.h"
#include "iconset.h"

#ifndef SIMCONTACTLISTACCOUNT_H
#define SIMCONTACTLISTACCOUNT_H

class SIMContactListGroup;
class SIMContactListContact;

class SIMContactListAccount : public SIMContactListItem {
// 	Q_OBJECT
public:
	SIMContactListAccount(PsiAccount *pa, SIMContactList *cl, SIMContactListItem *parent);
	~SIMContactListAccount();

	const QString &name();
	const QPixmap &pixmap();

	void setState(int _state);

	void showContextMenu(const QPoint&);
	QString toolTip();

	void setAlert(const XMPP::Jid &j, PsiIcon *icon);

	void updateEntry(const UserListItem &u);
	void removeEntry(const XMPP::Jid &j);

	SIMContactListItem *updateParent();

private:
	SIMContactListGroup *ensureGroup(const QString &group_name);

	int state_;
};

#endif
