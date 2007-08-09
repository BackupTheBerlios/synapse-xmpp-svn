#include "SIMContactListContact.h"
#include "SIMContactListGroup.h"
#include "SIMContactList.h"
#include "SIMContactListView.h"
#include "SIMContactListModel.h"

#include "iconset.h"
#include "psiaccount.h"

#include <QAction>
#include <QMenu>
#include <QModelIndex>

SIMContactListGroup::SIMContactListGroup(const QString &_name, PsiAccount *_pa, SIMContactList *cl, SIMContactListItem *parent)
:SIMContactListItem(SIMContactListItem::Group, _pa, cl, parent), group_name(_name)
{
}

SIMContactListGroup::~SIMContactListGroup()
{
}

const QString &SIMContactListGroup::name()
{
	return group_name;
}

const QPixmap &SIMContactListGroup::pixmap()
{
	if(true)
		return IconsetFactory::iconPtr("synapse-im/groupOpen")->pixmap();
	else
		return IconsetFactory::iconPtr("synapse-im/groupClosed")->pixmap();
}

SIMContactListContact *SIMContactListGroup::findEntry(const QString &j)
{
	SIMContactListContact *clc;
	for(int i=0; i<size(); i++)
	{
		if ( clc = (SIMContactListContact*) child(i)) {
			if(clc->jid().bare().compare(j) == 0)
				return clc;
		}
	}
	return NULL;
}

void SIMContactListGroup::showContextMenu(const QPoint& p)
{
	bool online = account()->loggedIn();

	QAction *rename = NULL;

	QMenu pm(0);

	if (online)
		rename = pm.addAction(IconsetFactory::icon("psi/edit/clear").icon(), SIMContactList::tr("Re&name"));

	QAction *ret = pm.exec(p);

	if (ret == rename) {
		QModelIndex m = ((SIMContactListModel*)contactList()->contactListView()->model())->index(row(), SIMContactListModel::NameColumn, this);
		contactList()->contactListView()->edit(m);
	}
}

QString SIMContactListGroup::toolTip()
{
	return QString("<b>") + name() + QString("</b>");
}

SIMContactListItem *SIMContactListGroup::updateParent()
{
	SIMContactListItem *newParent = parent();

	if (size() == 0) {
		if(contactList()->search().isEmpty())
			newParent = contactList()->invisibleGroup();
		else
			newParent = contactList()->searchGroup();
	} else {
		newParent = defaultParent();
	}

	return newParent;
}