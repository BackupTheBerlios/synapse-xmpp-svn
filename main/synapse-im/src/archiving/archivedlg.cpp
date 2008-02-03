#include "archivedlg.h"

#include <QTextCharFormat>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QMessageBox>

#include "psiaccount.h"
#include "getcollectionlisttask.h"
#include "getcollectiontask.h"
#include "xmpp_client.h"
#include "psiiconset.h"

ArchiveDlg::ArchiveDlg(const XMPP::Jid &jid, PsiAccount *pa)
{
	max = 30;
	page_ = 0;
	setupUi(this);
	tw_log->setColumnCount(3);
	QStringList headers;
	headers << tr("Time") << tr("") << tr("Message");
	tw_log->setHeaderLabels(headers);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("Archive for ") + jid.full());
#ifndef Q_WS_MAC
	setWindowIcon(IconsetFactory::icon("psi/history").icon());
#endif

	gct_ = NULL;

	pa_ = pa;
	jid_ = jid;

	last_ =  QDateTime::currentDateTime();

	connect(pb_close, SIGNAL(clicked()), this, SLOT(hide()));

	connect(calendar, SIGNAL(selectionChanged()), this, SLOT(dateSelected()));
	connect(calendar, SIGNAL(currentPageChanged(int, int)), this, SLOT(dateChanged(int, int)));

	connect(lw_conversations, SIGNAL(itemSelectionChanged()), this, SLOT(collectionSelected()));
	connect(lw_conversations, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(doCollectionContextMenu(const QPoint&)));
	lw_conversations->

	connect(tb_prevPage, SIGNAL(clicked()), this, SLOT(prevPage()));	connect(tb_nextPage, SIGNAL(clicked()), this, SLOT(nextPage()));

	gclt_ = new GetCollectionListTask(pa->client()->rootTask());
	connect(gclt_, SIGNAL(busy()), this, SLOT(busy()));
	connect(gclt_, SIGNAL(done()), this, SLOT(collectionListRetrieved()));
	connect(gclt_, SIGNAL(error()), this, SLOT(error()));

	gct_ = new GetCollectionTask(pa_->client()->rootTask());
	connect(gct_, SIGNAL(busy()), this, SLOT(busy()));
	connect(gct_, SIGNAL(done(int)), this, SLOT(collectionRetrieved(int)));
	connect(gct_, SIGNAL(msg(int, bool, const QString&)), this, SLOT(collectionMsg(int, bool, const QString&)));
	connect(gct_, SIGNAL(error()), this, SLOT(error()));
	
	QDate m(last_.date().year(), last_.date().month(), 1);
	gclt_->get(jid, m, 50);
//	X11WM_CLASS("history");
}

ArchiveDlg::~ArchiveDlg()
{
	delete gclt_;
	delete gct_;
}

void ArchiveDlg::dateChanged(int year, int month)
{
	lw_conversations->clear();
	QDate m(year, month, 1);
	gclt_->get(jid_, m, 50);
}

void ArchiveDlg::dateSelected()
{
	lw_conversations->clear();
	tw_log->clear();
	gclt_->get(jid_, calendar->selectedDate(), 50);
}

void ArchiveDlg::collectionSelected()
{
	if(lw_conversations->selectedItems().isEmpty())
		return;

	if(lw_conversations->selectedItems().isEmpty())
		return;

	QListWidgetItem *item = (*lw_conversations->selectedItems().begin());
	if(item) {
		if(gct_ == NULL) {
			gct_ = new GetCollectionTask(pa_->client()->rootTask());
			connect(gct_, SIGNAL(busy()), this, SLOT(busy()));
			connect(gct_, SIGNAL(done(int)), this, SLOT(collectionRetrieved(int)));
			connect(gct_, SIGNAL(msg(int, bool, const QString&)), this, SLOT(collectionMsg(int, bool, const QString&)));
		}
		tw_log->clear();
		last_ = QDateTime::fromString(item->data(Qt::UserRole).toString().left(15), Qt::ISODate);
		gct_->get(jid_, item->data(Qt::UserRole).toString(), 0, max);
	}

}

