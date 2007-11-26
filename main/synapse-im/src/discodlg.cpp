/*
 * discodlg.cpp - main dialog for the Service Discovery protocol
 * Copyright (C) 2003  Michail Pishchagin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "discodlg.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <q3popupmenu.h>
#include <Q3PtrList>

#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QAction>
#include <QSignalMapper>
#include <QPushButton>
#include <QToolButton>
#include <QToolBar>
#include <QScrollBar>

#include <QActionGroup>
#include <QEvent>
#include <QList>
#include <QContextMenuEvent>

#include <QItemDelegate>
#include <QPixmap>
#include <QPainter>

#include "xmpp_tasks.h"

#include "tasklist.h"
#include "psiaccount.h"
#include "psicon.h"
#include "busywidget.h"
#include "common.h"
#include "iconaction.h"
#include "psiiconset.h"
#include "psitooltip.h"
#include "stretchwidget.h"
#include "psioptions.h"
#include "accountlabel.h"

//----------------------------------------------------------------------------

PsiIcon category2icon(const QString &category, const QString &type, int status=STATUS_ONLINE)
{
	// TODO: update this to http://www.jabber.org/registrar/disco-categories.html#gateway

	// still have to add more options...
	if ( category == "service" || category == "gateway" ) {
		QString trans;

		if (type == "aim")
			trans = "aim";
		else if (type == "icq")
			trans = "icq";
		else if (type == "msn")
			trans = "msn";
		else if (type == "yahoo")
			trans = "yahoo";
		else if (type == "gadu-gadu" || type == "x-gadugadu")
			trans = "gadugadu";
		else if (type == "sms")
			trans = "sms";
		else
			trans = "transport";

		return PsiIconset::instance()->transportStatus(trans, status);

		// irc
		// jud
		// pager
		// jabber
		// serverlist
		// smtp
	}
	else if ( category == "conference" ) {
		if (type == "public" || type == "private" || type == "text" || type == "irc")
			return IconsetFactory::icon("psi/groupChat");
		else if (type == "url")
			return IconsetFactory::icon("psi/www");
		// irc
		// list
		// topic
	}
	else if ( category == "validate" ) {
		if (type == "xml")
			return IconsetFactory::icon("psi/xml");
		// grammar
		// spell
	}
	else if ( category == "user" || category == "client" ) {
		// client
		// forward
		// inbox
		// portable
		// voice
		return PsiIconset::instance()->status(STATUS_ONLINE);
	}
	// application
	   // bot
	   // calendar
	   // editor
	   // fileserver
	   // game
	   // whiteboard
	// headline
	   // logger
	   // notice
	   // rss
	   // stock
	// keyword
	   // dictionary
	   // dns
	   // software
	   // thesaurus
	   // web
	   // whois
	// render
	   // en2ru
	   // ??2??
	   // tts
	return PsiIcon();
}

//----------------------------------------------------------------------------
// DiscoGroupItem -- group similar DiscoItems into groups
//----------------------------------------------------------------------------

class DiscoGroupItem : public QTreeWidgetItem
{
public:
	enum Type {
		TRANSPORTS = 0,
		CONFERENCES,
		OTHERS,
		MAX
	};

	DiscoGroupItem(QString _type, QTreeWidget *parent)
	: QTreeWidgetItem(parent)
	{
		type_ = type(_type);
		if (type_ == TRANSPORTS)
			setText(0, QObject::tr("Transports"));
		else if (type_ == CONFERENCES)
			setText(0, QObject::tr("Conferences"));
		else
			setText(0, QObject::tr("Other"));
	}
	
	DiscoGroupItem(QString _type, QTreeWidgetItem *parent)
	: QTreeWidgetItem(parent)
	{
		type_ = type(_type);
		if (type_ == TRANSPORTS)
			setText(0, QObject::tr("Transports"));
		else if (type_ == CONFERENCES)
			setText(0, QObject::tr("Conferences"));
		else
			setText(0, QObject::tr("Other"));
	}

	int compare(QString _type)
	{
		return type(_type) - type_;
	}

	int type(QString _type)
	{
		if ((_type == "service") || (_type == "gateway"))
			return TRANSPORTS;
		else if (_type == "conference")
			return CONFERENCES;
		
		return OTHERS;
	}

private:
	int type_;
};

//----------------------------------------------------------------------------
// DiscoData -- a shared data struct
//----------------------------------------------------------------------------

class DiscoListItem;
class DiscoConnector : public QObject
{
	Q_OBJECT
public:
	DiscoConnector(QObject *parent)
	: QObject(parent) {}

signals:
	void itemUpdated(QTreeWidgetItem *);

private:
	friend class DiscoListItem;
};

struct DiscoData {
	PsiAccount *pa;
	TaskList *tasks;
	DiscoConnector *d;

	enum Protocol {
		Auto,

		Disco,
		Browse,
		Agents
	};
	Protocol protocol;
};

//----------------------------------------------------------------------------
// DiscoListItem
//----------------------------------------------------------------------------

class DiscoListItem : public QObject, public QTreeWidgetItem
{
	Q_OBJECT
public:
	DiscoListItem(DiscoItem it, DiscoData *d, QTreeWidget *parent);
	DiscoListItem(DiscoItem it, DiscoData *d, QTreeWidgetItem *parent);
	~DiscoListItem();

	QString text(int columns) const;
	void setOpen(bool open);
	const DiscoItem &item() const;

	void itemSelected();

public slots: // the two are used internally by class, and also called by DiscoDlg::Private::refresh()
	void updateInfo();
	void updateItems(bool parentAutoItems = false);
	QString getErrorInfo() const;

private slots:
	void discoItemsFinished();
	void discoInfoFinished();

	void doBrowse(bool parentAutoItems = false);
	void doAgents(bool parentAutoItems = false);
	void browseFinished();
	void agentsFinished();

private:
	DiscoItem di;
	DiscoData *d;
	bool isRoot;
	bool alreadyItems, alreadyInfo;
	bool autoItems; // used in updateItemsFinished
	bool autoInfo;
	QString errorInfo;

	void copyItem(const DiscoItem &);
	void updateInfo(const DiscoItem &);
	void updateItemsFinished(const DiscoList &);
	void autoItemsChildren() const; // automatically call disco#items for children :-)
	QString hash() { return computeHash( item().jid().full(), item().node() ); }
	QString computeHash( QString jid, QString node );

	// helper functions
	void init(DiscoItem it, DiscoData *dd);

	DiscoDlg *dlg() const;
};

DiscoListItem::DiscoListItem(DiscoItem it, DiscoData *_d, QTreeWidget *parent)
: QTreeWidgetItem (parent)
{
	isRoot = true;

	init(it, _d);
}

DiscoListItem::DiscoListItem(DiscoItem it, DiscoData *_d, QTreeWidgetItem *parent)
: QTreeWidgetItem (parent)
{
	isRoot = false;

	init(it, _d);
}

DiscoListItem::~DiscoListItem()
{
}

void DiscoListItem::init(DiscoItem _item, DiscoData *_d)
{
	d = _d;
	di = _item;
	copyItem(_item);

	alreadyItems = alreadyInfo = false;

	autoInfo = false;

	updateInfo();

	if ( !isRoot )
		autoInfo = true;
}

void DiscoListItem::copyItem(const DiscoItem &it)
{
	if ( !(!di.jid().full().isEmpty() && it.jid().full().isEmpty()) )
		di.setJid ( it.jid() );
	if ( !(!di.node().isEmpty() && it.node().isEmpty()) )
		di.setNode ( it.node() );
	if ( !(!di.name().isEmpty() && it.name().isEmpty()) )
		di.setName ( it.name() );
	
	if ( di.name().isEmpty() && !di.jid().full().isEmpty() ) // use JID in the Name column
		di.setName ( di.jid().full() );

	di.setAction ( it.action() );

	if ( !(!di.features().list().isEmpty() && it.features().list().isEmpty()) )
		di.setFeatures ( it.features() );
	if ( !(!di.identities().isEmpty() && it.identities().isEmpty()) )
		di.setIdentities ( it.identities() );

	if ( di.jid().userHost().left(4) == "jud." || di.jid().userHost().left(6) == "users." ) {
		// nasty hack for the nasty (and outdated) JUD service :-/
		if ( !di.features().canSearch() ) {
			QStringList features = di.features().list();
			features << "jabber:iq:search";
			di.setFeatures( features );
		}

		bool found = false;
		DiscoItem::Identities::ConstIterator it = di.identities().begin();
		for ( ; it != di.identities().end(); ++it) {
			if ( (*it).category == "service" && (*it).type == "jud" ) {
				found = true;
				break;
			}
		}
		if ( !found ) {
			DiscoItem::Identity id;
			id.category = "service";
			id.type     = "jud";
			DiscoItem::Identities ids;
			ids << id;
			di.setIdentities( ids );
		}
	}

	bool pixmapOk = false;
	if ( !di.identities().isEmpty() ) {
		DiscoItem::Identity id = di.identities().first();

		if ( !id.category.isEmpty() ) {
			PsiIcon ic = category2icon(id.category, id.type);

			if ( !ic.impix().isNull() ) {
				setIcon (0, ic.icon());
				pixmapOk = true;
			}
		}
	}

	if ( !pixmapOk )
		setIcon(0, PsiIconset::instance()->status(di.jid(), STATUS_ONLINE).icon());

	setText(0, di.name().simplified());
	setText(1, di.jid().full());
	setData(0, Qt::UserRole, di.jid().full());
	setText(2, di.node().simplified());

	if ( isSelected() ) // update actions
		emit d->d->itemUpdated( this );
}

QString DiscoListItem::text (int c) const
{
	if (c == 0)
		return di.name().simplified();
	else if (c == 1)
		return di.jid().full();
	else if (c == 2)
		return di.node().simplified();
	return "";
}

QString DiscoListItem::getErrorInfo() const
{
	return errorInfo;
}

const DiscoItem &DiscoListItem::item() const
{
	return di;
}

DiscoDlg *DiscoListItem::dlg() const
{
	return (DiscoDlg *)treeWidget()->parent();
}

void DiscoListItem::setOpen (bool o)
{
	if ( o ) {
		if ( !alreadyItems )
			updateItems();
		else
			autoItemsChildren();
	}

	QTreeWidgetItem::setExpanded(o);
}

void DiscoListItem::itemSelected()
{
	if ( !alreadyInfo )
		updateInfo();
}

void DiscoListItem::updateItems(bool parentAutoItems)
{
	if ( parentAutoItems ) {
		// save traffic
		if ( alreadyItems )
			return;

		// FIXME: currently, JUD doesn't seem to answer to browsing requests
		if ( item().identities().size() ) {
			DiscoItem::Identity id = item().identities().first();
			if ( id.category == "service" && id.type == "jud" )
				return;
		}
		QString j = item().jid().host(); // just another method to discover if we're gonna to browse JUD
		if ( item().jid().user().isEmpty() && (j.left(4) == "jud." || j.left(6) == "users.") )
			return;
	}

	autoItems = !parentAutoItems;

	autoItems = true;

	if ( d->protocol == DiscoData::Auto || d->protocol == DiscoData::Disco ) {
		JT_DiscoItems *jt = new JT_DiscoItems(d->pa->client()->rootTask());
		connect(jt, SIGNAL(finished()), SLOT(discoItemsFinished()));
		jt->get(di.jid(), di.node());
		jt->go(true);
		d->tasks->append(jt);
	}
	else if ( d->protocol == DiscoData::Browse )
		doBrowse(parentAutoItems);
	else if ( d->protocol == DiscoData::Agents )
		doAgents(parentAutoItems);
}

void DiscoListItem::discoItemsFinished()
{
	JT_DiscoItems *jt = (JT_DiscoItems *)sender();

	if ( jt->success() ) {
		updateItemsFinished(jt->items());
	}
	else if ( d->protocol == DiscoData::Auto ) {
		doBrowse();
		return;
	}
	else if ( !autoItems ) {
		QString error = jt->statusString();
		QMessageBox::critical(dlg(), tr("Error"), tr("There was an error getting items for <b>%1</b>.<br>Reason: %2").arg(di.jid().full()).arg(QString(error).replace('\n', "<br>")));
	}

	alreadyItems = true;
}

void DiscoListItem::doBrowse(bool parentAutoItems)
{
	if ( parentAutoItems ) {
		// save traffic
		if ( alreadyItems )
			return;

		if ( item().identities().size() ) {
			DiscoItem::Identity id = item().identities().first();
			if ( id.category == "service" && id.type == "jud" )
				return;
		}
	}

	autoItems = true;

	JT_Browse *jt = new JT_Browse(d->pa->client()->rootTask());
	connect(jt, SIGNAL(finished()), SLOT(browseFinished()));
	jt->get(di.jid());
	jt->go(true);
	d->tasks->append(jt);
}

void DiscoListItem::browseFinished()
{
	JT_Browse *jt = (JT_Browse *)sender();

	if ( jt->success() ) {
		// update info
		DiscoItem root;
		root.fromAgentItem( jt->root() );
		updateInfo(root);
		alreadyInfo = true;
		autoInfo = false;

		// update items
		AgentList from = jt->agents();
		DiscoList to;
		AgentList::Iterator it = from.begin();
		for ( ; it != from.end(); ++it) {
			DiscoItem item;
			item.fromAgentItem( *it );

			to.append( item );
		}

		updateItemsFinished(to);
	}
	else if ( d->protocol == DiscoData::Auto ) {
		doAgents();
		return;
	}
	else if ( !autoItems ) {
		QString error = jt->statusString();
		QMessageBox::critical(dlg(), tr("Error"), tr("There was an error browsing items for <b>%1</b>.<br>Reason: %2").arg(di.jid().full()).arg(QString(error).replace('\n', "<br>")));
	}

	alreadyItems = true;
}

void DiscoListItem::doAgents(bool parentAutoItems)
{
	if ( parentAutoItems ) {
		// save traffic
		if ( alreadyItems )
			return;

		if ( item().identities().size() ) {
			DiscoItem::Identity id = item().identities().first();
			if ( id.category == "service" && id.type == "jud" )
				return;
		}
	}

	autoItems = true;

	JT_GetServices *jt = new JT_GetServices(d->pa->client()->rootTask());
	connect(jt, SIGNAL(finished()), SLOT(agentsFinished()));
	jt->get(di.jid());
	jt->go(true);
	d->tasks->append(jt);
}

void DiscoListItem::agentsFinished()
{
	JT_GetServices *jt = (JT_GetServices *)sender();

	if ( jt->success() ) {
		// update info
		DiscoItem root;
		DiscoItem::Identity id;
		id.name     = tr("Jabber Service");
		id.category = "service";
		id.type     = "jabber";
		DiscoItem::Identities ids;
		ids.append(id);
		root.setIdentities(ids);
		updateInfo(root);
		alreadyInfo = true;
		autoInfo = false;

		// update items
		AgentList from = jt->agents();
		DiscoList to;
		AgentList::Iterator it = from.begin();
		for ( ; it != from.end(); ++it) {
			DiscoItem item;
			item.fromAgentItem( *it );

			to.append( item );
		}

		updateItemsFinished(to);
	}
	else if ( !autoItems ) {
		QString error = jt->statusString();
		QMessageBox::critical(dlg(), tr("Error"), tr("There was an error getting agents for <b>%1</b>.<br>Reason: %2").arg(di.jid().full()).arg(QString(error).replace('\n', "<br>")));
	}

	alreadyItems = true;
}

QString DiscoListItem::computeHash( QString jid, QString node )
{
	QString ret = jid.replace( '@', "\\@" );
	ret += "@";
	ret += node.replace( '@', "\\@" );
	return ret;
}

void DiscoListItem::updateItemsFinished(const DiscoList &list)
{
	Q3Dict<DiscoListItem> children;
	for(int i = 0; i<childCount(); i++) {
		DiscoListItem *ch = (DiscoListItem *)QTreeWidgetItem::child(i);
		children.insert( ch->hash(), ch );
	}

	// add/update items
	for(DiscoList::ConstIterator it = list.begin(); it != list.end(); ++it) {
		const DiscoItem a = *it;

		QString key = computeHash(a.jid().full(), a.node());
		DiscoListItem *ch = children[ key ];

		if ( ch ) {
			ch->copyItem ( a );
			children.remove( key );
		}
		else {
			new DiscoListItem (a, d, this);
		}
	}

	// remove all items that are not on new DiscoList
	children.setAutoDelete( true );
	children.clear();

		autoItemsChildren();

	// root item is initially hidden
	if ( isRoot && isHidden() )
		setHidden (false);
}

void DiscoListItem::autoItemsChildren() const
{
	if(!isRoot) {
		for(int i = 0; i<childCount(); i++) {
			DiscoListItem *ch = (DiscoListItem *)QTreeWidgetItem::child(i);
			ch->updateItems(true);
		}
	} else {
		for(int j = 0; j<childCount(); j++) {
			QTreeWidgetItem *gi = QTreeWidgetItem::child(j);
			for(int i = 0; i<gi->childCount(); i++) {
				DiscoListItem *ch = (DiscoListItem *)gi->child(i);
				ch->updateItems(true);
			}
		}
	}
}

void DiscoListItem::updateInfo()
{
	if ( d->protocol != DiscoData::Auto && d->protocol != DiscoData::Disco )
		return;

	JT_DiscoInfo *jt = new JT_DiscoInfo(d->pa->client()->rootTask());
	connect(jt, SIGNAL(finished()), SLOT(discoInfoFinished()));
	jt->get(di.jid(), di.node());
	jt->go(true);
	d->tasks->append(jt);
}

void DiscoListItem::discoInfoFinished()
{
	JT_DiscoInfo *jt = (JT_DiscoInfo *)sender();

	if ( jt->success() ) {
		updateInfo( jt->item() );
	}
	else {
		QString error_str = jt->statusString();
		int error_code = jt->statusCode();

		// we change the icon for the items with disco#info returning type=="cancel" || type=="wait" error codes
		// based on http://www.jabber.org/jeps/jep-0086.html

		// FIXME: use another method for checking XMPP error-types when Iris will provide one
		if ( error_code==400 || error_code==404 || error_code==405 || error_code==409 ||
		     error_code==500 || error_code==501 || error_code==503 || error_code==504 ) {
			bool pixmapOk = false;
			if ( !di.identities().isEmpty() ) {
				DiscoItem::Identity id = di.identities().first();
				if ( !id.category.isEmpty() ) {
					PsiIcon ic = category2icon(id.category, id.type, STATUS_ERROR);

					if ( !ic.impix().isNull() ) {
						setIcon (0, ic.icon());
						pixmapOk = true;
					}
				}
			}
			if ( !pixmapOk )
				setIcon(0, PsiIconset::instance()->status(di.jid(), STATUS_ERROR).icon());
		} else {
		}

		errorInfo=QString("%1").arg(QString(error_str).replace('\n', "<br>"));

		if ( !autoInfo && d->protocol != DiscoData::Auto ) {	
			QMessageBox::critical(dlg(), tr("Error"), tr("There was an error getting item's info for <b>%1</b>.<br>Reason: %2").arg(di.jid().full()).arg(QString(error_str).replace('\n', "<br>")));
		}
	}

	alreadyInfo = true;
	autoInfo = false;
}

void DiscoListItem::updateInfo(const DiscoItem &item)
{
	copyItem( item );

	if ( treeWidget()->indexOfTopLevelItem(QTreeWidgetItem::parent()) != -1 ) {
		DiscoGroupItem *gi = NULL;
		DiscoItem::Identities ids = item.identities();
		if(!ids.isEmpty()) {
			for(int i = 0; i<QTreeWidgetItem::parent()->childCount(); i++) {
 				gi = dynamic_cast<DiscoGroupItem*>(QTreeWidgetItem::parent()->child(i));
				if ((gi != NULL) && (gi->compare(ids.first().category) == 0)) {
						break;
				}
				gi = NULL;
			}

			if(gi == NULL) {
				gi = new DiscoGroupItem(ids.first().category, QTreeWidgetItem::parent());
				QTreeWidgetItem::parent()->removeChild(gi);
				QTreeWidgetItem::parent()->insertChild(gi->type(ids.first().category),gi);
				
			}

			if(gi != NULL) {
				QTreeWidgetItem::parent()->removeChild(this);
				if (gi->childCount() > 0) {
					for(int i = 0; i<gi->childCount(); i++) {
							if (text(0).compare(gi->child(i)->text(0)) < 0) {
							gi->insertChild(i, this);
							break;
						}
					}
				} else {
					gi->addChild(this);
				}
				gi->setExpanded(true);
			}
		}
	} else if (!isRoot) {
		QTreeWidgetItem *p = QTreeWidgetItem::parent();
		p->removeChild(this);
		if (p->childCount() > 0) {
			for(int i = 0; i<p->childCount(); i++) {
				if (text(0).compare(p->child(i)->text(0)) < 0) {
					p->insertChild(i, this);
					break;
				}
			}
		} else {
			p->addChild(this);
		}

	}

	if ( isRoot && isHidden() )
		setHidden (false);
}

class TransportDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	TransportDelegate(QWidget *parent = 0) : QItemDelegate(parent) {}

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const 
	{
		if (static_cast<DiscoListItem*>(index.internalPointer()) && !qVariantValue<QString>(index.data(Qt::UserRole)).isEmpty()) {
			QPixmap pix = qVariantValue<QIcon>(index.data(Qt::DecorationRole)).pixmap(32,32);
			QString name = qVariantValue<QString>(index.data(Qt::DisplayRole));
			QString jid = qVariantValue<QString>(index.data(Qt::UserRole));
			painter->save();
			painter->fillRect(option.rect, QBrush(option.palette.brush(QPalette::Base)));
			painter->translate(option.rect.x(), option.rect.y());
			painter->drawPixmap(0,0,pix);
			int h;
			{
				QFont font = painter->font();
				font.setBold(true);
				QFontMetrics fm(font);
				painter->setFont(font);
				h = (fm.height()+1)/2;
				painter->drawText(pix.width()+2, h, name);
				painter->translate(0, fm.height()+2);
			}
			{
				QFont font = painter->font();
				font.setBold(false);
				font.setItalic(true);
				QFontMetrics fm(font);
				h = (fm.height()+1)/2;
				painter->setFont(font);
				painter->drawText(pix.width()+2, h, jid);
				painter->translate(0, fm.height()+2);
			}
			painter->restore();
		} else {
			QItemDelegate::paint(painter, option, index);
		}
	}
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		if (static_cast<DiscoListItem*>(index.internalPointer()) && !qVariantValue<QString>(index.data(Qt::UserRole)).isEmpty()) {
			QPainter painter;
			QPixmap pix = qVariantValue<QIcon>(index.data(Qt::DecorationRole)).pixmap(32,32);
			QString name = qVariantValue<QString>(index.data(Qt::DisplayRole));
			QString jid = qVariantValue<QString>(index.data(Qt::UserRole));
			QFont font_n = painter.font();
			font_n.setBold(true);
			QFontMetrics fm_n(font_n);
			QFont font_j = painter.font();
			font_j.setBold(false);
			font_j.setItalic(true);
			QFontMetrics fm_j(font_j);
			int w = QMAX( fm_n.width(name), fm_j.width(jid) ) + pix.width() + 4;
			int h = QMAX( fm_n.height() + 4 + fm_j.height(), pix.height() );
			return QSize( w, h);
		} else {
			return QItemDelegate::sizeHint(option, index);
		}
	}
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		return QItemDelegate::createEditor(parent, option, index);
	}
	void setEditorData(QWidget *editor, const QModelIndex &index) const
	{
		QItemDelegate::setEditorData(editor, index);
	}
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
	{
		QItemDelegate::setModelData(editor, model, index);
	}

};

//----------------------------------------------------------------------------
// DiscoList
//----------------------------------------------------------------------------

class DiscoListView : public QTreeWidget
{
	Q_OBJECT
public:
	DiscoListView(QWidget *parent);

protected:
	bool maybeTip(const QPoint &);

	// reimplemented
	bool eventFilter(QObject* o, QEvent* e);
	void resizeEvent(QResizeEvent*);
};

DiscoListView::DiscoListView(QWidget *parent)
: QTreeWidget(parent)
{
	setColumnCount(3);
	QStringList headers;
	headers << tr("Name") << tr("JID") << tr("Node");
	setHeaderLabels(headers);
	setRootIsDecorated(false);
	setIndentation(4);
	hideColumn(1);
	hideColumn(2);
}

void DiscoListView::resizeEvent(QResizeEvent* e)
{
	QTreeWidget::resizeEvent(e);

	header()->adjustSize();
	resizeColumnToContents(0);
}

/**
 * \param pos should be in global coordinate system.
 */
