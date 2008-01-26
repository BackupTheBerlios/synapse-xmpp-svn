#ifndef GETCOLLECTIONLISTTASK_H
#define GETCOLLECTIONLISTTASK_H

#define MessageArchivingNS "http://www.xmpp.org/extensions/xep-0136.html#ns"

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QDomElement>
#include "xmpp_task.h"
#include "xmpp_jid.h"
#include "xmpp_xmlcommon.h"

class Collection {
public:
	Collection(const QDomElement &e){
		e_ = e;
		date_ = QDateTime::fromString(e_.attribute("start").left(15), Qt::ISODate);
		date_.setTimeSpec(Qt::UTC);
	};
	~Collection(){};

	QDateTime date() {
		return date_.toLocalTime();
	}

	QString start() {
		return e_.attribute("start");
	}

	const QString &subject() {
		return e_.attribute("subject");
	}

	bool isEncryted() {
		return e_.attribute("crypt") == "true";
	}

private:
	QDomElement e_;
	QDateTime date_;
};

class GetCollectionListTask : public XMPP::Task
{
	Q_OBJECT
public:
	GetCollectionListTask(XMPP::Task *parent);
	~GetCollectionListTask();

	void get(const XMPP::Jid &, const QDate, int max = 30);

	void onGo();
	bool take(const QDomElement &);

	QList<Collection> collections();

signals:
	void busy();
	void done();

private:
	QList<Collection> collections_;
};

#endif

