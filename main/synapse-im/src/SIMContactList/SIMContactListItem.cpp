#include "SIMContactListItem.h"
#include "SIMContactList.h"
#include "SIMContactListContact.h"
#include "SIMContactListMeta.h"
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
	
	if( this == contactList()->invisibleGroup() || this == contactList()->searchGroup() )
	{
		QList<SIMContactListItem*>::Iterator it = childItems.begin();
		while(it != childItems.end() && !inserted) {
			if (SIMContactListItem::compare_invisible(*it,item) >= 0) {
				childItems.insert(it,item);
				inserted = true;
			}
			it++;
		}

	} else {

		QList<SIMContactListItem*>::Iterator it = childItems.begin();
		while(it != childItems.end() && !inserted) {
			if (SIMContactListItem::compare(*it,item) >= 0) {
				childItems.insert(it,item);
				inserted = true;
			}
			it++;
		}
	}

	if (!inserted) {
		childItems.push_back(item);
	}
	updateParent(true);
}

void SIMContactListItem::removeChild(SIMContactListItem *item)
{
	childItems.removeAll(item);
	updateParent(true);
}

SIMContactListItem *SIMContactListItem::findItem(const QString &name, int _type)
{
	if (_type == SIMContactListItem::Group) {
		SIMContactListGroup *group = NULL;
		for(int i=0; i<size(); i++)
		{
			group = dynamic_cast<SIMContactListGroup*>(child(i));
			if(group && (group->name().compare(name) == 0))
				return child(i);
		}
		return NULL;
	} else if (_type == SIMContactListItem::Meta) {
		SIMContactListMeta *meta = NULL;
		for(int i=0; i<size(); i++)
		{
			meta = dynamic_cast<SIMContactListMeta*>(child(i));
			if(meta && (meta->name().compare(name) == 0))
				return child(i);
		}
		return NULL;
	}
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

		parentItem = parent;
	
		if (parent) {
			parent->appendChild(this);
		}
		
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

int SIMContactListItem::compare_invisible(SIMContactListItem *it1, SIMContactListItem *it2)
{
	// Contacts 	it2->do wstawienia
	//		it1->wstawiony
	if(it2->type() > it1->type())
		return 1;
	else
		return -1;
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
		SIMContactListMeta* it1_meta = dynamic_cast<SIMContactListMeta*>(it1);
		SIMContactListMeta* it2_meta = dynamic_cast<SIMContactListMeta*>(it2);

		QString it1_name = (it1_contact) ? it1_contact->name() : it1_meta->name();
		QString it2_name = (it2_contact) ? it2_contact->name() : it2_meta->name();
		
		if (!it1_name.isEmpty() && !it2_name.isEmpty()) {
			int it1_rank = rankStatus((it1_contact) ? it1_contact->status().type() : it1_meta->status().type());
			int it2_rank = rankStatus((it2_contact) ? it2_contact->status().type() : it2_meta->status().type());

			if (it2_contact && (it2_contact->parent()->type() == SIMContactListItem::Meta) && it2_contact->alerting())
				it2_rank == -1;

			if ((it1_contact && it2_contact) && it1_rank == it2_rank) {
				it1_rank = it1_contact->u()->jidPriority();
 				it2_rank = it2_contact->u()->jidPriority();
			}

			if (it1_rank > it2_rank)
				return 1;
			else if (it1_rank < it2_rank)
				return -1;
			else
				return it1_name.compare(it2_name, Qt::CaseInsensitive);
		}
		else if (!it1_name.isEmpty()) {
			return -1;
		}
		else if (!it2_name.isEmpty()) {
			return 1;
		}	
	} else {
		if(it2->type() > SIMContactListItem::Group)
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

void SIMContactListItem::updateParent(bool reload)
{
	SIMContactListItem *newParent = parent();

	if (type_ == Contact) {
				SIMContactListContact *contact;
				contact = dynamic_cast<SIMContactListContact*>(this);
				newParent = SIMContactListContact::updateParent(contact, contactList());
	} else if (type_ == Meta) {
				SIMContactListMeta *meta;
				meta = dynamic_cast<SIMContactListMeta*>(this);
				newParent = meta->updateParent();
	} else if (type_ == Group) {
				reload = false;
				SIMContactListGroup *group;
				group = dynamic_cast<SIMContactListGroup*>(this);
				newParent = group->updateParent();
	} else if (type_ == Account) {
				reload = false;
				SIMContactListAccount *account;
				account = dynamic_cast<SIMContactListAccount*>(this);
				newParent = account->updateParent();
	} else if (type_ == Root) {
				reload = false;
	}

	if (reload || newParent != parentItem)
		setParent(newParent);
}

void SIMContactListItem::updateParents()
{
	QList<SIMContactListItem*> items(childItems);
	for (int i = 0; i<items.size(); ++i)
		items.at(i)->updateParents();
	updateParent();
}

void SIMContactListItem::updateOptions()
{
	QList<SIMContactListItem*> items(childItems);
	for (int i = 0; i<items.size(); ++i)
		items.at(i)->updateOptions();
	if (type_ == SIMContactListItem::Contact)
		((SIMContactListContact* )this)->updateOptions();
}