bool DiscoListView::maybeTip(const QPoint &pos)
{
	DiscoListItem* i = dynamic_cast<DiscoListItem*>(itemAt(viewport()->mapFromGlobal(pos)));
	if(!i)
		return false;

	// NAME <JID> (Node "NODE")
	//
	// Identities:
	// (icon) NAME (Category "CATEGORY"; Type "TYPE")
	// (icon) NAME (Category "CATEGORY"; Type "TYPE")
	//
	// Features:
	// NAME (http://jabber.org/feature)
	// NAME (http://jabber.org/feature)

	// top row
	QString text = "<qt><nobr>";
	DiscoItem item = i->item();

	if ( item.name()!=item.jid().full() )
		text += item.name() + " ";

	text += "&lt;" + item.jid().full() + "&gt;";

	if ( !item.node().isEmpty() )
		text += " (" + tr("Node") + " \"" + item.node() + "\")";

	text += "</nobr>";

	if ( !item.identities().isEmpty() || !item.features().list().isEmpty() )
		text += "<br>\n";

	// identities
	if ( !item.identities().isEmpty() ) {
		text += "<br>\n<b>" + tr("Identities:") + "</b>\n";

		DiscoItem::Identities::ConstIterator it = item.identities().begin();
		for ( ; it != item.identities().end(); ++it) {
			text += "<br>";
			PsiIcon icon( category2icon((*it).category, (*it).type) );
			if ( !icon.name().isEmpty() )
				text += "<icon name=\"" + icon.name() + "\"> ";
			text += (*it).name;
			text += " (" + tr("Category") + " \"" + (*it).category + "\"; " + tr("Type") + " \"" + (*it).type + "\")\n";
		}

		if ( !item.features().list().isEmpty() )
			text += "<br>\n";
	}

	// features
	if ( !item.features().list().isEmpty() ) {
		text += "<br>\n<b>" + tr("Features:") + "</b>\n";

		QStringList features = item.features().list();
		QStringList::ConstIterator it = features.begin();
		for ( ; it != features.end(); ++it) {
			Features f( *it );
			text += "\n<br>";
			if ( f.id() > Features::FID_None )
				text += f.name() + " (";
			text += *it;
			if ( f.id() > Features::FID_None )
				text += ")";
		}
	}

	QString errorInfo=i->getErrorInfo();
	if ( !errorInfo.isEmpty() ) {
		text += "<br>\n<br>\n<b>" + tr("Error:") + "</b>\n";
		text += errorInfo;
	}

	text += "</qt>";
	PsiToolTip::showText(pos, text, this);
	return true;
}

