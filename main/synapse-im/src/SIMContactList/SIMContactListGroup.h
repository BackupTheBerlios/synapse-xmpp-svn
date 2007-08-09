#include "SIMContactListItem.h"

#ifndef SIMCONTACTLISTGROUP_H
#define SIMCONTACTLISTGROUP_H
#include "xmpp_jid.h"

class SIMContactListContact;

class SIMContactListGroup : public SIMContactListItem {
// 	Q_OBJECT
public:
	SIMContactListGroup(const QString &_name, PsiAccount *_pa, SIMContactList *cl, SIMContactListItem *parent);
	~SIMContactListGroup();

	const QString &name();
	const QPixmap &pixmap();

	SIMContactListContact *findEntry(const QString &j);

	void showContextMenu(const QPoint&);
	QString toolTip();

	SIMContactListItem *updateParent();

private:
	QString group_name;
};

#endif
