//
// C++ Implementation: historydlg.cpp
//
// Description: a dialog to show events history.
//
//
// Author: Andrzej W�cik <andrzej@hi-low.eu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "historydlg.h"

#include <q3popupmenu.h>
#include <q3header.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <QFileDialog>
#include <qmessagebox.h>
#include <QFrame>
#include <qapplication.h>
#include <qclipboard.h>
#include <QTextStream>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include "psiaccount.h"
#include "psievent.h"
#include "busywidget.h"
#include "applicationinfo.h"
#include "common.h"
#include "psiiconset.h"
#include "textutil.h"
#include "jidutil.h"
#include "userlist.h"

#include "historydb.h"

DateItem::DateItem(QDate date)
{
	date_ = date;
	setText(0,date.toString(Qt::SystemLocaleDate));
	QString sort = QString(date.year()* 10000 +date.month()*100 + date.day());
	setText(1,sort);
}

QDate DateItem::date()
{
	return date_;
}

HistoryItem::HistoryItem()
{
}

HistoryDlg::HistoryDlg(const XMPP::Jid& j, PsiAccount* pa)
: pa_(pa), jidFull_(j)
{
	setupUi(this);
	setModal(false);
	setAttribute(Qt::WA_DeleteOnClose);
	pa_->dialogRegister(this, jidFull_);
	setWindowTitle(tr("History for ") + j.full());
#ifndef Q_WS_MAC
	setWindowIcon(IconsetFactory::icon("psi/history").icon());
#endif


	DateTree->setHeaderLabel(tr("Date"));
	DateTree->setSortingEnabled(true);
	DateTree->setColumnHidden(1,true);

	EventsTree->setColumnCount(4);
	QStringList headers;
	headers << tr("Type") << tr("Time") << tr("Origin") << tr("Text");
	EventsTree->setHeaderLabels(headers);
	EventsTree->sortItems(1,Qt::AscendingOrder);
	EventsTree->setSortingEnabled(true);

	connect(EventsTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(actionOpenEvent(QTreeWidgetItem *, int)));
	connect(tb_previousMonth, SIGNAL(clicked()), SLOT(doPrev()));
	connect(tb_latest, SIGNAL(clicked()), SLOT(doLatest()));
	connect(tb_nextMonth, SIGNAL(clicked()), SLOT(doNext()));
	connect(pb_find, SIGNAL(clicked()), SLOT(doFind()));
	connect(pb_export, SIGNAL(clicked()), SLOT(doExport()));
	connect(pb_close, SIGNAL(clicked()), SLOT(close()));

	jid_ = j.bare();
	doLatest();

	EventsTree->resizeColumnToContents(0);
	EventsTree->resizeColumnToContents(1);
	EventsTree->resizeColumnToContents(2);

	X11WM_CLASS("history");
}

HistoryDlg::~HistoryDlg()
{
	pa_->dialogUnregister(this);
}

void HistoryDlg::loadPage(QString date,QString searchFor)
{
	EventsTree->clear();
	HistoryDB *h = HistoryDB::instance();
	h->getEvents(EventsTree,jid_,date,searchFor);
}

void HistoryDlg::dateSelected(QTreeWidgetItem *item, int column)
{
	lookDate = ((DateItem*)item)->date();
	loadPage(lookDate.toString(),findText);
}

void HistoryDlg::actionOpenEvent(QTreeWidgetItem *item, int column)
{
	Message m;
	QString sTime;
	sTime = ((DateItem*)DateTree->currentItem())->date().toString(Qt::ISODate) + "T" + item->text(1);
	m.setTimeStamp(QDateTime::fromString(sTime, Qt::ISODate));
	m.setFrom(jid_);
	m.setType(item->text(0));
	m.setBody(item->text(3));
	m.setSpooled(true);

	MessageEvent *me = new MessageEvent(m, 0);
	me->setOriginLocal((item->text(2) == "to") ? true : false);
	openEvent(me);
}

void HistoryDlg::doMonths()
{
	HistoryDB *h = HistoryDB::instance();
	DateTree->clear();
	h->getDates(this, DateTree, jid_,lookDate, findText);
	DateTree->sortByColumn(1,Qt::DescendingOrder);
	DateItem *di = (DateItem*)DateTree->takeTopLevelItem(0);
	if(di)
	{
		lookDate = di->date();
		DateTree->insertTopLevelItem(0,di);
		DateTree->setCurrentItem(di);
		loadPage(lookDate.toString(),findText);
	}
}

void HistoryDlg::doNext()
{
	lookDate = lookDate.addMonths(1);
	doMonths();
}

void HistoryDlg::doLatest()
{
	lookDate = QDate::currentDate();
	doMonths();
}

void HistoryDlg::doPrev()
{
	lookDate = lookDate.addMonths(-1);
	doMonths();
}

void HistoryDlg::doFind()
{
	DateTree->clear();
	findText = le_find->text();
	HistoryDB *h = HistoryDB::instance();
	h->getDatesMatching(this, DateTree, jid_, findText);
	DateItem *di = (DateItem*)DateTree->takeTopLevelItem(0);
	if(di)
	{
		DateTree->insertTopLevelItem(0,di);
		DateTree->setCurrentItem(di);
		lookDate = di->date();
		loadPage(lookDate.toString(),findText);
	}
}

void HistoryDlg::doExport()
{
	if(option.lastSavePath.isEmpty())
		option.lastSavePath = QDir::homeDirPath();

 	UserListItem *u = pa_->findFirstRelevant(jid_);
	QString them = JIDUtil::nickOrJid(u->name(), u->jid().full());
	QString s = JIDUtil::encode(them).toLower();

	QString str = option.lastSavePath + "/" + s + ".txt";
	int y = QMessageBox::information(this, tr("Export"), tr("Export all history of chats or just from selected day?"), tr("&All"), tr("&Day"));
	while(1) {
		str = QFileDialog::getSaveFileName(this, tr("Export message history"), str, tr("Text files (*.txt);;Html files (*.html);;All files (*.*)"));
		if(str.isEmpty())
			break;

		QFileInfo fi(str);
		if(fi.exists()) {
			int x = QMessageBox::information(this, tr("Confirm overwrite"), tr("File already exists, overwrite?"), tr("&Yes"), tr("&No"));
			if(x != 0)
				continue;
		}

		option.lastSavePath = fi.dirPath();
		QDate date;
		if(y!=0)
			date = lookDate;
		HistoryDB::instance()->exportHistory(pa_, jidFull_, str, date);
		break;
	}
}

#include "moc_historydlg.cpp"