bool DiscoListView::eventFilter(QObject* o, QEvent* e)
{
	if (e->type() == QEvent::ToolTip && o->isWidgetType()) {
		QWidget*    w  = static_cast<QWidget*>(o);
		QHelpEvent* he = static_cast<QHelpEvent*>(e);
		maybeTip(w->mapToGlobal(he->pos()));
		return true;
	}
	return QTreeWidget::eventFilter(o, e);
}

//----------------------------------------------------------------------------
// DiscoDlg::Private
//----------------------------------------------------------------------------

class DiscoDlg::Private : public QObject
{
	Q_OBJECT

private:
	class ProtocolAction : public QAction
	{
	public:
		ProtocolAction(QString text, QString toolTip, QObject *parent, QSignalMapper *sm, int parm)
			: QAction(text, parent)
		{
			setText(text);
			setIconText(text);
	
			setCheckable(true);
			setToolTip(toolTip);
			connect(this, SIGNAL(activated()), sm, SLOT(map()));
			sm->setMapping(this, parm);
		}
	};

	// helper class to store browser history
	class History {
	private:
		QTreeWidgetItem *item;
	public:
		History(QTreeWidgetItem *it) {
			item = it;
		}

		~History() {
			if ( item )
				delete item;
		}

		QTreeWidgetItem *takeItem() {
			QTreeWidgetItem *i = item;
			item = 0;
			return i;
		}
	};

public: // data
	DiscoDlg *dlg;
	Jid jid;
	QString node;

