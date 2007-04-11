//
// C++ Implementation: historydb
//
// Description: Class to access history database.
//
//
// Author: Andrzej Wójcik <andrzej@hi-low.eu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <QObject>
#include <QDateTime>
#include <QVariant>
#include "historydb.h"
#include "historydlg.h"
#include "profiles.h"
#include "psiiconset.h"
#include "im.h"
#include "jidutil.h"
#include "psiaccount.h"
#include "userlist.h"

static QString getNext(QString *str)
{
	// are we in space?
	int n = 0;
	if(str->at(n).isSpace()) {
		// get out of it
		while(n < (int)str->length() && str->at(n).isSpace())
			++n;
		if(n == (int)str->length())
			return QString::null;
	}
	// find end or next space
	while(n < (int)str->length() && !str->at(n).isSpace())
		++n;
	QString result = str->mid(0, n);
	*str = str->mid(n);
	return result;
}

// wraps a string against a fixed width
static QStringList wrapString(const QString &str, int wid)
{
	QStringList lines;
	QString cur;
	QString tmp = str;
	//printf("parsing: [%s]\n", tmp.latin1());
	while(1) {
		QString word = getNext(&tmp);
		if(word == QString::null) {
			lines += cur;
			break;
		}
		//printf("word:[%s]\n", word.latin1());
		if(!cur.isEmpty()) {
			if((int)cur.length() + (int)word.length() > wid) {
				lines += cur;
				cur = "";
			}
		}
		if(cur.isEmpty()) {
			// trim the whitespace in front
			for(int n = 0; n < (int)word.length(); ++n) {
				if(!word.at(n).isSpace()) {
					if(n > 0)
						word = word.mid(n);
					break;
				}
			}
		}
		cur += word;
	}
	return lines;
}


HistoryDB::HistoryDB()
: QObject(0)
{
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(pathToProfile(activeProfile) + "/history.db");
	if (!db.open()) {
		QMessageBox::critical(0, qApp->tr("Cannot open database"),
		qApp->tr("Unable to establish a database connection.\n"
			"This example needs SQLite support. Please read "
			"the Qt SQL driver documentation for information how "
			"to build it.\n\n"
			"Click Cancel to exit."), QMessageBox::Cancel);
	} else {
		tablesList_ = tablesList();
		if (!tablesList_.contains("tablesList"))
			createIndex();
	}
}

HistoryDB::~HistoryDB()
{
	db.close();
}

void HistoryDB::createIndex()
{
	QSqlQuery inquery("CREATE TABLE tablesList VALUES ('name' TEXT)");
	QSqlQuery tnquery("INSERT INTO tablesList VALUES ('tableList')");
	tablesList_ = tablesList();
}

QStringList HistoryDB::tablesList()
{
	QStringList tl;
	QSqlQueryModel query;
	query.setQuery("SELECT name FROM tablesList");
	for(int i=0; i< query.rowCount(); i++)
	{
		tl << query.record(i).value("name").toString();
	}
	return tl;
}

void HistoryDB::createJidTable(QString j)
{
	QSqlQuery tquery("CREATE TABLE '" + j + "' ('type' TEXT, 'origin' TEXT, 'date' TEXT, 'time' TEXT, 'text' TEXT, 'html' TEXT);");
	QSqlQuery tnquery("INSERT INTO tablesList VALUES ('" + j +"')");
	tablesList_ = tablesList();
}

bool HistoryDB::logEvent(QString j, PsiEvent *e)
{
	j = getTableName(j);
	if(!tablesList_.contains(j))
		createJidTable(j);

	if(e->type() == PsiEvent::Message) 
	{
		MessageEvent *me = (MessageEvent *)e;
		const Message &m = me->message();
		QString html("");
		if (m.containsHTML())
			html = m.html().toString();
		QSqlQuery query("INSERT INTO '" + j + "' VALUES ('" + m.type() + "','" + (e->originLocal() ? "to" : "from") + "','" + m.timeStamp().date().toString() + "','" + m.timeStamp().time().toString() + "','" + m.body() + "','" + html + "')");
	} 
	else if(e->type() == PsiEvent::Auth) 
	{
		AuthEvent *ae = (AuthEvent *)e;	
		QSqlQuery query("INSERT INTO '" + j + "' VALUES ('" + ae->authType() + "','" + (e->originLocal() ? "to" : "from") + "','" + ae->timeStamp().date().toString() + "','" + ae->timeStamp().time().toString() + "','" + "" + "','" + "" + "')");
	}
	return true;
}

