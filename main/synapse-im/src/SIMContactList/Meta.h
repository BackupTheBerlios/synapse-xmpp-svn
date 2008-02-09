#include "Item.h"
#include "Name.h"
#include "userlist.h"
//#include "iconset.h"

#ifndef META_H
#define META_H

namespace SIMContactList {

class Meta : public Item {
public:
	Meta(const QString &_name, PsiAccount *_pa, List *cl, Item *parent);
	~Meta();

	QString name();
	XMPP::Jid jid();
	QPixmap state();
	QPixmap pixmap();
	QPixmap avatar();
	const QString &description();
	const Name &contactName();
	const QColor &textColor();

// 	bool alerting();
// 	void setAlertIcon(PsiIcon *icon);

// 	void setUserListItem(const UserListItem &_u);
	UserListItem *u();

	XMPP::Status status();

	void showContextMenu(const QPoint&);
	QString toolTip();

	Item *updateParent();
//	void updateOptions();
private:
	QString name_;
};

};

#endif