	DiscoData data;

	QToolBar *toolBar;
	IconAction *actRegister, *actSearch, *actJoin, *actAHCommand, *actVCard, *actAdd;

	typedef Q3PtrList<History> HistoryList;
	HistoryList backHistory, forwardHistory;

	BusyWidget *busy;

public: // functions
	Private(DiscoDlg *parent, PsiAccount *pa);
	~Private();

public slots:
	void doDisco(QString host = QString::null, QString node = QString::null);

	void actionStop();

	void updateComboBoxes(Jid j, QString node);

	void itemUpdateStarted();
	void itemUpdateFinished();

	void disableButtons();
	void enableButtons(const DiscoItem &);

	void itemSelected ();
	void itemDoubleclicked (QTreeWidgetItem *, int column);
	bool eventFilter (QObject *, QEvent *);

	void setProtocol(int);

	// features...
	void actionActivated(int);

	void objectDestroyed(QObject *);

private:
	friend class DiscoListItem;
};

DiscoDlg::Private::Private(DiscoDlg *parent, PsiAccount *pa)
{
	dlg = parent;
	data.pa = pa;
	data.tasks = new TaskList;
	connect(data.tasks, SIGNAL(started()),  SLOT(itemUpdateStarted()));
	connect(data.tasks, SIGNAL(finished()), SLOT(itemUpdateFinished()));
	data.d = new DiscoConnector(this);
	connect(data.d, SIGNAL(itemUpdated()), SLOT(itemSelected ()));
	data.protocol = DiscoData::Auto;

	backHistory.setAutoDelete(true);
	forwardHistory.setAutoDelete(true);

	// mess with widgets
	busy = parent->busy;
	connect(busy, SIGNAL(destroyed(QObject *)), SLOT(objectDestroyed(QObject *)));

	QTreeWidget *lv_discoOld = dlg->lv_disco;
	dlg->lv_disco = new DiscoListView(dlg);
	replaceWidget(lv_discoOld, dlg->lv_disco);
	QPalette pal = dlg->lv_disco->palette();
	pal.setColor(QPalette::Highlight, pal.color(QPalette::Base));
	dlg->lv_disco->setPalette(pal);

	TransportDelegate *delegate = new TransportDelegate;
	dlg->lv_disco->setItemDelegate(delegate);

	dlg->lv_disco->installEventFilter (this);
	connect(dlg->lv_disco, SIGNAL(itemSelectionChanged ()), SLOT(itemSelected ()));;
	connect(dlg->lv_disco, SIGNAL(itemDoubleClicked (QTreeWidgetItem *, int )),    SLOT(itemDoubleclicked (QTreeWidgetItem *, int )));;

	// protocol actions
	QSignalMapper *pm = new QSignalMapper(this);
	connect(pm, SIGNAL(mapped(int)), SLOT(setProtocol(int)));
	QActionGroup *protocolActions = new QActionGroup (this);
	protocolActions->setExclusive(true);

	ProtocolAction *autoProtocol = NULL, *discoProtocol = NULL, *browseProtocol = NULL, *agentsProtocol = NULL;
	if (PsiOptions::instance()->getOption("options.ui.show-deprecated.service-discovery.protocol-selector").toBool()) {
		autoProtocol = new ProtocolAction (tr("Auto"), tr("Automatically determine protocol"), protocolActions, pm, DiscoData::Auto);
		discoProtocol = new ProtocolAction ("D", tr("Service Discovery"), protocolActions, pm, DiscoData::Disco);
		browseProtocol = new ProtocolAction ("B", tr("Browse Services"), protocolActions, pm, DiscoData::Browse);
		agentsProtocol = new ProtocolAction ("A", tr("Browse Agents"), protocolActions, pm, DiscoData::Agents);
		autoProtocol->setChecked(true);
	}

	// custom actions
	QSignalMapper *sm = new QSignalMapper(this);
	connect(sm, SIGNAL(mapped(int)), SLOT(actionActivated(int)));
	actRegister = new IconAction (tr("Register"), "psi/register", tr("&Register"), 0, dlg);
	connect (actRegister, SIGNAL(activated()), sm, SLOT(map()));
	sm->setMapping(actRegister, Features::FID_Register);
	actSearch = new IconAction (tr("Search"), "psi/search", tr("&Search"), 0, dlg);
	connect (actSearch, SIGNAL(activated()), sm, SLOT(map()));
	sm->setMapping(actSearch, Features::FID_Search);
	actJoin = new IconAction (tr("Join"), "psi/groupChat", tr("&Join"), 0, dlg);
	connect (actJoin, SIGNAL(activated()), sm, SLOT(map()));
	sm->setMapping(actJoin, Features::FID_Groupchat);
	actAHCommand = new IconAction (tr("Execute command"), "psi/command", tr("&Execute command"), 0, dlg);
	connect (actAHCommand, SIGNAL(activated()), sm, SLOT(map()));
	sm->setMapping(actAHCommand, Features::FID_AHCommand);
	actVCard = new IconAction (tr("vCard"), "psi/vCard", tr("&vCard"), 0, dlg);
	connect (actVCard, SIGNAL(activated()), sm, SLOT(map()));
	sm->setMapping(actVCard, Features::FID_VCard);
	actAdd = new IconAction (tr("Add to roster"), "psi/addContact", tr("&Add to roster"), 0, dlg);
	connect (actAdd, SIGNAL(activated()), sm, SLOT(map()));
	sm->setMapping(actAdd, Features::FID_Add);

	// create toolbar
	toolBar = new QToolBar(tr("Service Discovery toolbar"), dlg);
	toolBar->setIconSize(QSize(16,16));
	toolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	replaceWidget(dlg->toolBarPlaceholder, toolBar);

	toolBar->addAction(actRegister);
	toolBar->addAction(actSearch);
	toolBar->addAction(actJoin);
	toolBar->addAction(actAHCommand);

	toolBar->addSeparator();
	toolBar->addAction(actAdd);
	toolBar->addAction(actVCard);
	toolBar->addAction(actAHCommand);

	// select protocol
	if (PsiOptions::instance()->getOption("options.ui.show-deprecated.service-discovery.protocol-selector").toBool()) {
		toolBar->addSeparator();
		toolBar->addAction(autoProtocol);
		toolBar->addAction(discoProtocol);
		toolBar->addAction(browseProtocol);
		toolBar->addAction(agentsProtocol);
	}

	toolBar->addWidget(new StretchWidget(toolBar));
	AccountLabel* lb_ident = new AccountLabel(toolBar);
	lb_ident->setAccount(pa);
	lb_ident->setShowJid(false);
	toolBar->addWidget(lb_ident);

	// misc stuff
	disableButtons();
}

