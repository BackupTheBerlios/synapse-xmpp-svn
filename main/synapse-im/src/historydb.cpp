//
// C++ Implementation: historydb
//
// Description: Class to access history database.
//
//
// Author: Andrzej W�cik <andrzej@hi-low.eu>, (C) 2007
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
#include "filetransfer.h"
#include "common.h"
#include "qca.h"
#include "psioptions.h"

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
	optionsUpdate();
}

HistoryDB::~HistoryDB()
{
	db.close();
}

void HistoryDB::optionsUpdate()
{
	db.close();
	
	QString backend = PsiOptions::instance()->getOption("options.history.backend").toString();
	if(backend.compare("Postgres") == 0)
		db = QSqlDatabase::addDatabase("QPSQL");
	else if(backend.compare("MySQL") == 0)
		db = QSqlDatabase::addDatabase("QMYSQL");
	else if(backend.compare("ODBC") == 0)
		db = QSqlDatabase::addDatabase("QODBC");
	else {
		db = QSqlDatabase::addDatabase("QSQLITE");
		db.setDatabaseName(pathToProfile(activeProfile) + "/history_v2.db");
	}

	if (backend.compare("SQLite 3") != 0) {
		db.setDatabaseName(PsiOptions::instance()->getOption("options.history.database").toString());
		db.setUserName(PsiOptions::instance()->getOption("options.history.user").toString());
		db.setPassword(PsiOptions::instance()->getOption("options.history.password").toString());
		db.setPort(PsiOptions::instance()->getOption("options.history.port").toInt());
		db.setHostName(PsiOptions::instance()->getOption("options.history.host").toString());
	}

	if (!db.open()) {
		QMessageBox::critical(0, tr("Cannot open database"),
		tr("Unable to establish a database connection.\n"
			"Possible selected database backend is not available or misconfigured.\nPlease read "
			"the Qt SQL driver documentation for information how "
			"to build it.\n\n"
			"Click Cancel to exit."), QMessageBox::Cancel);
	} else {
		tablesList_ = tablesList();
		if (!tablesList_.contains("tablesList"))
			createIndex();

	}
	
}

void HistoryDB::createIndex()
{
	QSqlQuery inquery("CREATE TABLE tablesList (name TEXT);", db);
	QSqlQuery tnquery("INSERT INTO tablesList VALUES ('tablesList');", db);
	tablesList_ = tablesList();
}

QStringList HistoryDB::tablesList()
{
	QStringList tl;
	QSqlQueryModel query;
	query.setQuery("SELECT name FROM tablesList", db);
	for(int i=0; i< query.rowCount(); i++)
	{
		tl << query.record(i).value("name").toString();
	}
	return tl;
}

void HistoryDB::createJidTable(QString j)
{
	QSqlQuery tquery1("CREATE TABLE DATE_" + j + " (id TEXT,date TEXT);", db);
	QSqlQuery tquery2("CREATE TABLE MSG_" + j + " (type TEXT, origin TEXT, date TEXT, time TEXT, text TEXT, html TEXT);", db);
	QSqlQuery tnquery("INSERT INTO tablesList VALUES ('" + j +"')", db);
	tablesList_ = tablesList();
}

bool HistoryDB::logEvent(QString j, PsiEvent *e)
{
	j = getTableName(j);
	if(!tablesList_.contains(j))
		createJidTable(j);

	if(e->type() == PsiEvent::Message && PsiOptions::instance()->getOption("option.history.messages").toBool()) 
	{
		MessageEvent *me = (MessageEvent *)e;
		const Message &m = me->message();
		QString html("");
		ensureDate(j, m.timeStamp().date());
		if (m.containsHTML())
			html = m.htmlString();
		QSqlQuery query("INSERT INTO MSG_" + j + " VALUES ('" + m.type() + "','" + (e->originLocal() ? "to" : "from") + "','" + m.timeStamp().date().toString() + "','" + m.timeStamp().time().toString() + "','" + m.body() + "','" /*+ html*/ + "')", db);
	}
	else if(e->type() == PsiEvent::File && PsiOptions::instance()->getOption("option.history.file-transfers").toBool())
	{
		FileEvent *fe = (FileEvent *)e;
		XMPP::FileTransfer *ft = fe->fileTransfer();
		ensureDate(j, fe->timeStamp().date());
		QSqlQuery query("INSERT INTO MSG_" + j + " VALUES ('file','" + (e->originLocal() ? "to" : "from") + "','" + fe->timeStamp().date().toString() + "','" + fe->timeStamp().time().toString() + "',' Transfering file : " + ft->fileName() + QString("\n Size: %1").arg(ft->fileSize()) + "','')", db);
	}
	else if(e->type() == PsiEvent::Auth && PsiOptions::instance()->getOption("option.history.messages").toBool()) 
	{
		AuthEvent *ae = (AuthEvent *)e;	
		ensureDate(j,ae->timeStamp().date());
		QSqlQuery query("INSERT INTO MSG_" + j + " VALUES ('" + ae->authType() + "','" + (e->originLocal() ? "to" : "from") + "','" + ae->timeStamp().date().toString() + "','" + ae->timeStamp().time().toString() + "','" + "" + "','" + "" + "')", db);
	}
	return true;
}

