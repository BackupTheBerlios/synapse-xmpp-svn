#include "Item.h"
#include "Contact.h"
#include "Meta.h"
#include "Account.h"
#include "Group.h"
#include "List.h"
#include "Model.h"
#include "View.h"

#include "psiaccount.h"
#include "xmpp_status.h"
#include "iconset.h"
#include "psiiconset.h"

using namespace SIMContactList;

Account::Account(PsiAccount *_pa, List *cl, Item *parent)
:Item(Item::TAccount, _pa, cl, parent)
{
	state_ = 0;
}

Account::~Account()
{
}

const QString &Account::name()
{
	return account()->name();
}

const QPixmap &Account::pixmap()
{
	if(state_ != -1)
		return PsiIconset::instance()->statusPtr(state_)->pixmap();
	else
		return IconsetFactory::iconPtr("psi/connect")->pixmap();
}

void Account::setState(int _state)
{
	state_ = _state;
}

Group *Account::ensureGroup(const QString &group_name)
{
	Group *group = NULL;
	if(contactList()->showAccounts()) {
		group = (Group *)findItem(group_name, Item::TGroup);
		if(group)
			return group;
		group = new Group(group_name, account(), contactList(), this);
		appendChild(group);
	} else {
		group = (Group *)contactList()->findItem(group_name, Item::TGroup);
		if(group)
			return group;
		group = new Group(group_name, account(), contactList(), contactList()->rootItem());
		contactList()->rootItem()->appendChild(group);
	}

	return group;
}

Meta *Account::ensureMeta(const QString &meta_name, Group *group)
{
	Meta *meta = NULL;

	meta = (Meta *)group->findItem(meta_name, Item::TMeta);
	if(meta)
		return meta;

	meta = (Meta *)contactList()->invisibleGroup()->findItem(meta_name, Item::TMeta);
	if(meta)
		return meta;

	meta = (Meta *)contactList()->searchGroup()->findItem(meta_name, Item::TMeta);
	if(meta)
		return meta;

	meta = new Meta(meta_name, account(), contactList(), group);
	group->appendChild(meta);

	return meta;
}

void Account::setAlert(const Jid &j, PsiIcon *icon)
{
	Contact *clc = contactList()->findEntry(j.bare(), false);
	if (clc) {
		clc->setAlertIcon(icon);
		Item *parent = clc->parent();
		clc->updateParents();
		if(clc->parent() != parent) {
			parent->updateParent();
			parent = clc->parent();
			parent->updateParent();
		} else {
			((Item *)clc)->updateParent(true);
			/*if (clc->parent()->type() == SIMContactListItem::Meta) {
			parent->removeChild(clc);
			parent->appendChild(clc);*/
		}
	}
}

void Account::updateEntry(const UserListItem &u)
{
	if(!account()->enabled())
		return;

	Contact *clc = contactList()->findEntry(u.jid().bare(), u.isSelf());
	if(clc) {
		if ((clc->u()->groups() == u.groups() && clc->u()->metas() == u.metas()) || u.isSelf() || !clc->u()->inList()) {
			clc->setUserListItem(u);
			Item *parent = clc->parent();
			((Item *)clc)->updateParent(true);
			contactList()->dataChanged();
			return;
		} else {
			Item *parent = clc->parent();
			parent->removeChild(clc);
			delete clc;
			clc = NULL;
		}
	}

	if(u.isSelf()) {
		if(contactList()->showAccounts()) {
			clc = new Contact(u, account(), contactList(), this);
			appendChild(clc);
		} else {
			clc = new Contact(u, account(), contactList(), contactList()->rootItem());
			contactList()->rootItem()->appendChild(clc);
		}
	} else {
		QStringList groups = u.groups();
		QString group_name;
		if(!groups.isEmpty())
			group_name = groups.takeFirst();
		else if (u.isTransport())
			group_name = QObject::tr("Agents/Transports");
		else if (u.inList())
			group_name = QObject::tr("General");
		else
			group_name = QObject::tr("Not in list");
		Group *group = ensureGroup(group_name);
		QStringList metas = u.metas();
		if(metas.isEmpty()) {
			clc = new Contact(u, account(), contactList(), group);
			group->appendChild(clc);
		} else {
			Meta *meta = ensureMeta(metas.takeFirst(), group);
			clc = new Contact(u, account(), contactList(), meta);
			meta->appendChild(clc);
		}
		((Item *)clc)->updateParent(true);
		contactList()->dataChanged();
	}
}

void Account::removeEntry(const Jid &j)
{
	Contact *clc = contactList()->findEntry(j.bare());
	if(clc) {
		Item *group = clc->parent();
		group->removeChild(clc);
		delete clc;
		contactList()->dataChanged();
	}
}

void Account::showContextMenu(const QPoint& p)
{
}

QString Account::toolTip()
{
	return QString("<b>") + name() + QString("</b>");
}

Item *Account::updateParent()
{
	Item *newParent = parent();

	if (size() == 0 || !account()->enabled())
		newParent = contactList()->invisibleGroup();
	else
		newParent = defaultParent();

	return newParent;
}
