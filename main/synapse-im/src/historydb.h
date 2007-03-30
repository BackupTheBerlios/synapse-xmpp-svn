//
// C++ Interface: historydb
//
// Description: Class to access history database.
//
//
// Author: Andrzej Wójcik <andrzej@hi-low.eu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef HISTORYDB_H
#define HISTORYDB_H
#include <QTreeWidget>
#include <QMessageBox>
#include <QtSql>

#include "psievent.h"
#include "historydlg.h"

class HistoryDB : public QObject
{
	Q_OBJECT
public:
	QTreeWidgetItem *getDates(HistoryDlg *dlg,QTreeWidget *dateTree,QString j, QDate selected, QString searchFor="");
	HistoryItem *getEvents(QTreeWidget *eventsTree,QString j, QString date, QString searchFor="");
	static HistoryDB *instance();
	bool logEvent(QString j, PsiEvent *e);
	void exportHistory(PsiAccount *pa, XMPP::Jid jid, QString path, QDate selectedDate);
private:
	HistoryDB();
	~HistoryDB();

	QString getTableName(QString j);
	QStringList tablesList();
	void createIndex();
	void createJidTable(QString j);

private:
	QSqlDatabase db;
	QStringList tablesList_;
	static HistoryDB *instance_;
};
#endif
