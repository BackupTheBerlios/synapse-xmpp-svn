#ifndef GETCOLLECTIONTASK_H
#define GETCOLLECTIONTASK_H

#define MessageArchivingNS "http://www.xmpp.org/extensions/xep-0136.html#ns"

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QDomElement>
#include "xmpp_task.h"
#include "xmpp_jid.h"
#include "xmpp_xmlcommon.h"

class GetCollectionTask : public XMPP::Task
{
	Q_OBJECT
public:
	GetCollectionTask(XMPP::Task *parent);
	~GetCollectionTask();

	void get(const XMPP::Jid &, const QString &, int page = 0, int max = 30);

	void onGo();
	bool take(const QDomElement &);

signals:
	void msg(int sec, bool direction, const QString &body);
	void busy();
	void done(int count);
};

#endif
