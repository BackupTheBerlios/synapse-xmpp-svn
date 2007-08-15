#include "SIMContactListItem.h"
#include "SIMContactList.h"
#include "SIMContactListContact.h"
#include "SIMContactListGroup.h"
#include "psiaccount.h"
#include "psicon.h"
#include "xmpp_status.h"

using namespace XMPP;

static inline int rankStatus(int status) 
{
	switch (status) {
		case Status::FFC : return 0;
		case Status::Online : return 1;
		case Status::Away : return 2;
		case Status::XA : return 3;
		case Status::DND : return 4;
		case Status::Invisible : return 5;
		default:
			return 6;
	}
	return 0;
}

SIMContactListItem::SIMContactListItem(int _type, PsiAccount *_pa, SIMContactList *cl, SIMContactListItem *parent)
:type_(_type), pa(_pa), contactList_(cl)
{
	parentItem = parent;
	defaultParent_ = parent;
	childItems.clear();
}

SIMContactListItem::~SIMContactListItem()
{
	qDeleteAll(childItems);
}

PsiAccount *SIMContactListItem::account()
{
	return pa;
}

void SIMContactListItem::appendChild(SIMContactListItem *item)
{
	bool inserted = false;
		
	QList<SIMContactListItem*>::Iterator it = childItems.begin();
	while(it != childItems.end() && !inserted) {
		if (SIMContactListItem::compare(*it,item) >= 0) {
			childItems.insert(it,item);
			inserted = true;
		}
		it++;
	}

	if (!inserted) {
		childItems.push_back(item);
	}
}

void SIMContactListItem::removeChild(SIMContactListItem *item)
{
	childItems.removeAll(item);
}

SIMContactListGroup *SIMContactListItem::findGroup(const QString &group_name)
{
	SIMContactListGroup *group = NULL;
	for(int i=0; i<size(); i++)
	{
		group = dynamic_cast<SIMContactListGroup*>(child(i));
		if(group && (group->name().compare(group_name) == 0))
			return (SIMContactListGroup*)child(i);
	}
	return NULL;
}

SIMContactListContact *SIMContactListItem::findEntry(const QString &j, bool self)
{
	SIMContactListContact *contact = NULL;

	if(self && ((type() != Root) && (type() != Account)))
		return NULL;

	if(contact = dynamic_cast<SIMContactListContact*>(this))
		if(j.compare(contact->jid().bare()) == 0)
			return contact;

	contact = NULL;
	for(int i=0; i<childItems.size(); i++) {
		contact = childItems.at(i)->findEntry(j);
		if(contact)
			return contact;
	}
	return NULL;
}

SIMContactListItem *SIMContactListItem::child(int row)
{
	return childItems.value(row);
}

void SIMContactListItem::setDefaultParent(SIMContactListItem* parent)
{
	defaultParent_ = parent;
}

void SIMContactListItem::setParent(SIMContactListItem* parent)
{
	if (parentItem != parent) {
		if (parentItem) {
			parentItem->removeChild(this);
		}
	
		if (parent) {
			parent->appendChild(this);
		}
		
		parentItem = parent;
		contactList_->dataChanged();
	}
}

int SIMContactListItem::type()
{
	return type_;
}

int SIMContactListItem::size() const
{
	return childItems.size();
}

int SIMContactListItem::row() const
{
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<SIMContactListItem*>(this));

	return 0;
}

int SIMContactListItem::columnCount() const
{
	return childItems.count();
}

const QVariant &SIMContactListItem::data(int column) const
{
//	return itemData.value(column);
}

SIMContactListItem *SIMContactListItem::parent()
{
	return parentItem;
}

SIMContactList *SIMContactListItem::contactList()
{
	return contactList_;
}

SIMContactListItem *SIMContactListItem::defaultParent()
{
	return defaultParent_;
}


void SIMContactListItem::showContextMenu(const QPoint&)
{
	
}

int SIMContactListItem::compare(SIMContactListItem *it1, SIMContactListItem *it2)
{
	// Contacts 	it2->do wstawienia
	//		it1->wstawiony
	if(it1->type() > SIMContactListItem::Group) {
		if(it2->type() == SIMContactListItem::Group)
			return -1;

		SIMContactListContact* it1_contact = dynamic_cast<SIMContactListContact*>(it1);
		SIMContactListContact* it2_contact = dynamic_cast<SIMContactListContact*>(it2);
		if (it1_contact && it2_contact) {
			if (rankStatus(it1_contact->status().type()) > rankStatus(it2_contact->status().type()))
				return 1;
			if (rankStatus(it1_contact->status().type()) < rankStatus(it2_contact->status().type()))
				return -1;
			if (it1_contact->name() < it2_contact->name()) {
				return -1;
			}
			else if (it1_contact->name() > it2_contact->name()) {
				return 1;
			}
			else {
				return 0;
			}
		}
		else if (it1_contact) {
			return -1;
		}
		else if (it2_contact) {
			return 1;
		}	
	}else {
		if(it2->type() == SIMContactListItem::Contact)
			return 1;

		SIMContactListGroup* it1_group = dynamic_cast<SIMContactListGroup*>(it1);
		SIMContactListGroup* it2_group = dynamic_cast<SIMContactListGroup*>(it2);
		if (it1_group && it2_group) {
			if (it1_group->name() == QObject::tr("General") || it2_group->name() == QObject::tr("Agents/Transports") || it2_group->name() == QObject::tr("Not in list"))
				return -1;
			if (it2_group->name() == QObject::tr("General") || it1_group->name() == QObject::tr("Agents/Transports") ||
			it1_group->name() == QObject::tr("Not in list"))
				return 1;

			if (it1_group->name() < it2_group->name()) {
				return -1;
			}
			else if (it1_group->name() > it2_group->name()) {
				return 1;
			}
			else {
				return 0;
			}
		}
		else if (it1_group) {
			return -1;
		}
		else if (it2_group) {
			return 1;
		}
	}
}

void SIMContactListItem::updateParent()
{
	SIMContactListItem *newParent = parent();

	if (type_ == Contact) {
				SIMContactListContact *contact;
				contact = dynamic_cast<SIMContactListContact*>(this);
//				newParent = contact->updateParent();
				newParent = SIMContactListContact::updateParent(contact, contactList());
	} else if (type_ == Group) {
				SIMContactListGroup *group;
				group = dynamic_cast<SIMContactListGroup*>(this);
				newParent = group->updateParent();
	} else if (type_ == Account) {
				SIMContactListAccount *account;
				account = dynamic_cast<SIMContactListAccount*>(this);
				newParent = account->updateParent();
	}

	if (newParent != parentItem)
		setParent(newParent);
}

void SIMContactListItem::updateParents()
{
	QList<SIMContactListItem*> items(childItems);
	for (int i = 0; i<items.size(); ++i)
		items.at(i)->updateParents();
	updateParent();
}

