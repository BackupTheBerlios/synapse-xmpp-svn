#include "garchive.h"
#include "garchive_otr.h"
#include "xmpp.h"
#include "xmpp_xmlcommon.h"
#include "psicon.h"
#include "psiaccount.h"
#include "chatdlg.h"
#include "userlist.h"
#include <QtGui>

using namespace Otr;

class JT_GArchive : public Task
{
	Q_OBJECT
public:
	JT_GArchive(Task *t,const Jid& jid);
	
	void setJid(const Jid& jid);
	
	bool take(const QDomElement &e);
	bool isOtrOn(const Jid& jid);
	void changeOtr(const Jid& jid);
	void setSave(bool to);
	OtrList *list();
	
public slots:
	void onGo();
	void parseResult();
	
signals:
	void otrChanged(const XMPP::Jid& jid);
	void saveChanged(bool state);
private:
	Jid receiver_;
	OtrList *otrList;
	QDomElement result;
};

JT_GArchive::JT_GArchive(Task *t, const Jid& j): Task(t), receiver_(j)
{
	otrList = new OtrList();
}

void JT_GArchive::setJid(const Jid& j)
{
	receiver_ = j;
}

bool JT_GArchive::take(const QDomElement &e)
{
	if(e.tagName() == "iq")
	{
		bool found_otr;
		bool found_query;
		bool found_save;
		QDomElement fo = findSubTag(e, "otr", &found_otr);
		QDomElement fq = findSubTag(e, "query", &found_query);
		QDomElement fs = findSubTag(e, "save", &found_save);
		found_query = found_query && (fq.attribute("xmlns")=="google:nosave");
		if((!iqVerify(e, receiver_, id())) && !found_otr && !found_query && !found_save) // czy aby napewno?? sa takie bez id i bez sender!!
			return false;
	
		if(e.attribute("type") == "result")
		{
			result = e;
			QTimer::singleShot(5, this, SLOT(parseResult()));
			setSuccess();
			return true;
		}
		if(e.attribute("type") == "set")
		{
			bool found;
			QDomElement query = findSubTag(e, "query", &found);
			if(found)
			{
				if(query.attribute("xmlns") == "google:nosave")
				{
					QDomElement item = findSubTag(query, "item", &found);
					if(found)
					{
						bool enabled = (item.attribute("value") == "enabled") ? true : false;
						Jid j(item.attribute("jid"));
					        if(otrList->isOn(j) != enabled)
						{
							otrList->setOn(j,enabled);
							otrChanged(j);
						}
					}
						
				}
				return true;
			}

			query = findSubTag(e, "otr", &found);
			if(found)
			{
				return true;
			}
		}
	} else if(e.tagName() == "message" && (e.attribute("to") == receiver_.bare()))
	{
		if(e.attribute("type") == "")
		{
			bool found;
			QDomElement nos = findSubTag(e, "nos:x", &found);
			if(found && nos.attribute("xmlns:nos") == "google:nosave")
			{
				bool enabled = (nos.attribute("value") == "enabled") ? true : false;
				Jid j(e.attribute("from"));
			        if(otrList->isOn(j) != enabled)
				{
					otrList->setOn(j,enabled);
					otrChanged(j);
				}
			}
			return true;
		} else if(e.attribute("type") == "error")
		{
			bool found;
			QDomElement nos = findSubTag(e, "nos:x", &found);
			if(found && nos.attribute("xmlns:nos") == "google:nosave")
			{
				QDomElement error = findSubTag(e, "error", &found);
			}
			return true;
		}
	}
	return false;
}

void JT_GArchive::parseResult()
{
	bool found;
	QDomElement otr = findSubTag(result, "arc:otr", &found);
	if(found)
	{
		for(QDomNode n = otr.firstChild(); !n.isNull(); n = n.nextSibling())
		{
			QDomElement a = n.toElement();
			if(a.isNull())
				continue;
			if(a.tagName() == "arc::record")
				otrList->setOn(a.attribute("jid"), (a.attribute("otr")=="false") ? false : true);
		}
	}
	QDomElement save = findSubTag(result, "save", &found);
	if(found && (save.attribute("xmlns") == "http://jabber.org/protocol/archive"))
	{
		QDomElement def = findSubTag(save, "default", &found);
		if(found)
		{
			saveChanged((def.attribute("save") == "true"));
		}
	}
}

bool JT_GArchive::isOtrOn(const Jid& jid)
{
	return otrList->isOn(jid);
}