DiscoDlg::Private::~Private()
{
	delete data.tasks;
}

void DiscoDlg::Private::doDisco(QString _host, QString _node)
{
	PsiAccount *pa = data.pa;
	if ( !pa->checkConnected(dlg) )
		return;

	// Strip whitespace
	Jid j;
	QString host = _host;
	if ( host.isEmpty() )
		host = dlg->cb_address->currentText();
	j = host.stripWhiteSpace();
	if ( !j.isValid() )
		return;

	QString n = _node.stripWhiteSpace();

	jid  = j;
	node = n;

	updateComboBoxes(jid, node);

	data.tasks->clear(); // also will call all all necessary functions
	disableButtons();

	dlg->lv_disco->takeTopLevelItem(0);

	// create new root item
	DiscoItem di;
	di.setJid( jid );
	di.setNode( node );

	DiscoListItem *root = new DiscoListItem (di, &data, dlg->lv_disco);
	root->setHidden (true); // don't confuse users with empty root

	root->setOpen(true); // begin browsing
}

void DiscoDlg::Private::updateComboBoxes(Jid j, QString n)
{
	data.pa->psi()->recentBrowseAdd( j.full() );
	dlg->cb_address->clear();
	dlg->cb_address->insertStringList(data.pa->psi()->recentBrowseList());

	data.pa->psi()->recentNodeAdd( n );
}

