#include "SIMContactListItem.h"
#include "SIMContactName.h"
#include "userlist.h"
//#include "iconset.h"

#ifndef SIMCONTACTLISTMETA_H
#define SIMCONTACTLISTMETA_H

class SIMContactListMeta : public SIMContactListItem {
public:
	SIMContactListMeta(const QString &_name, PsiAccount *_pa, SIMContactList *cl, SIMContactListItem *parent);
	~SIMContactListMeta();

	QString name();
	XMPP::Jid jid();
	QPixmap state();
	QPixmap pixmap();
	QPixmap avatar();
	const QString &description();
	const SIMContactName &contactName();
	const QColor &textColor();

// 	bool alerting();
// 	void setAlertIcon(PsiIcon *icon);

// 	void setUserListItem(const UserListItem &_u);
	UserListItem *u();

	XMPP::Status status();

	void showContextMenu(const QPoint&);
	QString toolTip();

	SIMContactListItem *updateParent();
//	void updateOptions();
private:
	QString name_;
};

#endif
