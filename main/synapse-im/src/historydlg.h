//
// C++ Implementation: historydlg.h
//
// Description: a dialog to show events history.
//
//
// Author: Andrzej Wójcik <andrzej@hi-low.eu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef HISTORYDLG_H
#define HISTORYDLG_H

#include <q3listview.h>
#include <q3simplerichtext.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QDateTime>
#include "xmpp.h"
#include "psievent.h"

#include "ui_historydlgui.h"

using namespace XMPP;

class PsiEvent;
class PsiAccount;
class EDBItem;
class EDBResult;

class DateItem : public QTreeWidgetItem
{
public:
	DateItem(QDate date);

	QDate date();

private:
	QDate date_;
};

class HistoryItem : public QTreeWidgetItem
{
public:
	HistoryItem();
};

class HistoryDlg : public QDialog, public Ui::History
{
	Q_OBJECT
public:
	HistoryDlg(const XMPP::Jid& j, PsiAccount* pa);
	~HistoryDlg();

	void loadPage(QString date, QString searchFor="");

public slots:
	void dateSelected(QTreeWidgetItem *item, int column);
	void actionOpenEvent(QTreeWidgetItem *item, int column);
	void doPrev();
	void doLatest();
	void doNext();
	void doFind();
	void doExport();

signals:
	void openEvent(PsiEvent *e);

private:
	void doMonths();

	PsiAccount *pa_;
	XMPP::Jid jidFull_;
	QString jid_;
	QString findText;
	QDate lookDate;
};

#endif

