#include "Contact.h"
#include "Group.h"
#include "List.h"
#include "View.h"
#include "Model.h"

#include "iconset.h"
#include "psiaccount.h"

#include <QAction>
#include <QMenu>
#include <QModelIndex>

using namespace SIMContactList;

Group::Group(const QString &_name, PsiAccount *_pa, List *cl, Item *parent)
:Item(Item::TGroup, _pa, cl, parent), group_name(_name)
{
}

Group::~Group()
{
}

const QString &Group::name()
{
	return group_name;
}

const QPixmap &Group::pixmap()
{
	if(true)
		return IconsetFactory::iconPtr("synapse-im/groupOpen")->pixmap();
	else
		return IconsetFactory::iconPtr("synapse-im/groupClosed")->pixmap();
}

Contact *Group::findEntry(const QString &j)
{
	Contact *clc;
	for(int i=0; i<size(); i++)
	{
		if ( clc = (Contact*) child(i)) {
			if(clc->jid().bare().compare(j) == 0)
				return clc;
		}
	}
	return NULL;
}

void Group::showContextMenu(const QPoint& p)
{
	bool online = account()->loggedIn();

	QAction *rename = NULL;

	QMenu pm(0);

	if (online)
		rename = pm.addAction(IconsetFactory::icon("psi/edit/clear").icon(), List::tr("Re&name"));

	QAction *ret = pm.exec(p);

	if (ret == rename) {
		QModelIndex m = ((Model*)contactList()->contactListView()->model())->index(row(), Model::NameColumn, this);
		contactList()->contactListView()->edit(m);
	}
}

QString Group::toolTip()
{
	return QString("<b>") + name() + QString("</b>");
}

Item *Group::updateParent()
{
	Item *newParent = parent();

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