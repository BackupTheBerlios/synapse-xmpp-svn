#include "SIMContactListItem.h"
#include "SIMContactListContact.h"
#include "SIMContactListMeta.h"
#include "SIMContactListAccount.h"
#include "SIMContactListGroup.h"
#include "SIMContactList.h"
#include "SIMContactListModel.h"
#include "SIMContactListView.h"

#include "psiaccount.h"
#include "xmpp_status.h"
#include "iconset.h"
#include "psiiconset.h"

SIMContactListAccount::SIMContactListAccount(PsiAccount *_pa, SIMContactList *cl, SIMContactListItem *parent)
:SIMContactListItem(SIMContactListItem::Account, _pa, cl, parent)
{
	state_ = 0;
}

SIMContactListAccount::~SIMContactListAccount()
{
}

const QString &SIMContactListAccount::name()
{
	return account()->name();
}

const QPixmap &SIMContactListAccount::pixmap()
{
	if(state_ != -1)
		return PsiIconset::instance()->statusPtr(state_)->pixmap();
	else
		return IconsetFactory::iconPtr("psi/connect")->pixmap();
}

void SIMContactListAccount::setState(int _state)
{
	state_ = _state;
}

SIMContactListGroup *SIMContactListAccount::ensureGroup(const QString &group_name)
{
	SIMContactListGroup *group = NULL;
	if(contactList()->showAccounts()) {
		group = (SIMContactListGroup *)findItem(group_name, SIMContactListItem::Group);
		if(group)
			return group;
		group = new SIMContactListGroup(group_name, account(), contactList(), this);
		appendChild(group);
	} else {
		group = (SIMContactListGroup *)contactList()->findItem(group_name, SIMContactListItem::Group);
		if(group)
			return group;
		group = new SIMContactListGroup(group_name, account(), contactList(), contactList()->rootItem());
		contactList()->rootItem()->appendChild(group);
	}

	return group;
}

SIMContactListMeta *SIMContactListAccount::ensureMeta(const QString &meta_name, SIMContactListGroup *group)
{
	SIMContactListMeta *meta = NULL;
	if(contactList()->showAccounts()) {
		meta = (SIMContactListMeta *)findItem(meta_name, SIMContactListItem::Meta);
		if(meta)
			return meta;
		meta = new SIMContactListMeta(meta_name, account(), contactList(), group);
	} else {
		meta = (SIMContactListMeta *)contactList()->findItem(meta_name, SIMContactListItem::Meta);
		if(meta)
			return meta;
		meta = new SIMContactListMeta(meta_name, account(), contactList(), group);
	//	contactList()->rootItem()->appendChild(group);
	}
/*	printf("group_size : %d\n", group->size());
	for(int i=0; i<group->size(); i++)
	{
		meta = NULL;
		if ((meta = dynamic_cast<SIMContactListMeta*>(group->child(i)))) {
			printf("meta\n");
			if (meta->name().compare(meta_name) == 0)
				return meta;
		}
	}
	if(meta == NULL) {
	for(int i=0; i<contactList()->invisibleGroup()->size(); i++)
	{
		meta = NULL;
		if ((meta = dynamic_cast<SIMContactListMeta*>(contactList()->invisibleGroup()->child(i)))) {
			printf("meta\n");
			if (meta->name().compare(meta_name) == 0)
				return meta;
		}
	}
	}

	printf("not found %s\n", meta_name.toAscii().data());
	meta = new SIMContactListMeta(meta_name, account(), contactList(), group);*/
	return meta;
}

void SIMContactListAccount::setAlert(const Jid &j, PsiIcon *icon)
{
	SIMContactListContact *clc = contactList()->findEntry(j.bare(), false);
	if (clc) {
		clc->setAlertIcon(icon);
		SIMContactListItem *parent = clc->parent();
		clc->updateParents();
		if(clc->parent() != parent) {
			parent->updateParent();
			parent = clc->parent();
			parent->updateParent();
		} else if (clc->parent()->type() == SIMContactListItem::Meta) {
			parent->removeChild(clc);
			parent->appendChild(clc);
		}
	}
}

void SIMContactListAccount::updateEntry(const UserListItem &u)
{
	if(!account()->enabled())
		return;

	SIMContactListContact *clc = contactList()->findEntry(u.jid().bare(), u.isSelf());
	if(clc) {
		if ((clc->u()->groups() == u.groups() && clc->u()->metas() == u.metas()) || u.isSelf() || !clc->u()->inList()) {
			clc->setUserListItem(u);
			SIMContactListItem *parent = clc->parent();
			clc->updateParents();
			if(clc->parent() != parent) {
				parent->updateParent();
				parent = clc->parent();
				parent->updateParent();
			} else {
				parent->removeChild(clc);
				parent->appendChild(clc);
			}
			if(parent->type() == SIMContactListItem::Meta) {
				SIMContactListItem *item = parent;
				parent = item->parent();
				parent->removeChild(item);
				parent->appendChild(item);
			}
			contactList()->dataChanged();
			return;
		} else {
			SIMContactListItem *parent = clc->parent();
			parent->removeChild(clc);
			delete clc;
			clc = NULL;
		}
	}

	if(u.isSelf()) {
		if(contactList()->showAccounts()) {
			clc = new SIMContactListContact(u, account(), contactList(), this);
			appendChild(clc);
		} else {
			clc = new SIMContactListContact(u, account(), contactList(), contactList()->rootItem());
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
		SIMContactListGroup *group = ensureGroup(group_name);
		QStringList metas = u.metas();
		if(metas.isEmpty()) {
			clc = new SIMContactListContact(u, account(), contactList(), group);
			group->appendChild(clc);
		} else {
			SIMContactListMeta *meta = ensureMeta(metas.takeFirst(), group);
			clc = new SIMContactListContact(u, account(), contactList(), meta);
			meta->appendChild(clc);
			meta->updateParents();
		}
		group->updateParents();
		contactList()->dataChanged();
	}
}

void SIMContactListAccount::removeEntry(const Jid &j)
{
	SIMContactListContact *clc = contactList()->findEntry(j.bare());
	if(clc) {
		SIMContactListItem *group = clc->parent();
		group->removeChild(clc);
		delete clc;
		contactList()->dataChanged();
	}
}

void SIMContactListAccount::showContextMenu(const QPoint& p)
{
}

QString SIMContactListAccount::toolTip()
{
	return QString("<b>") + name() + QString("</b>");
}

SIMContactListItem *SIMContactListAccount::updateParent()
{
	SIMContactListItem *newParent = parent();

	if (size() == 0 || !account()->enabled())
		newParent = contactList()->invisibleGroup();
	else
		newParent = defaultParent();

	return newParent;
}