void DiscoDlg::Private::actionStop()
{
	data.tasks->clear();
}

void DiscoDlg::Private::itemUpdateStarted()
{
	if ( busy )
		busy->start();
}

void DiscoDlg::Private::itemUpdateFinished()
{
	if ( busy )
		busy->stop();
}

void DiscoDlg::Private::disableButtons()
{
	DiscoItem di;
	enableButtons ( di );
}

void DiscoDlg::Private::enableButtons(const DiscoItem &it)
{
	bool itemSelected = !it.jid().full().isEmpty();

	// custom actions
	Features f = it.features();
	actRegister->setEnabled( f.canRegister() );
	actSearch->setEnabled( f.canSearch() );
	actJoin->setEnabled( f.canGroupchat() );
	actAdd->setEnabled( itemSelected );
	actVCard->setEnabled( f.haveVCard() );
	actAHCommand->setEnabled( f.canCommand() );
}

void DiscoDlg::Private::itemSelected ()
{
	QList<QTreeWidgetItem*> list = dlg->lv_disco->selectedItems();
	DiscoListItem *it = dynamic_cast<DiscoListItem *>(list.first());
	if ( !it ) {
		disableButtons();
		return;
	}

	it->itemSelected();
	if ( it->childCount() == 0) {
		it->updateItems(true);
		it->setExpanded( true );
	}
	

	const DiscoItem di = it->item();
	enableButtons ( di );
}