QString HistoryDB::getTableName(QString j)
{
	return QCA::Hash("md5").hashToString(j.toLatin1());
}

void HistoryDB::ensureDate(const QString &j, const QDate &date)
{
	QSqlQueryModel query;
	query.setQuery("SELECT date FROM DATE_"+j+" WHERE date='"+date.toString()+"';", db);
	if(query.rowCount()==0) {
		QSqlQuery squery("INSERT INTO DATE_"+j+" VALUES('"+date.toString(Qt::ISODate)+"','"+date.toString()+"');", db);
	}
}

void HistoryDB::getDates(HistoryDlg *dlg, QTreeWidget *dateTree, QString j, int from, int count)
{
	QSqlQueryModel query;
	QString tableName;
	tableName = getTableName(j);
	query.setQuery("SELECT date FROM DATE_"+tableName+" ORDER BY id DESC LIMIT "+QString("%1").arg(from)+","+QString("%1").arg(from+count)+"", db);
//	QColor red(255,0,0);
	for(int i=0; i< query.rowCount(); i++)
	{
		QDate date = QDate::fromString(query.record(i).value("date").toString());
		DateItem *item = new DateItem(date);
		connect(dateTree,SIGNAL(itemClicked(QTreeWidgetItem *, int)),dlg,SLOT(dateSelected(QTreeWidgetItem*,int)));
		dateTree->addTopLevelItem(item);
	}
}

QTreeWidgetItem *HistoryDB::getDatesMatching(HistoryDlg *dlg, QTreeWidget *dateTree, QString j, QString searchFor)
{
	QSqlQueryModel query;
	QString tableName;
	tableName = getTableName(j);
	query.setQuery("SELECT date,text FROM MSG_" + tableName + " WHERE text LIKE '%"+searchFor+"%' ORDER BY date LIMIT 50", db);
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

HistoryItem *HistoryDB::getEvents(QTreeWidget *eventsTree, QString j, QDate date, QString searchFor)
{
	QString tableName;
	tableName = getTableName(j);
	QSqlQueryModel query;
	query.setQuery("SELECT type,origin,time,text,html FROM MSG_" + tableName + " WHERE date='"+date.toString()+"' ORDER BY time", db);
	for(int i=0; i< query.rowCount(); i++)
	{
		QString icon;
		if(query.record(i).value("type").toString() == "headline")
			icon = "psi/headline";
		else if(query.record(i).value("type").toString() == "chat")
			icon = "psi/chat";
		else if(query.record(i).value("type").toString() == "error")
			icon = "psi/system";
		else if(query.record(i).value("type").toString() == "file")
			icon = "psi/file";
		else
			icon = "psi/message";
		QTime  time;
		HistoryItem *item = new HistoryItem();
		item->setIcon(0, QIcon(IconsetFactory::icon(icon).impix()));
		item->setText(1, query.record(i).value("time").toString());
		item->setText(2, query.record(i).value("origin").toString());
		item->setText(3, query.record(i).value("text").toString());
		for(int j=1; j<4; j++)
			item->setTextColor(j, ((item->text(2) == "to") ? PsiOptions::instance()->getOption("options.ui.look.colors.chat.my").value<QColor>() : PsiOptions::instance()->getOption("options.ui.look.colors.chat.contact").value<QColor>()));
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

void HistoryDB::deleteEvents(QString j,QDate date,QTime time)
{
	QSqlQuery query(QString("DELETE FROM MSG_") + getTableName(j) + " WHERE date='" + date.toString() + ((time.isValid()) ? (QString("' and time='") + time.toString()) : "") + "'", db);
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
		query.setQuery("SELECT type,origin,date,time,text,html FROM MSG_" + tableName + " WHERE date='"+date.toString()+"' ORDER BY time", db);
	}
	else
		query.setQuery("SELECT * FROM MSG_" + tableName + " ORDER BY date AND time", db);
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