void JT_GArchive::changeOtr(const Jid& jid)
{
	QDomElement e = createIQ(doc(), "set", receiver_.full(), id());
	QDomElement q = doc()->createElement("otr");
	q.setAttribute("xmlns","http://jabber.org/protocol/archive");
	QDomElement r = doc()->createElement("record");
	r.setAttribute("otr",(otrList->isOn(jid) ? "false" : "true"));
	r.setAttribute("jid",jid.bare());
	q.appendChild(r);
	e.appendChild(q);
	send(e);
}

void JT_GArchive::setSave(bool to)
{
	QDomElement e = createIQ(doc(), "set", receiver_.full(), id());
	QDomElement q = doc()->createElement("save");
	q.setAttribute("xmlns","http://jabber.org/protocol/archive");
	QDomElement r = doc()->createElement("default");
	r.setAttribute("save",(to ? "true" : "false"));
	q.appendChild(r);
	e.appendChild(q);
	send(e);
	onGo();
}

OtrList *JT_GArchive::list()
{
	return otrList;
}

void JT_GArchive::onGo()
{
	QDomElement e = createIQ(doc(), "get", receiver_.full(), id());
	QDomElement q = doc()->createElement("otr");
	q.setAttribute("xmlns","http://jabber.org/protocol/archive");
	e.appendChild(q);
	send(e);
	
	QDomElement r = createIQ(doc(), "get", receiver_.full(), id());
	QDomElement s = doc()->createElement("save");
	s.setAttribute("xmlns","http://jabber.org/protocol/archive");
	r.appendChild(s);
	send(r);
}

void GArchive::setup(PsiAccount *_pa,const Jid& j, bool enabled)
{
	pa_ = _pa;
	receiver_ = j;
	task_ = NULL;
	save_ = false;
	if(enabled)
		setEnabled(true);
}

void GArchive::init()
{
}

void GArchive::reset()
{
	save_ = false;
	if(task_ != NULL)
		task_->go(false);
}

void GArchive::setJid(const XMPP::Jid& j)
{
}

void GArchive::setEnabled(bool en)
{
	if(task_ == NULL && en)
	{
		task_ = new JT_GArchive(pa_->client()->rootTask(),receiver_);
		connect(task_,SIGNAL(otrChanged(const XMPP::Jid&)),SLOT(updated(const XMPP::Jid&)));
		connect(task_,SIGNAL(saveChanged(bool)),SLOT(set(bool)));
		reset();
	}
}

bool GArchive::isEnabled()
{
	return (save_ && ((task_ != NULL) ? true : false));
}

GArchive::~GArchive()
{
	if(task_ != NULL)
		delete task_;
}

bool GArchive::isEvent(const Jid& jid)
{
	if(task_ != NULL)
		return task_->isOtrOn(jid);
	else
		return true;
}

int GArchive::countEvents()
{
	return 0;
}


void GArchive::updated(const XMPP::Jid& jid)
{
	ChatDlg *t = pa_->psi()->getChatInTabs(jid.bare()+"/"+(*pa_->find(jid)->priority()).name());
	if(t != NULL)
		t->updateOtr();
}

void GArchive::changed()
{
	if(task_ != NULL)
		task_->setSave(!(isEnabled()));
}

void GArchive::change(const XMPP::Jid& jid)
{
	if(task_ != NULL)
		task_->changeOtr(jid);
}

void GArchive::set(bool state)
{
	if(save_ != state)
	{
		if(task_ != NULL)
		{
		    OtrList *l = task_->list();
		    for(OtrList::Iterator it=l->begin(); it!=l->end(); it++)
		    {
			    ChatDlg *t = pa_->psi()->getChatInTabs((*it).jid().bare()+"/"+(*pa_->find((*it).jid())->priority()).name());
			    if(t != NULL)
				    t->updateSave(state);
		    }
		}
	}
	save_ = state;
}

QString GArchive::name()
{
	return QString("Google Message Archives");
}

QString GArchive::version()
{
	return QString("20070627");
}

ContactViewItem *GArchive::cvi()
{
	return NULL;
}

void GArchive::setCvi(ContactViewItem *i)
{
}

void GArchive::setPsiCon(PsiCon *psi)
{
}

void GArchive::process(const XMPP::Jid& j, const QString& s)
{
}

Q_EXPORT_PLUGIN2(google_archive, GArchive);

#include "garchive.moc"

