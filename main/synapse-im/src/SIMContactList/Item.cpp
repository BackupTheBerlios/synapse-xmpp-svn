#include "Item.h"
#include "List.h"
#include "Contact.h"
#include "Meta.h"
#include "Group.h"
#include "psiaccount.h"
#include "psicon.h"
#include "xmpp_status.h"

using namespace XMPP;

using namespace SIMContactList;

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

Item::Item(int _type, PsiAccount *_pa, List *cl, Item *parent)
:type_(_type), pa(_pa), contactList_(cl)
{
	parentItem = parent;
	defaultParent_ = parent;
	childItems.clear();
}

Item::~Item()
{
	qDeleteAll(childItems);
}

PsiAccount *Item::account()
{
	return pa;
}

void Item::appendChild(Item *item)
{
	bool inserted = false;
	
	if( this == contactList()->invisibleGroup() || this == contactList()->searchGroup() )
	{
		QList<Item*>::Iterator it = childItems.begin();
		while(it != childItems.end() && !inserted) {
			if (Item::compare_invisible(*it,item) >= 0) {
				childItems.insert(it,item);
				inserted = true;
			}
			it++;
		}

	} else {

		QList<Item*>::Iterator it = childItems.begin();
		while(it != childItems.end() && !inserted) {
			if (Item::compare(*it,item) >= 0) {
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

void Item::removeChild(Item *item)
{
	childItems.removeAll(item);
	updateParent(true);
}

Item *Item::findItem(const QString &name, int _type)
{
	if (_type == Item::TGroup) {
		Group *group = NULL;
		for(int i=0; i<size(); i++)
		{
			group = dynamic_cast<Group*>(child(i));
			if(group && (group->name().compare(name) == 0))
				return child(i);
		}
		return NULL;
	} else if (_type == Item::TMeta) {
		Meta *meta = NULL;
		for(int i=0; i<size(); i++)
		{
			meta = dynamic_cast<Meta*>(child(i));
			if(meta && (meta->name().compare(name) == 0))
				return child(i);
		}
		return NULL;
	}
}

Contact *Item::findEntry(const QString &j, bool self)
{
	Contact *contact = NULL;

	if(self && ((type() != TRoot) && (type() != TAccount)))
		return NULL;

	if(contact = dynamic_cast<Contact*>(this))
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

Item *Item::child(int row)
{
	return childItems.value(row);
}

void Item::setDefaultParent(Item* parent)
{
	defaultParent_ = parent;
}

void Item::setParent(Item* parent)
{
//	if (parentItem != parent) {
		if (parentItem) {
			parentItem->removeChild(this);
		}

		parentItem = parent;
	
		if (parent) {
			parent->appendChild(this);
		}
		
		contactList_->dataChanged();
//	}
}

int Item::type()
{
	return type_;
}

int Item::size() const
{
	return childItems.size();
}

int Item::row() const
{
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<Item*>(this));

	return 0;
}

int Item::columnCount() const
{
	return childItems.count();
}

const QVariant &Item::data(int column) const
{
//	return itemData.value(column);
}

Item *Item::parent()
{
	return parentItem;
}

List *Item::contactList()
{
	return contactList_;
}

Item *Item::defaultParent()
{
	return defaultParent_;
}


void Item::showContextMenu(const QPoint&)
{
	
}

int Item::compare_invisible(Item *it1, Item *it2)
{
	// Contacts 	it2->do wstawienia
	//		it1->wstawiony
	if(it2->type() > it1->type())
		return 1;
	else
		return -1;
}

int Item::compare(Item *it1, Item *it2)
{
	// Contacts 	it2->do wstawienia
	//		it1->wstawiony
	if(it1->type() > Item::TGroup) {
		if(it2->type() == Item::TGroup)
			return -1;

		Contact* it1_contact = dynamic_cast<Contact*>(it1);
		Contact* it2_contact = dynamic_cast<Contact*>(it2);
		Meta* it1_meta = dynamic_cast<Meta*>(it1);
		Meta* it2_meta = dynamic_cast<Meta*>(it2);

		QString it1_name = (it1_contact) ? it1_contact->name() : it1_meta->name();
		QString it2_name = (it2_contact) ? it2_contact->name() : it2_meta->name();
		
		if (!it1_name.isEmpty() && !it2_name.isEmpty()) {
			int it1_rank = rankStatus((it1_contact) ? it1_contact->status().type() : it1_meta->status().type());
			int it2_rank = rankStatus((it2_contact) ? it2_contact->status().type() : it2_meta->status().type());

			if (it2_contact && (it2_contact->parent()->type() == Item::TMeta) && it2_contact->alerting())
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
		if(it2->type() > Item::TGroup)
			return 1;

		Group* it1_group = dynamic_cast<Group*>(it1);
		Group* it2_group = dynamic_cast<Group*>(it2);
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

void Item::updateParent(bool reload)
{
	Item *newParent = parent();

	if (type_ == TContact) {
				Contact *contact;
				contact = dynamic_cast<Contact*>(this);
				newParent = Contact::updateParent(contact, contactList());
	} else if (type_ == TMeta) {
				Meta *meta;
				meta = dynamic_cast<Meta*>(this);
				newParent = meta->updateParent();
	} else if (type_ == TGroup) {
				reload = false;
				Group *group;
				group = dynamic_cast<Group*>(this);
				newParent = group->updateParent();
	} else if (type_ == TAccount) {
				reload = false;
				Account *account;
				account = dynamic_cast<Account*>(this);
				newParent = account->updateParent();
	} else if (type_ == TRoot) {
				reload = false;
	}

	if (reload || newParent != parentItem)
		setParent(newParent);
}

void Item::updateParents()
{
	QList<Item*> items(childItems);
	for (int i = 0; i<items.size(); ++i)
		items.at(i)->updateParents();
	updateParent();
}

void Item::updateOptions()
{
	QList<Item*> items(childItems);
	for (int i = 0; i<items.size(); ++i)
		items.at(i)->updateOptions();
	if (type_ == Item::TContact)
		((Contact* )this)->updateOptions();
}