void DiscoDlg::Private::itemDoubleclicked (QTreeWidgetItem *item, int column)
{
	DiscoListItem *it = (DiscoListItem *)item;
	if ( !it )
		return;

	const DiscoItem d = it->item();
	const Features &f = d.features();

	// set the prior state of item
	// FIXME: causes minor flickering
	long id = 0;

	// trigger default action
	if (f.canCommand() ) {
		id = Features::FID_AHCommand;
	}
	if (!d.identities().isEmpty()) {
		// FIXME: check the category and type for JUD!
		DiscoItem::Identity ident = d.identities().first();
		bool searchFirst = ident.category == "service" && ident.type == "jud";

		if ( searchFirst && f.canSearch() ) {
			id = Features::FID_Search;
		}
		else if ( ident.category == "conference" &&  f.canGroupchat() ) {
			id = Features::FID_Groupchat;
		}
		else if ( f.canRegister() ) {
				id = Features::FID_Register;
		}
	}

	if ( id > 0 ) {
		it->updateItems(true);
		it->setExpanded( true );
		emit dlg->featureActivated( Features::feature(id), d.jid(), d.node() );
	}
}

bool DiscoDlg::Private::eventFilter (QObject *object, QEvent *event)
{
	if ( object == dlg->lv_disco ) {
		if ( event->type() == QEvent::ContextMenu ) {
			QContextMenuEvent *e = (QContextMenuEvent *)event;

			QList<QTreeWidgetItem*> list = dlg->lv_disco->selectedItems();
			DiscoListItem *it = dynamic_cast<DiscoListItem *>(list.first());
			if ( !it )
				return true;

			// prepare features list
			QList<long> idFeatures;
			QStringList features = it->item().features().list();
			{	// convert all features to their IDs
				QStringList::Iterator it = features.begin();
				for ( ; it != features.end(); ++it) {
					Features f( *it );
					if ( f.id() > Features::FID_None )
						idFeatures.append( Features::id(*it) );
				}
				//qHeapSort(idFeatures);
			}

			QList<long> ids;
			{	// ensure, that there's in no duplicated IDs inside. FIXME: optimize this, anyone?
				long id = 0, count = 0;
				QList<long>::Iterator it;
				while ( count < (long)idFeatures.count() ) {
					bool found = false;

					for (it = idFeatures.begin(); it != idFeatures.end(); ++it) {
						if ( id == *it ) {
							if ( !found ) {
								found = true;
								ids.append( id );
							}
							count++;
						}
					}
					id++;
				}
			}

			// prepare popup menu
			Q3PopupMenu p;

			// custom actions
			p.insertSeparator();
			actRegister->addTo(&p);
			actSearch->addTo(&p);
			actJoin->addTo(&p);

			p.insertSeparator();
			actAdd->addTo(&p);
			actVCard->addTo(&p);
			actAHCommand->addTo(&p);

			// popup with all available features
			Q3PopupMenu *fm = new Q3PopupMenu(&p);
			{
				QList<long>::Iterator it = ids.begin();
				for ( ; it != ids.end(); ++it)
					fm->insertItem(Features::name(*it), *it + 10000); // TODO: add pixmap
			}

			//p.insertSeparator();
			//int menuId = p.insertItem(tr("Activate &Feature"), fm);
			//p.setItemEnabled(menuId, !ids.isEmpty());

			// display popup
			e->accept();
			int r = p.exec ( e->globalPos() );

			if ( r > 10000 )
				actionActivated(r-10000);

			return true;
		}
	}

	return false;
}

