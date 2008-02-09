#include "Meta.h"
#include "Contact.h"
#include "List.h"

using namespace SIMContactList;

Meta::Meta(const QString &_name, PsiAccount *_pa, List *cl, Item *parent)
:Item(Item::TMeta, _pa, cl, parent), name_(_name)
{
}

Meta::~Meta()
{
}

QString Meta::name()
{
 	return name_;
}

XMPP::Jid Meta::jid()
{
	if (child(0))
		return (dynamic_cast<Contact*>(child(0)))->jid();
}

QPixmap Meta::state()
{
	if (child(0))
		return (dynamic_cast<Contact*>(child(0)))->state();
	else
		return QPixmap(16,16);
}

QPixmap Meta::pixmap()
{
	if (child(0))
		return (dynamic_cast<Contact*>(child(0)))->pixmap();
	else
		return QPixmap(16,16);
}

QPixmap Meta::avatar()
{
	if (child(0))
		return (dynamic_cast<Contact*>(child(0)))->avatar();
	else
		return QPixmap(16,16);
}

const QString &Meta::description()
{
	if (child(0))
		return (dynamic_cast<Contact*>(child(0)))->description();
	else
		return "";
}

const Name &Meta::contactName()
{
	if (child(0))
		return (dynamic_cast<Contact*>(child(0)))->contactName();
	else
		return Name(name(), textColor(), 0);
}

const QColor &Meta::textColor()
{
	if (child(0))
		return (dynamic_cast<Contact*>(child(0)))->textColor();
	else
		return QColor(0,0,0);
}

UserListItem *Meta::u()
{
	return (dynamic_cast<Contact*>(child(0)))->u();
}

XMPP::Status Meta::status()
{
	if (child(0))
		return (dynamic_cast<Contact*>(child(0)))->status();
	else
		return XMPP::Status(XMPP::Status::Offline);
}

void Meta::showContextMenu(const QPoint &p)
{
	if (child(0))
		return (dynamic_cast<Contact*>(child(0)))->showContextMenu(p);
}

QString Meta::toolTip()
{
	QString toolTip_;
	int i = 0;

	while(child(i)) {
		toolTip_ += (dynamic_cast<Contact*>(child(i)))->toolTip();
		i++;
	}

	if(i ==0 )
		return name();

	return toolTip_;
}

Item *Meta::updateParent()
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