void ArchiveDlg::doCollectionContextMenu(const QPoint &pos)
{
	QMenu cm;
	QAction* _open = cm.addAction(tr("Open"));
	QAction* _delete = cm.addAction(tr("Delete"));

	QListWidgetItem *item = (*lw_conversations->selectedItems().begin());

	if(item) {
		QAction* x = cm.exec(lw_conversations->mapToGlobal(pos));

		if(gct_ == NULL) {
			gct_ = new GetCollectionTask(pa_->client()->rootTask());
			connect(gct_, SIGNAL(busy()), this, SLOT(busy()));
			connect(gct_, SIGNAL(done(int)), this, SLOT(collectionRetrieved(int)));
			connect(gct_, SIGNAL(msg(int, bool, const QString&)), this, SLOT(collectionMsg(int, bool, const QString&)));
		}

		tw_log->clear();
		last_ = QDateTime::fromString(item->data(Qt::UserRole).toString().left(15), Qt::ISODate);

		if(x == _open) {
			gct_->get(jid_, item->data(Qt::UserRole).toString(), 0, max);
			//actionOpenEvent(EventsTree->currentItem());
		}
		else if (x == _delete) {
			gct_->deleteCollection(jid_, item->data(Qt::UserRole).toString());
			QDate m(last_.date().year(), last_.date().month(), 1);
			lw_conversations->clear();
			gclt_->get(jid_, m, 50);
		}
	}
}

void ArchiveDlg::prevPage()
{
	page_ = page_ - 1;
	tw_log->clear();

	QListWidgetItem *item = lw_conversations->selectedItems().first();
	if(gct_ == NULL) {
		gct_ = new GetCollectionTask(pa_->client()->rootTask());
		connect(gct_, SIGNAL(busy()), this, SLOT(busy()));
		connect(gct_, SIGNAL(done(int)), this, SLOT(collectionRetrieved(int)));
		connect(gct_, SIGNAL(msg(int, bool, const QString&)), this, SLOT(collectionMsg(int, bool, const QString&)));
	}
	last_ = QDateTime::fromString(item->data(Qt::UserRole).toString().left(15), Qt::ISODate);
	gct_->get(jid_, item->data(Qt::UserRole).toString(), page_, max);
}

void ArchiveDlg::nextPage()
{
	page_ = page_ + 1;
	tw_log->clear();

	QListWidgetItem *item = lw_conversations->selectedItems().first();
	if(gct_ == NULL) {
		gct_ = new GetCollectionTask(pa_->client()->rootTask());
		connect(gct_, SIGNAL(busy()), this, SLOT(busy()));
		connect(gct_, SIGNAL(done(int)), this, SLOT(collectionRetrieved(int)));
		connect(gct_, SIGNAL(msg(int, bool, const QString&)), this, SLOT(collectionMsg(int, bool, const QString&)));
	}
	last_ = QDateTime::fromString(item->data(Qt::UserRole).toString().left(15), Qt::ISODate);
	gct_->get(jid_, item->data(Qt::UserRole).toString(), page_, max);
}

void ArchiveDlg::busy()
{
	progressBar->setMinimum(0);
	progressBar->setMaximum(0);
}

void ArchiveDlg::collectionListRetrieved()
{
	progressBar->setMaximum(100);
	progressBar->setValue(100);

	int i = 0;
	QList<Collection> collections = gclt_->collections();
	QTextCharFormat chf;
	chf.setFontWeight(75);
	QList<Collection>::iterator it;
	for(it = collections.begin(); it != collections.end(); ++it) {
		i++;
		calendar->setDateTextFormat((*it).date().date(), chf);
		if(calendar->selectedDate() == (*it).date().date()) {
			QListWidgetItem *item = new QListWidgetItem((*it).date().time().toString());
			item->setData(Qt::UserRole, (*it).start());
			lw_conversations->addItem(item);
		}
	}
	i++;
}

void ArchiveDlg::error()
{
	QMessageBox::critical(0, "Error", "Error while trying to use\none of XEP-0136 features.");
}

void ArchiveDlg::collectionMsg(int sec, bool direction, const QString &body)
{
	QDateTime time = last_.addSecs(sec);
	time.setTimeSpec(Qt::UTC);
	QStringList sl;
	sl << time.toLocalTime().time().toString() << (direction ? "from" : "to" ) << body;
	QTreeWidgetItem *item = new QTreeWidgetItem(sl);
	tw_log->addTopLevelItem(item);
	tw_log->resizeColumnToContents(0);
	tw_log->resizeColumnToContents(1);
}

void ArchiveDlg::collectionRetrieved(int count)
{
	progressBar->setMaximum(100);
	progressBar->setValue(100);
}