void DiscoDlg::Private::actionActivated(int id)
{
	QList<QTreeWidgetItem*> list = dlg->lv_disco->selectedItems();
	DiscoListItem *it = dynamic_cast<DiscoListItem *>(list.first());
	if ( !it )
		return;

	emit dlg->featureActivated(Features::feature(id), it->item().jid(), it->item().node());
}

void DiscoDlg::Private::objectDestroyed(QObject *obj)
{
	if ( obj == busy )
		busy = 0;
}

void DiscoDlg::Private::setProtocol(int p)
{
	data.protocol = (DiscoData::Protocol)p;
}

//----------------------------------------------------------------------------
// DiscoDlg
//----------------------------------------------------------------------------

DiscoDlg::DiscoDlg(PsiAccount *pa, const Jid &jid, const QString &node)
	: QDialog(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
  	setupUi(this);

	// initialize
	d = new Private(this, pa);
	d->jid  = jid;
	d->node = node;

	setWindowTitle(CAP(caption()));
	setWindowIcon(PsiIconset::instance()->transportStatus("transport", STATUS_ONLINE).icon());
	X11WM_CLASS("disco");

	connect (pb_browse, SIGNAL(clicked()), d, SLOT(doDisco()));
	pb_browse->setDefault(false);
	pb_browse->setAutoDefault(false);

	connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
	pb_close->setDefault(false);
	pb_close->setAutoDefault(false);

	cb_address->insertStringList(pa->psi()->recentBrowseList()); // FIXME
	cb_address->setFocus();
	connect(cb_address, SIGNAL(activated(const QString &)), d, SLOT(doDisco()));
	cb_address->setCurrentText(d->jid.full());

	if ( pa->loggedIn() )
		doDisco();
}

DiscoDlg::~DiscoDlg()
{
	delete d;
}

void DiscoDlg::doDisco(QString host, QString node)
{
	d->doDisco(host, node);
}

#include "discodlg.moc"
