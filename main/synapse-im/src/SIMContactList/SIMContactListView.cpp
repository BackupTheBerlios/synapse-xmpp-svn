#include <QTreeView>
#include <QHeaderView>
#include <QContextMenuEvent>

#include <QScrollBar>

#include "SIMContactList.h"
#include "SIMContactListModel.h"
#include "SIMContactListView.h"
#include "SIMContactListItem.h"
#include "SIMContactListMeta.h"
#include "SIMContactListContact.h"
#include "SIMContactListGroup.h"

#include "psioptions.h"
#include "psiaccount.h"
#include "common.h"

SIMContactListView::SIMContactListView(QWidget* parent) : QTreeView(parent)
{
	setUniformRowHeights(false);
	setRootIsDecorated(false);
	setDragEnabled(true);
	setDragDropMode(DragDrop);
	setSelectionMode(NoSelection);
	setEditTriggers(QAbstractItemView::EditKeyPressed|QAbstractItemView::AnyKeyPressed);
	setIndentation(0);
	updateOptions();
	header()->hide();
	header()->setStretchLastSection(false);
	connect(this, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(qlv_doubleclick(const QModelIndex&)));
}

int SIMContactListView::showIcons() const
{
	return showIcons_;
}

void SIMContactListView::setShowIcons(int k)
{
	showIcons_ = k;
	if (header()->count() > SIMContactListModel::PixmapColumn) {
		header()->setSectionHidden(SIMContactListModel::PixmapColumn,!(k>1));
	}
	
	if (k == IconsOnly) {
		showColumn(SIMContactListModel::StateColumn);
		hideColumn(SIMContactListModel::PixmapColumn);
		hideColumn(SIMContactListModel::AvatarColumn);
	} else if (k == IconsAndAvatars) {
		showColumn(SIMContactListModel::StateColumn);
		hideColumn(SIMContactListModel::PixmapColumn);
		showColumn(SIMContactListModel::AvatarColumn);
	} else if (k == IconsOnAvatars) {
		hideColumn(SIMContactListModel::StateColumn);
		showColumn(SIMContactListModel::PixmapColumn);
		hideColumn(SIMContactListModel::AvatarColumn);
	}
}


void SIMContactListView::resetExpandedState()
{
	QAbstractItemModel* m = model();
	for (int i=0; i < m->rowCount(QModelIndex()); i++) {
		QModelIndex index = m->index(i,0);
		if (m->data(index,SIMContactListModel::ExpandedRole).toBool()) {
			SIMContactListGroup *group = (static_cast<SIMContactListGroup*>(index.internalPointer()));
			if(group)
				setExpanded(index, group->contactList()->isGroupOpen(group->name()));
		}
	}
}

void SIMContactListView::doItemsLayout()
{
	QTreeView::doItemsLayout();
	resetExpandedState();
}

bool SIMContactListView::event(QEvent *event)
{
	if (event->type() == QEvent::ToolTip) {
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
		const QModelIndex index = indexAt(helpEvent->pos());
		QPoint pos = mapToGlobal(helpEvent->pos());
		return model()->setData(index,QVariant(pos),Qt::ToolTipRole);
	}
	else if (event->type() == QEvent::Wheel) {
		QWheelEvent *ew= static_cast<QWheelEvent *>(event);
		QScrollBar *sb = verticalScrollBar();
     		int numDegrees = ew->delta() / 8;
     		int numSteps = numDegrees / 15;
	
		if (numSteps < (sb->minimum() - sb->value()))
			numSteps = sb->value() * (-1);
		else if (numSteps > (sb->maximum() - sb->value()))
			numSteps = (sb->maximum() - sb->value());

       		sb->setValue(verticalScrollBar()->value() - numSteps);
		return true;
	}
	return QWidget::event(event);
}

// Drag & Drop

void SIMContactListView::dragEnterEvent(QDragEnterEvent *e)
{
	if(e->mimeData()->hasFormat("application/x-contact-data") || e->mimeData()->hasUrls())
		e->acceptProposedAction();
}
	
void SIMContactListView::dragMoveEvent(QDragMoveEvent *e)
{
	if(e->mimeData()->hasFormat("application/x-contact-data") || e->mimeData()->hasUrls())
		e->acceptProposedAction();
}