QString HistoryDB::getTableName(QString j)
{
	j = j.replace(QChar('@'),"_");
	j = j.replace(QChar('.'),"_");
	return "JID"+j;
}

QTreeWidgetItem *HistoryDB::getDates(HistoryDlg *dlg, QTreeWidget *dateTree, QString j, QDate selected, QString searchFor)
{
	QSqlQueryModel query;
	QString tableName;
	tableName = getTableName(j);
 	query.setQuery(QString("SELECT date") + ((searchFor.isEmpty())?"":",text") +" FROM " + tableName + " WHERE date LIKE '%" + selected.shortMonthName(selected.month()) + "%" + QVariant(selected.year()).toString() +"%' ORDER BY date");
	while (query.canFetchMore())
     		query.fetchMore();
	DateItem *last = NULL;
	QColor red(255,0,0);
	for(int i=0; i< query.rowCount(); i++)
	{
		QDate date;
		date = date.fromString(query.record(i).value("date").toString());
		if ((last == NULL) || (last->date() != date))
		{
			DateItem *item = new DateItem(date);
			connect(dateTree,SIGNAL(itemClicked(QTreeWidgetItem *, int)),dlg,SLOT(dateSelected(QTreeWidgetItem*,int)));
			dateTree->addTopLevelItem(item);
			if(date == selected)
				dateTree->setCurrentItem(item);
			
			last = item;
		}
		if(!searchFor.isEmpty() && query.record(i).value("text").toString().contains(searchFor))
			last->setTextColor(0, red);
	}
	return NULL;
}

QTreeWidgetItem *HistoryDB::getDatesMatching(HistoryDlg *dlg, QTreeWidget *dateTree, QString j, QString searchFor)
{
	QSqlQueryModel query;
	QString tableName;
	tableName = getTableName(j);
	query.setQuery("SELECT date,text FROM " + tableName + " WHERE text LIKE '%"+searchFor+"%' ORDER BY date LIMIT 50");
	while (query.canFetchMore())
		query.fetchMore();

	DateItem *last = NULL;
	QColor red(255,0,0);
	for(int i=0; i< query.rowCount(); i++)
	{
		QDate date;
		date = date.fromString(query.record(i).value("date").toString());
		if ((last == NULL) || (last->date() != date))
		{
			DateItem *item = new DateItem(date);
			connect(dateTree,SIGNAL(itemClicked(QTreeWidgetItem *, int)),dlg,SLOT(dateSelected(QTreeWidgetItem*,int)));
			dateTree->addTopLevelItem(item);
			last = item;
		}
		if(!searchFor.isEmpty() && query.record(i).value("text").toString().contains(searchFor))
			last->setTextColor(0, red);
	}
	return NULL;
}

HistoryItem *HistoryDB::getEvents(QTreeWidget *eventsTree, QString j, QString date, QString searchFor)
{
	QString tableName;
	tableName = getTableName(j);
	QSqlQueryModel query;
	query.setQuery("SELECT type,origin,time,text,html FROM " + tableName + " WHERE date='"+date+"' ORDER BY time");
	for(int i=0; i< query.rowCount(); i++)
	{
		QString icon;
		if(query.record(i).value("type").toString() == "headline")
			icon = "psi/headline";
		else if(query.record(i).value("type").toString() == "chat")
			icon = "psi/chat";
		else if(query.record(i).value("type").toString() == "error")
			icon = "psi/system";
		else
			icon = "psi/message";
		QTime  time;
		HistoryItem *item = new HistoryItem();
		item->setIcon(0, QIcon(IconsetFactory::icon(icon).impix()));
		item->setText(1, query.record(i).value("time").toString());
		item->setText(2, query.record(i).value("origin").toString());
		item->setText(3, query.record(i).value("text").toString());
		if(!searchFor.isEmpty() && query.record(i).value("text").toString().contains(searchFor))
		{
			QColor red(255,0,0);
			item->setTextColor(1, red);
			item->setTextColor(2, red);
			item->setTextColor(3, red);
		}
		eventsTree->addTopLevelItem(item);
	}
	return NULL;
}

