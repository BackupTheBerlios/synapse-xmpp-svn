#include "SIMContactListModel.h"

#include "SIMContactList.h"
#include "SIMContactListItem.h"
#include "SIMContactListAccount.h"
#include "SIMContactListGroup.h"
#include "SIMContactListMeta.h"
#include "SIMContactListContact.h"
#include "SIMContactListView.h"
#include "SIMContactName.h"

#include <QTextDocument>

#include "common.h"
#include "xmpp_status.h"
#include "psitooltip.h"
#include "psiaccount.h"
#include "psicon.h"
#include "psicontactlist.h"
#include "psioptions.h"

#define COLUMNS 4

using namespace XMPP;

SIMContactListModel::SIMContactListModel(SIMContactList *contactList) : contactList_(contactList)
{
	connect(contactList_,SIGNAL(s_dataChanged()),this,SLOT(contactList_changed()));
}

QVariant SIMContactListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	SIMContactListAccount *account;
	SIMContactListGroup *group;
	SIMContactListMeta *meta;
	SIMContactListContact *contact;

	SIMContactListItem* item = static_cast<SIMContactListItem*>(index.internalPointer());
	if (role == Qt::DisplayRole && index.column() == NameColumn) {
		if (( contact = dynamic_cast<SIMContactListContact*>(item))) {
			return qVariantFromValue(contact->contactName());
		} else if ((meta = dynamic_cast<SIMContactListMeta*>(item))) {
			return qVariantFromValue(meta->contactName());
		} else if ((group = dynamic_cast<SIMContactListGroup*>(item))) {
			return qVariantFromValue(group->name());
		} else if ((account = dynamic_cast<SIMContactListAccount*>(item))) {
			return qVariantFromValue(account->name());
		}
	} else if (role == Qt::DecorationRole && index.column() == StateColumn) {
		if (( contact = dynamic_cast<SIMContactListContact*>(item))) {
			return qVariantFromValue(contact->state());
		} else if (( meta = dynamic_cast<SIMContactListMeta*>(item))) {
			return qVariantFromValue(meta->state());
		} else if ((group = dynamic_cast<SIMContactListGroup*>(item))) {
			return qVariantFromValue(group->pixmap());
		} else if ((account = dynamic_cast<SIMContactListAccount*>(item))) {
			return qVariantFromValue(account->pixmap());
		}
	} else if (role == Qt::DecorationRole && index.column() == PixmapColumn) {
		if (( contact = dynamic_cast<SIMContactListContact*>(item))) {
			return qVariantFromValue(contact->pixmap());
		} else if (( meta = dynamic_cast<SIMContactListMeta*>(item))) {
			return qVariantFromValue(meta->pixmap());
		} else if ((group = dynamic_cast<SIMContactListGroup*>(item))) {
			return qVariantFromValue(group->pixmap());
		} else if ((account = dynamic_cast<SIMContactListAccount*>(item))) {
			return qVariantFromValue(account->pixmap());
		}
	} else if (role == Qt::DecorationRole && index.column() == AvatarColumn) {
		if (( contact = dynamic_cast<SIMContactListContact*>(item))) {
			return qVariantFromValue(contact->avatar());
		} else if (( meta = dynamic_cast<SIMContactListMeta*>(item))) {
			return qVariantFromValue(meta->avatar());
		}
	}
	else if (role == Qt::BackgroundColorRole) {
		if ((contact = dynamic_cast<SIMContactListContact*>(item))) {
			return qVariantFromValue(PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.background").value<QColor>());
		} else if ((meta = dynamic_cast<SIMContactListMeta*>(item))) {
			return qVariantFromValue(PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.background").value<QColor>());
		} else if ((group = dynamic_cast<SIMContactListGroup*>(item))) {
			return qVariantFromValue(PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.grouping.header-background").value<QColor>());
		} else if ((account = dynamic_cast<SIMContactListAccount*>(item))) {
			return qVariantFromValue(PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.profile.header-background").value<QColor>());
		}
	}
	else if (role == Qt::TextColorRole) {
		if ((contact = dynamic_cast<SIMContactListContact*>(item))) {
			return qVariantFromValue(contact->textColor());
		}
		else if ((meta = dynamic_cast<SIMContactListMeta*>(item))) {
			return qVariantFromValue(meta->textColor());
		}
		else if ((group = dynamic_cast<SIMContactListGroup*>(item))) {
			return qVariantFromValue(PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.grouping.header-foreground").value<QColor>());
		}
		else if ((account = dynamic_cast<SIMContactListAccount*>(item))) {
			return qVariantFromValue(PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.profile.header-foreground").value<QColor>());
		}
	}
	else if (role == Qt::FontRole) {
		return qVariantFromValue(PsiOptions::instance()->getOption("options.ui.look.font.contactlist").value<QFont>());
	}
	else if (role == Qt::EditRole) {
		if (( contact = dynamic_cast<SIMContactListContact*>(item))) {
			return qVariantFromValue(contact->name());
		} else if (( meta = dynamic_cast<SIMContactListMeta*>(item))) {
			return qVariantFromValue(meta->name());
		} else if ((group = dynamic_cast<SIMContactListGroup*>(item))) {
			return qVariantFromValue(group->name());
		}		
	}
	else if (role == ExpandedRole) {
		if ((contact = dynamic_cast<SIMContactListContact*>(item))) {
			return QVariant(false);
		}
		else {
			return QVariant(true);
		}
	} 
	return QVariant();
}

bool SIMContactListModel::setData(const QModelIndex& index, const QVariant& data, int role)
{
	if (!index.isValid())
		return false;

	SIMContactListAccount *account;
	SIMContactListGroup *group;
	SIMContactListMeta *meta;
	SIMContactListContact *contact;

	SIMContactListItem* item = static_cast<SIMContactListItem*>(index.internalPointer());
	if (role == ContextMenuRole) {
		if (( contact = dynamic_cast<SIMContactListContact*>(item))) {
			contact->showContextMenu(data.toPoint());
		} else if (( meta = dynamic_cast<SIMContactListMeta*>(item))) {
			meta->showContextMenu(data.toPoint());
		} else if ((group = dynamic_cast<SIMContactListGroup*>(item))) {
			group->showContextMenu(data.toPoint());
		}
	}

	if (role == Qt::ToolTipRole) {
		if ((contact = dynamic_cast<SIMContactListContact*>(item))) {
			PsiToolTip::showText(data.toPoint(),contact->toolTip(),contactList_->contactListView());
		} else if ((meta = dynamic_cast<SIMContactListMeta*>(item))) {
			PsiToolTip::showText(data.toPoint(),meta->toolTip(),contactList_->contactListView());
		} else if ((group = dynamic_cast<SIMContactListGroup*>(item))) {
			PsiToolTip::showText(data.toPoint(),group->toolTip(),contactList_->contactListView());
		} else if ((account = dynamic_cast<SIMContactListAccount*>(item))) {
			PsiToolTip::showText(data.toPoint(),account->toolTip(),contactList_->contactListView());
		}
	}

	if (role == Qt::EditRole) {
		if ((contact = dynamic_cast<SIMContactListContact*>(item))) {
			contact->account()->actionRename(contact->u()->jid(), data.toString());
		} else if ((group = dynamic_cast<SIMContactListGroup*>(item))) {
			QList<PsiAccount*> lpa = group->account()->psi()->contactList()->enabledAccounts();
			foreach(PsiAccount* pa, lpa)
				pa->actionGroupRename(group->name(), data.toString());
		}
	}

	return true;
}


Qt::ItemFlags SIMContactListModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	if (index.column() == NameColumn)
		f = f | Qt::ItemIsEditable;
	return f;
}

QVariant SIMContactListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(section);
	Q_UNUSED(orientation);
	Q_UNUSED(role);
	return QVariant();
}

QModelIndex SIMContactListModel::index( int row, int column, SIMContactListItem *item) const
{
	return createIndex(row, column, item);
}


QModelIndex SIMContactListModel::index( int row, int column, const QModelIndex &parent) const
{
	SIMContactListItem *parentItem;
	if (!parent.isValid())
		parentItem = contactList_->rootItem();
	else
		parentItem = static_cast<SIMContactListItem*>(parent.internalPointer());

	SIMContactListItem *item = parentItem->child(row);
	return (item ? createIndex(row, column, item) : QModelIndex());	
}

QModelIndex SIMContactListModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	SIMContactListItem *parent = (static_cast<SIMContactListItem*>(index.internalPointer()))->parent();
	return (parent == contactList_->rootItem() ? QModelIndex() : createIndex(parent->row(),0,parent));
}

int SIMContactListModel::rowCount(const QModelIndex &parent) const
{
	SIMContactListItem* parentItem;
	if (parent.isValid()) {
		parentItem = static_cast<SIMContactListItem*>(parent.internalPointer());
	}
	else {
		parentItem = contactList_->rootItem();
	}

	return (parentItem ? parentItem->size() : 0);
}

int SIMContactListModel::columnCount(const QModelIndex &parent) const
{
	return COLUMNS;
}

void SIMContactListModel::contactList_changed()
{
	reset();
}