void SIMContactListView::dropEvent(QDropEvent *e)
{
	SIMContactListItem *item = static_cast<SIMContactListItem*>(indexAt(e->pos()).internalPointer());
	if (item == NULL || !item->account()->loggedIn()) {
		e->ignore();
		return;
	}

	if(e->mimeData()->hasFormat("application/x-contact-data")) {
		QByteArray itemData = e->mimeData()->data("application/x-contact-data");
		QDataStream dataStream(&itemData, QIODevice::ReadOnly);

		QPixmap pixmap;
		QPoint offset;
		dataStream >> pixmap >> offset;
		
		SIMContactListContact *contact = static_cast<SIMContactListContact*>(indexAt(offset).internalPointer());

		if(item && contact) {
// Przebudować dla obsługi Metakontaktów -- should work!
			if (item->type() == SIMContactListItem::Contact)
				item = item->parent();

			if (item->type() == SIMContactListItem::Group)
			{
				SIMContactListGroup *group = static_cast<SIMContactListGroup*>(item);
				SIMContactListGroup *oldParent = static_cast<SIMContactListGroup*>(contact->parent());
				contact->setDefaultParent(item);
				contact->updateParents();
				contact->account()->actionGroupRemove(contact->jid(), oldParent->name());
				contact->account()->actionGroupAdd(contact->jid(), group->name());
			} else  if (item->type() == SIMContactListItem::Meta) {
				SIMContactListMeta *meta = static_cast<SIMContactListMeta*>(item);
				SIMContactListGroup *oldParent = static_cast<SIMContactListGroup*>(contact->parent());
				SIMContactListGroup *newgroup = static_cast<SIMContactListGroup*>(item->parent());
				contact->setDefaultParent(item);
				contact->updateParents();
				contact->account()->actionGroupRemove(contact->jid(), oldParent->name());
				contact->account()->actionGroupAdd(contact->jid(), newgroup->name());
				contact->account()->actionMetaAdd(contact->jid(), meta->name(), meta->size() + 1);
			}
			e->accept();
		}
	} else if (e->mimeData()->hasUrls()) {
		if (item->type() == SIMContactListItem::Meta)
			item = item->child(0);
		SIMContactListContact *contact = static_cast<SIMContactListContact*>(item);
		if (contact) {
			QStringList filesList;
			QList<QUrl> urlList(e->mimeData()->urls());
			for(int i=0; i<urlList.size(); ++i)
				filesList << urlList.takeAt(i).toLocalFile();
			contact->account()->actionSendFiles(contact->jid(), filesList);
		}
	} else {
		e->ignore();
	}
}

void SIMContactListView::mousePressEvent(QMouseEvent *e)
{
	if (e->modifiers().testFlag(Qt::ShiftModifier)) {
		//drag event!!
		SIMContactListItem *item = static_cast<SIMContactListItem*>(indexAt(e->pos()).internalPointer());
		if(item && (item->type() == SIMContactListItem::Contact) && item->account()->loggedIn()) {
			SIMContactListContact *contact = static_cast<SIMContactListContact*>(item);
			if(contact) {
				QPixmap pixmap = contact->state();
				QByteArray itemData;
				QDataStream dataStream(&itemData, QIODevice::WriteOnly);
				dataStream << pixmap << QPoint(e->pos());
				QMimeData *mimeData = new QMimeData;
				mimeData->setData("application/x-contact-data", itemData);
				QDrag *drag = new QDrag(this);
				drag->setMimeData(mimeData);
				drag->setPixmap(pixmap);
				startDrag(Qt::MoveAction);
				drag->exec(Qt::MoveAction);
			}
		}
		return;
	}

	if (qlv_singleclick(e)) {
		return;
	}
	QTreeView::mousePressEvent(e);
}

void SIMContactListView::qlv_doubleclick(const QModelIndex &index)
{
	if(option.singleclick)
		return;
	if(!index.isValid())
		return;
	SIMContactListItem* item = static_cast<SIMContactListItem*>(index.internalPointer());
	if(item)
		scActionDefault(item);
}