void HistoryDB::exportHistory(PsiAccount *pa, XMPP::Jid jid, QString path, QDate date)
{
	QFile f(path);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		QMessageBox::information(0, tr("Error"), tr("Error writing to file."));
		return;
	}
	QTextStream stream(&f);

	bool html_export = path.endsWith(".html",Qt::CaseInsensitive);

	QString tableName = getTableName(jid.bare());

	QString us = pa->nick().toUtf8();
	UserListItem *u = pa->findFirstRelevant(jid);
	QString them = JIDUtil::nickOrJid(u->name().toUtf8(), u->jid().full());

	if (html_export) {
		QString tmp;
		tmp += QString("<html><head>\n");
		tmp += QString("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
		tmp += QString("<meta http-equiv=\"Content-Style-Type\" content=\"text\\css\">\n");
		tmp += QString("<style type=\"text/css\">\n");
		tmp += QString("<!--\n");
		tmp += QString("div.design { margin: 0; background: #c8c8c8; }\n");
		tmp += QString("div.page { margin: 0; display: block; width: 800px; margin-left: 10%; margin-right: 10%; color: black; }\n");
		tmp += QString("div.us { background: #FF0000 }\n");
		tmp += QString("div.them { background: #0000FF; }\n");
		tmp += QString("-->\n</style>\n</head>\n<body><div class=\"design\"><div class=\"page\">\n");
		stream << tmp << endl;
	}
	QSqlQueryModel query;
	if(!date.isNull())
	{
		query.setQuery("SELECT type,origin,date,time,text,html FROM " + tableName + " WHERE date='"+date.toString()+"' ORDER BY time");
	}
	else
		query.setQuery("SELECT * FROM " + tableName + " ORDER BY date AND time");
	for(int i=0; i< query.rowCount(); i++)
	{
		date = date.fromString(query.record(i).value("date").toString());
		QDateTime dt;
		dt.setDate(date);
		QTime t;
		dt.setTime(t.fromString(query.record(i).value("time").toString()));
		QString ts;
		ts = dt.toString(Qt::SystemLocaleDate);

		QString nick;
		if(query.record(i).value("origin").toString() == "to")
			nick = us;
		else
			nick = them;

		if (html_export) {
			if (nick == us) ts = QString("<div class=\"us\"><b>(") + ts + QString(")</b>");
			if (nick == them) ts = QString("<div class=\"them\"><b>(") + ts + QString(")</b>");
		} else 
			ts = QString("(%1)").arg(ts);

		QString heading = QString("%1 ").arg(ts) + nick + ": ";

		QString type = query.record(i).value("type").toString();
		if((type == "headline") || (type == "chat") || (type == ""))
		{
			stream << heading << endl;
			QString txt;
			QString body = query.record(i).value("text").toString();

			QStringList lines = QStringList::split('\n', body, true);
			for(QStringList::ConstIterator lit = lines.begin(); lit != lines.end(); ++lit) {
				QStringList sub = wrapString(*lit, 72);
				for(QStringList::ConstIterator lit2 = sub.begin(); lit2 != sub.end(); ++lit2)
					txt += QString("    ") + *lit2 + '\n';
			}
			stream << txt.toUtf8() << endl;
			if (html_export) {
				QString tmp = QString("</div>");
				stream << tmp << endl;
			}
		}
	}
	if (html_export) {
		QString txt;
		txt += QString("</div></div></body></html>");
		stream << txt << endl;
	}
	f.close();
}


HistoryDB *HistoryDB::instance()
{
	if(instance_ == NULL)
		instance_ = new HistoryDB();
	return instance_;
}

HistoryDB* HistoryDB::instance_ = NULL;
