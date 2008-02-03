#include "getcollectiontask.h"

GetCollectionTask::GetCollectionTask(Task *parent)
: Task(parent)
{
}

GetCollectionTask::~GetCollectionTask()
{
}

void GetCollectionTask::get(const XMPP::Jid &jid, const QString &start, int page, int max)
{
	QDomElement iq = createIQ(doc(), "get", "", id());
	QDomElement ret = doc()->createElement("retrieve");
	ret.setAttribute("xmlns", MessageArchivingNS);
	ret.setAttribute("with", jid.bare());
	ret.setAttribute("start", start);
	QDomElement set = doc()->createElement("set");
	set.setAttribute("xmlns", "http://jabber.org/protocol/rsm");
	set.appendChild(textTag(doc(), "max", QString("%1").arg(max)));
	if(page != 0)
		set.appendChild(textTag(doc(), "after", QString("%1").arg((max*page)-1)));
	ret.appendChild(set);
	iq.appendChild(ret);
	send(iq);
	emit busy();
}

void GetCollectionTask::deleteCollection(const XMPP::Jid &jid, const QString &start)
{
	QDomElement iq = createIQ(doc(), "set", "", id());
	QDomElement ret = doc()->createElement("remove");
	ret.setAttribute("xmlns", MessageArchivingNS);
	ret.setAttribute("with", jid.bare());
	ret.setAttribute("start", start);
	iq.appendChild(ret);
	send(iq);
	emit busy();
}

void GetCollectionTask::onGo()
{
}

bool GetCollectionTask::take(const QDomElement &x)
{
	int count = 0;
	if(!iqVerify(x, "", id()))
		return false;

	if(x.attribute("type") == "result") {
		bool found;
		QDomElement list = findSubTag(x, "chat", &found);
		if(found) {
			if(list.attribute("xmlns") != MessageArchivingNS)
				return false;

			for(QDomNode n = list.firstChild(); !n.isNull(); n = n.nextSibling()) {
				QDomElement i = n.toElement();
				if(i.isNull())
					continue;

				QString body;
				if(i.tagName() == "from") {
					XMLHelper::readEntry(i, "body", &body);
					emit msg(i.attribute("secs").toUInt(), true, body);
				}
				else if(i.tagName() == "to") {
					XMLHelper::readEntry(i, "body", &body);
					emit msg(i.attribute("secs").toUInt(), false, body);
				}
				else if(i.tagName() == "set") {
					XMLHelper::readNumEntry(i, "count", &count); 
				}
			}

			emit done(count);
		}
		return true;
	} else if(x.attribute("type") == "error") {
		emit done(0);
		emit error();
	}

	return false;
}
