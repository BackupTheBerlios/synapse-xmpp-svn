#include "SIMContactListItem.h"
#include "SIMContactListContact.h"
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
		group = findGroup(group_name);
		if(group)
			return group;
		group = new SIMContactListGroup(group_name, account(), contactList(), this);
		appendChild(group);
	} else {
		group = contactList()->findGroup(group_name);
		if(group)
			return group;
		group = new SIMContactListGroup(group_name, account(), contactList(), contactList()->rootItem());
		contactList()->rootItem()->appendChild(group);
	}
	//contactList()->dataChanged();
	return group;
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
		}
	}
}

void SIMContactListAccount::updateEntry(const UserListItem &u)
{
	if(!account()->enabled())
		return;
	printf("jid = %s\n", u.jid().bare().ascii());
	SIMContactListContact *clc = contactList()->findEntry(u.jid().bare(), u.isSelf());
	if(clc) {
		printf("clc found\n");
		if (clc->u()->groups() == u.groups() || u.isSelf()) {
			printf("update\n");
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
			contactList()->dataChanged();
			return;
		} else {
			printf("remove\n");
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
		clc = new SIMContactListContact(u, account(), contactList(), group);
		group->appendChild(clc);
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