#include "getcollectionlisttask.h"

GetCollectionListTask::GetCollectionListTask(Task *parent)
: Task(parent)
{
}

GetCollectionListTask::~GetCollectionListTask()
{
}

void GetCollectionListTask::get(const XMPP::Jid &jid, const QDate date, int max)
{
	QDomElement iq = createIQ(doc(), "get", "", id());
	QDomElement list = doc()->createElement("list");
	list.setAttribute("xmlns", MessageArchivingNS);
	list.setAttribute("with", jid.bare());
	list.setAttribute("start", QDateTime(date).toString(Qt::ISODate) + ".000Z"); /*.toString(Qt::ISODate)*/
	QDomElement set = doc()->createElement("set");
	set.setAttribute("xmlns", "http://jabber.org/protocol/rsm");
	set.appendChild(textTag(doc(), "max", QString("%1").arg(max)));
	list.appendChild(set);
	iq.appendChild(list);
	send(iq);
	emit busy();
}

void GetCollectionListTask::onGo()
{
}

bool GetCollectionListTask::take(const QDomElement &x)
{
	if(!iqVerify(x, "", id()))
		return false;

	if(x.attribute("type") == "result") {
		bool found;
		QDomElement list = findSubTag(x, "list", &found);
		if(found) {
			if(list.attribute("xmlns") != MessageArchivingNS)
				return false;

			collections_.clear();
			for(QDomNode n = list.firstChild(); !n.isNull(); n = n.nextSibling()) {
				QDomElement i = n.toElement();
				if(i.isNull())
					continue;

				if(i.tagName() == "chat") {
					collections_.append(Collection(i));
				}
			}

			emit done();

			return true;
		}
	}

	return false;
}

QList<Collection> GetCollectionListTask::collections()
{
	return collections_;
}