bool SIMContactListView::qlv_singleclick(QMouseEvent *e)
{
	bool done = false;

	const QModelIndex index = indexAt(e->pos());
	if(!index.isValid())
		return false;

	QAbstractItemModel* m = model();
	SIMContactListItem* item = static_cast<SIMContactListItem*>(index.internalPointer());

	if(e->button() == Qt::MidButton) {
		if(item->type() == SIMContactListItem::Contact || item->type() == SIMContactListItem::Meta) {
			scActionDefault(item);
			done = true;
		}
	}
	else {
		if (e->button() == Qt::LeftButton && item->type() == SIMContactListItem::Group) {
			setExpanded(index, m->data(index,SIMContactListModel::ExpandedRole).toBool() && !isExpanded(index));
			SIMContactListGroup *group = static_cast<SIMContactListGroup*>(item);
			if(group)
				group->contactList()->setGroupOpen(group->name(), isExpanded(index));
			done = true;
		}
		else if(option.useleft) {
			if(e->button() == Qt::LeftButton) {
				if(option.singleclick) {
					model()->setData(index,QVariant(mapToGlobal(e->pos())),SIMContactListModel::ContextMenuRole);
					done = true;
				}
//				else {
//					lcto_active = true;
//					lcto_pos = pos;
//					lcto_item = i;
//					QTimer::singleShot(QApplication::doubleClickInterval()/2, this, SLOT(leftClickTimeOut()));
//				}
			}
			else if(option.singleclick && e->button() == Qt::RightButton) {
				if(item->type() == SIMContactListItem::Contact || item->type() == SIMContactListItem::Meta) {
					scActionDefault(item);
					done = false;
				}
			}
		}
		else {
			if(e->button() == Qt::RightButton) {
				model()->setData(index,QVariant(mapToGlobal(e->pos())),SIMContactListModel::ContextMenuRole);
				done = true;
			}
			if(e->button() == Qt::LeftButton && option.singleclick) {
				if(item->type() == SIMContactListItem::Contact || item->type() == SIMContactListItem::Meta) {
					scActionDefault(item);
					done = false;
				}
			}
		}
	}
	return done;

//	d->typeAhead = "";
}

void SIMContactListView::scActionDefault(SIMContactListItem *item)
{
	if (item->type() == SIMContactListItem::Meta)
		item = item->child(0);
	
	if (item->type() == SIMContactListItem::Contact) {
		item->account()->actionDefault((static_cast<SIMContactListContact*>(item))->jid());
	} else if (item->type() == SIMContactListItem::Meta) {
		item->account()->actionDefault((static_cast<SIMContactListMeta*>(item))->jid());
	}
}

void SIMContactListView::resizeColumns()
{
	QHeaderView* headerView = header();
	int columns = headerView->count();
	if (columns > SIMContactListModel::NameColumn) {
		headerView->setResizeMode(SIMContactListModel::NameColumn,QHeaderView::Stretch);
	}
	if (columns > SIMContactListModel::StateColumn) {
		headerView->setResizeMode(SIMContactListModel::StateColumn,QHeaderView::Custom);
		headerView->resizeSection(SIMContactListModel::StateColumn,20);
	}
	if (columns >  SIMContactListModel::PixmapColumn) {
		headerView->setResizeMode(SIMContactListModel::PixmapColumn,QHeaderView::Custom);
		headerView->resizeSection(SIMContactListModel::PixmapColumn,iconSize().width()+2);
	}
	if (columns > SIMContactListModel::AvatarColumn) {
		headerView->setResizeMode(SIMContactListModel::AvatarColumn,QHeaderView::Custom);
		headerView->resizeSection(SIMContactListModel::AvatarColumn,iconSize().width());
	}
}

void SIMContactListView::setModel(QAbstractItemModel* model)
{
	QTreeView::setModel(model);
	resizeColumns();
	setShowIcons(showIcons());
}

void SIMContactListView::updateOptions()
{
	int iconSize = PsiOptions::instance()->getOption("options.ui.contactlist.avatar.size").toInt() + 4;
	setShowIcons(PsiOptions::instance()->getOption("options.ui.contactlist.avatar.show").toInt());
	setIconSize(QSize(iconSize,iconSize));
	resizeColumns();
	if(model())
		((SIMContactListModel*)model())->contactList_changed();
}

// Branches ? We don't want no steenking branches !
/*void ContactListView::drawBranches(QPainter*, const QRect&, const QModelIndex&) const
{
}*/
