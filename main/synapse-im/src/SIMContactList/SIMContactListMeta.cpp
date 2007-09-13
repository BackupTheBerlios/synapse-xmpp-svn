#include "SIMContactListMeta.h"
#include "SIMContactListContact.h"
#include "SIMContactList.h"

SIMContactListMeta::SIMContactListMeta(const QString &_name, PsiAccount *_pa, SIMContactList *cl, SIMContactListItem *parent)
:SIMContactListItem(SIMContactListItem::Meta, _pa, cl, parent), name_(_name)
{
}

SIMContactListMeta::~SIMContactListMeta()
{
}

QString SIMContactListMeta::name()
{
 	return name_;
}

XMPP::Jid SIMContactListMeta::jid()
{
	return (dynamic_cast<SIMContactListContact*>(child(0)))->jid();
}

QPixmap SIMContactListMeta::state()
{
	if (child(0))
		return (dynamic_cast<SIMContactListContact*>(child(0)))->state();
	else
		return QPixmap(16,16);
}

QPixmap SIMContactListMeta::pixmap()
{
	if (child(0))
		return (dynamic_cast<SIMContactListContact*>(child(0)))->pixmap();
	else
		return QPixmap(16,16);
}

QPixmap SIMContactListMeta::avatar()
{
	if (child(0))
		return (dynamic_cast<SIMContactListContact*>(child(0)))->avatar();
	else
		return QPixmap(16,16);
}

const QString &SIMContactListMeta::description()
{
	if (child(0))
		return (dynamic_cast<SIMContactListContact*>(child(0)))->description();
	else
		return "";
}

const SIMContactName &SIMContactListMeta::contactName()
{
	if (child(0))
		return (dynamic_cast<SIMContactListContact*>(child(0)))->contactName();
	else
		return SIMContactName(name(), textColor());
}

const QColor &SIMContactListMeta::textColor()
{
	if (child(0))
		return (dynamic_cast<SIMContactListContact*>(child(0)))->textColor();
	else
		return QColor(0,0,0);
}

UserListItem *SIMContactListMeta::u()
{
	return (dynamic_cast<SIMContactListContact*>(child(0)))->u();
}

XMPP::Status SIMContactListMeta::status()
{
	if (child(0))
		return (dynamic_cast<SIMContactListContact*>(child(0)))->status();
	else
		return XMPP::Status(XMPP::Status::Offline);
}

void SIMContactListMeta::showContextMenu(const QPoint &p)
{
	if (child(0))
		return (dynamic_cast<SIMContactListContact*>(child(0)))->showContextMenu(p);
}

QString SIMContactListMeta::toolTip()
{
	QString toolTip_;
	int i = 0;

	while(child(i)) {
		toolTip_ += (dynamic_cast<SIMContactListContact*>(child(i)))->toolTip();
		i++;
	}

	if(i ==0 )
		return name();

	return toolTip_;
}

SIMContactListItem *SIMContactListMeta::updateParent()
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

