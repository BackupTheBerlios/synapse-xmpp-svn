//
// C++ Interface: historydb
//
// Description: Class to access history database.
//
//
// Author: Andrzej W�jcik <andrzej@hi-low.eu>, (C) 2007
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
	enum Backend {
		SQLITE,
		POSTGRES,
		MYSQL,
		ODBC
	};
	void getDates(HistoryDlg *dlg,QTreeWidget *dateTree,QString j,int from, int count);
	QTreeWidgetItem *getDatesMatching(HistoryDlg *dlg, QTreeWidget *dateTree, QString j, QString searchFor);

	HistoryItem *getEvents(QTreeWidget *eventsTree,QString j, QDate date, QString searchFor="");
	static HistoryDB *instance();
	bool logEvent(QString j, PsiEvent *e);
	void exportHistory(PsiAccount *pa, XMPP::Jid jid, QString path, QDate selectedDate);

	void deleteEvents(QString j,QDate date,QTime time);

public slots:
	void optionsUpdate();

private:
	HistoryDB();
	~HistoryDB();

	void ensureDate(const QString &j, const QDate &date);
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
