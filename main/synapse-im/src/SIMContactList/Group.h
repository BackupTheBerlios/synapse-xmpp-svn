#include "Item.h"

#ifndef GROUP_H
#define GROUP_H
#include "xmpp_jid.h"

namespace SIMContactList {

class Contact;

class Group : public Item {
// 	Q_OBJECT
public:
	Group(const QString &_name, PsiAccount *_pa, List *cl, Item *parent);
	~Group();

	const QString &name();
	const QPixmap &pixmap();

	SIMContactList::Contact *findEntry(const QString &j);

	void showContextMenu(const QPoint&);
	QString toolTip();

	Item *updateParent();

private:
	QString group_name;
};

};

#endif
