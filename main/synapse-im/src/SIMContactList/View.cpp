#include <QTreeView>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QLineEdit>

#include <QScrollBar>

#include "List.h"
#include "Model.h"
#include "View.h"
#include "Item.h"
#include "Meta.h"
#include "Contact.h"
#include "Group.h"

#include "psioptions.h"
#include "psiaccount.h"
#include "common.h"

using namespace SIMContactList;

View::View(QWidget* parent, QComboBox *search) : QTreeView(parent)
{
	setUniformRowHeights(false);
	setRootIsDecorated(false);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDragDropMode(DragDrop);
	setAutoScroll(true);
	setSelectionMode(NoSelection);
	setWordWrap(true);
	setEditTriggers(QAbstractItemView::EditKeyPressed);
	setIndentation(0);
	optionsUpdate();
	search_ = search;
	header()->hide();
	header()->setStretchLastSection(false);
	connect(this, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(qlv_doubleclick(const QModelIndex&)));
}

int View::showIcons() const
{
	return showIcons_;
}

void View::setShowIcons(int k)
{
	showIcons_ = k;
	if (header()->count() > Model::PixmapColumn) {
		header()->setSectionHidden(Model::PixmapColumn,!(k>1));
	}
	
	if (k == IconsOnly) {
		showColumn(Model::StateColumn);
		hideColumn(Model::PixmapColumn);
		hideColumn(Model::AvatarColumn);
	} else if (k == IconsAndAvatars) {
		showColumn(Model::StateColumn);
		hideColumn(Model::PixmapColumn);
		showColumn(Model::AvatarColumn);
	} else if (k == IconsOnAvatars) {
		hideColumn(Model::StateColumn);
		showColumn(Model::PixmapColumn);
		hideColumn(Model::AvatarColumn);
	}
}


void View::resetExpandedState()
{
	QAbstractItemModel* m = model();
	for (int i=0; i < m->rowCount(QModelIndex()); i++) {
		QModelIndex index = m->index(i,0);
		if (m->data(index,Model::ExpandedRole).toBool()) {
			Group *group = (static_cast<Group*>(index.internalPointer()));
			if(group)
				setExpanded(index, group->contactList()->isGroupOpen(group->name()));
		}
	}
}

void View::resizeEvent(QResizeEvent *e)
{
	QTreeView::resizeEvent(e);
	// hack to deal with QItemDelegate which needs to change width when scrollbar is in use.
	if(model() && (e->oldSize().width() != e->size().width()))
		QTreeView::doItemsLayout();
	emit resizeEventNotifier((QWidget*)this);
}

void View::doItemsLayout()
{
	QTreeView::doItemsLayout();
	resetExpandedState();
}

bool View::event(QEvent *event)
{
	if (event->type() == QEvent::ToolTip) {
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
		const QModelIndex index = indexAt(helpEvent->pos());
		QPoint pos = mapToGlobal(helpEvent->pos());
		return model()->setData(index,QVariant(pos),Qt::ToolTipRole);
	}
	else if (event->type() == QEvent::KeyPress) {
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);
		if (((ke->modifiers() == Qt::NoModifier) && (ke->key() != Qt::Key_Escape )) || (ke->modifiers() == Qt::ShiftModifier || ke->modifiers() == Qt::AltModifier)) {
			if (ke->key() == Qt::Key_Backspace) {
				QString tmp = search_->lineEdit()->text();
				tmp = tmp.left(tmp.size()-1);
				search_->setEditText(tmp);
				return true;
			} else if (!ke->text().isEmpty()) {
				QString tmp = search_->lineEdit()->text() + ke->text();
				search_->setEditText(tmp);
				return true;
			}
		}
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

void View::dragEnterEvent(QDragEnterEvent *e)
{
	if(e->mimeData()->hasFormat("application/x-contact-data") || e->mimeData()->hasUrls())
		e->acceptProposedAction();
}
	
void View::dragMoveEvent(QDragMoveEvent *e)
{
	if(e->mimeData()->hasFormat("application/x-contact-data") || e->mimeData()->hasUrls())
		e->acceptProposedAction();
}

void View::dropEvent(QDropEvent *e)
{
	Item *item = static_cast<Item*>(indexAt(e->pos()).internalPointer());
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
		
		Contact *contact = static_cast<Contact*>(indexAt(offset).internalPointer());

		if(item && contact) {
// Przebudować dla obsługi Metakontaktów -- should work!
			if (item->type() == Item::TContact)
				item = item->parent();

			if (item->type() == Item::TGroup)
			{
				Group *group = static_cast<Group*>(item);
				Group *oldParent = static_cast<Group*>(contact->parent());
				contact->setDefaultParent(item);
				contact->updateParents();
				contact->account()->actionGroupRemove(contact->jid(), oldParent->name());
				contact->account()->actionGroupAdd(contact->jid(), group->name());
			} else  if (item->type() == Item::TMeta) {
				Meta *meta = static_cast<Meta*>(item);
				Group *oldParent = static_cast<Group*>(contact->parent());
				Group *newgroup = static_cast<Group*>(item->parent());
				contact->setDefaultParent(item);
				contact->updateParents();
				contact->account()->actionGroupRemove(contact->jid(), oldParent->name());
				contact->account()->actionGroupAdd(contact->jid(), newgroup->name());
				contact->account()->actionMetaAdd(contact->jid(), meta->name(), meta->size() + 1);
			}
			e->accept();
		}
	} else if (e->mimeData()->hasUrls()) {
		if (item->type() == Item::TMeta)
			item = item->child(0);
		Contact *contact = static_cast<Contact*>(item);
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

void View::mousePressEvent(QMouseEvent *e)
{
	if (e->modifiers().testFlag(Qt::ShiftModifier)) {
		//drag event!!
		Item *item = static_cast<Item*>(indexAt(e->pos()).internalPointer());
		if(item && (item->type() == Item::TContact) && item->account()->loggedIn()) {
			Contact *contact = static_cast<Contact*>(item);
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

void View::qlv_doubleclick(const QModelIndex &index)
{
	if(PsiOptions::instance()->getOption("options.ui.contactlist.use-single-click").toBool())
		return;
	if(!index.isValid())
		return;
	Item* item = static_cast<Item*>(index.internalPointer());
	if(item)
		scActionDefault(item);
}

bool View::qlv_singleclick(QMouseEvent *e)
{
	bool done = false;

	const QModelIndex index = indexAt(e->pos());
	if(!index.isValid())
		return false;

	QAbstractItemModel* m = model();
	Item* item = static_cast<Item*>(index.internalPointer());

	if(e->button() == Qt::MidButton) {
		if(item->type() == Item::TContact || item->type() == Item::TMeta) {
			scActionDefault(item);
			done = true;
		}
	}
	else {
		if (e->button() == Qt::LeftButton && item->type() == Item::TGroup) {
			setExpanded(index, m->data(index,Model::ExpandedRole).toBool() && !isExpanded(index));
			Group *group = static_cast<Group*>(item);
			if(group)
				group->contactList()->setGroupOpen(group->name(), isExpanded(index));
			done = true;
		}
		else if(PsiOptions::instance()->getOption("options.ui.contactlist.use-left-click").toBool()) {
			if(e->button() == Qt::LeftButton) {
				if(PsiOptions::instance()->getOption("options.ui.contactlist.use-single-click").toBool()) {
					model()->setData(index,QVariant(mapToGlobal(e->pos())),Model::ContextMenuRole);
					done = true;
				}
//				else {
//					lcto_active = true;
//					lcto_pos = pos;
//					lcto_item = i;
//					QTimer::singleShot(QApplication::doubleClickInterval()/2, this, SLOT(leftClickTimeOut()));
//				}
			}
			else if(PsiOptions::instance()->getOption("options.ui.contactlist.use-single-click").toBool() && e->button() == Qt::RightButton) {
				if(item->type() == Item::TContact || item->type() == Item::TMeta) {
					scActionDefault(item);
					done = false;
				}
			}
		}
		else {
			if(e->button() == Qt::RightButton) {
				model()->setData(index,QVariant(mapToGlobal(e->pos())),Model::ContextMenuRole);
				done = true;
			}
			if(e->button() == Qt::LeftButton && PsiOptions::instance()->getOption("options.ui.contactlist.use-single-click").toBool()) {
				if(item->type() == Item::TContact || item->type() == Item::TMeta) {
					scActionDefault(item);
					done = false;
				}
			}
		}
	}
	return done;

//	d->typeAhead = "";
}

void View::scActionDefault(Item *item)
{
	if (item->type() == Item::TMeta)
		item = item->child(0);
	
	if (item->type() == Item::TContact) {
		item->account()->actionDefault((static_cast<Contact*>(item))->jid());
	} else if (item->type() == Item::TMeta) {
		item->account()->actionDefault((static_cast<Meta*>(item))->jid());
	}
}

void View::resizeColumns()
{
	QHeaderView* headerView = header();
	int columns = headerView->count();
	if (columns > Model::NameColumn) {
		headerView->setResizeMode(Model::NameColumn,QHeaderView::Stretch);
	}
	if (columns > Model::StateColumn) {
		headerView->setResizeMode(Model::StateColumn,QHeaderView::Fixed);
		headerView->resizeSection(Model::StateColumn,20);
	}
	if (columns >  Model::PixmapColumn) {
		headerView->setResizeMode(Model::PixmapColumn,QHeaderView::Fixed);
		headerView->resizeSection(Model::PixmapColumn,iconSize().width()+2);
	}
	if (columns > Model::AvatarColumn) {
		headerView->setResizeMode(Model::AvatarColumn,QHeaderView::Fixed);
		headerView->resizeSection(Model::AvatarColumn,iconSize().width());
	}
}

void View::setModel(QAbstractItemModel* model)
{
	QTreeView::setModel(model);
	resizeColumns();
	setShowIcons(showIcons());
}

void View::optionsUpdate()
{
	int iconSize = PsiOptions::instance()->getOption("options.ui.contactlist.avatar.size").toInt() + 4;
	setShowIcons(PsiOptions::instance()->getOption("options.ui.contactlist.avatar.show").toInt());
	setIconSize(QSize(iconSize,iconSize));
	resizeColumns();
	if(model())
		((Model*)model())->contactList_changed();
}

// Branches ? We don't want no steenking branches !
/*void ContactListView::drawBranches(QPainter*, const QRect&, const QModelIndex&) const
{
}*/
