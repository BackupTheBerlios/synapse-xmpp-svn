#include "gmail_notify.h"
#include "psiaccount.h"
#include "iconset.h"
#include "xmpp.h"
#include "xmpp_xmlcommon.h"
#include <QObject>
#include <QTimer>
#include <QtGui>
//#include "contactview.h"
#include "psicon.h"
#include "mainwin.h"
#include "xmpp_message.h"
#include "xmpp_client.h"

class testb : public QObject
{
	Q_OBJECT
};

class JT_GMailNotify : public Task
{
	Q_OBJECT
public:
	JT_GMailNotify(Task *t,const Jid& jid);
	void setJid(const Jid& jid);
	
	bool take(const QDomElement &x);
	
	QString parse(const QDomElement &p);
	bool isNewMail();
	int newMailCount();
	Message *newMailInfo();

public slots:	
	void onGo();
	void parseRespond();

private:
	Jid receiver_;
	Message message_;
	int count_;
	QTimer *timer_;
	
	QDomElement respond;

	bool newMail_;
	bool error_;
	bool inProgress_;
};

JT_GMailNotify::JT_GMailNotify(Task *t, const Jid& j): Task(t), receiver_(j)
{
	timer_ = new QTimer(this, "gmail_notify");
	connect(timer_, SIGNAL(timeout()), this, SLOT(onGo()));
	newMail_ = false;
	error_ = false;
}

void JT_GMailNotify::setJid(const Jid& jid)
{
	receiver_ = jid;
}

void JT_GMailNotify::onGo()
{
	inProgress_ =  true;
	QDomElement e = createIQ(doc(), "get",receiver_.full(), id());
	QDomElement q = doc()->createElement("query");
	q.setAttribute("xmlns","google:mail:notify");
	e.appendChild(q);
	send(e);
}

bool JT_GMailNotify::take(const QDomElement& e)
{
	bool found;
	QDomElement xxx = findSubTag(e, "new-mail", &found);
	if(!(iqVerify(e, receiver_, id()) || found))
		return false;
	
	if((e.attribute("type") == "result") || (e.attribute("type") == "set"))
	{
		respond = e;
		QTimer::singleShot(5, this, SLOT(parseRespond()));
		setSuccess();
		return true;
	} else {
		setError(e);
		if(e.attribute("type") == "error")
		{
			error_ = true;
			timer_->start(10000);
		}
		return false;
	}
}

void JT_GMailNotify::parseRespond()
{
	bool found;
	error_ = false;
	QDomElement mailbox = findSubTag(respond, "mailbox", &found);
	if(found && inProgress_)
	{
		QString info;
		info = "";
		count_ = 0;
		for(QDomNode n = mailbox.firstChild(); !n.isNull(); n = n.nextSibling())
		{
			info = info + parse(n.toElement()) + "--------------\n";
			count_++;
		}
		if(!info.isEmpty())
		{
//			Message m;
			XMPP::Jid tmp_jid(receiver_.domain());
			message_.setFrom(tmp_jid);
			message_.setSubject(tr("New mail"));
			message_.setBody(info);
//			message_ = m;
			newMail_ = true;
		} else {
			count_ = 0;
			newMail_ = false;
		}
		inProgress_ = false;
		emit finished();
	} else {
		QDomElement new_mail = findSubTag(respond, "new-mail", &found);
		if(found && !inProgress_)
			onGo();
	}
}

QString JT_GMailNotify::parse(const QDomElement& p)
{
	if(!p.isNull() && p.tagName() == "mail-thread-info")
	{
		QString senders_s = QString(tr("Senders:"));
		bool found;
		QDomElement senders = findSubTag(p, "senders", &found);
		if(found)
		{
			for(QDomNode n = senders.firstChild(); !n.isNull(); n = n.nextSibling())
			{
				QDomElement a = n.toElement();
				if(a.isNull())
					continue;
				if(a.tagName() == "sender")
					senders_s = senders_s + " " + a.attribute("name") + " <" + a.attribute("address") + ">";
			}
		}
		QString subject_s;
		QString snippet_s;
		XMLHelper::readEntry(p, "subject", &subject_s);
		XMLHelper::readEntry(p, "snippet", &snippet_s);
		return senders_s + tr("\nSubject: ") + subject_s + tr("\nSnippet: ") + snippet_s + "\n";
	}
	return QString();
}

bool JT_GMailNotify::isNewMail()
{
	return (newMail_ && !error_);
}

int JT_GMailNotify::newMailCount()
{
	return count_;
}

Message *JT_GMailNotify::newMailInfo()
{
	return &message_;
}

void GMailNotify::setup(PsiAccount *pa, const Jid& receiver, bool enable)
{
	pa_ = pa;
	task_ = new JT_GMailNotify(pa_->client()->rootTask(), receiver);
	connect(task_,SIGNAL(finished()),SLOT(done()));
	enabled_ = enable;
	cvi_ = NULL;
}

GMailNotify::~GMailNotify()
{
	delete task_;
}

void GMailNotify::init()
{
	if(enabled_)
		task_->go(false);
}

void GMailNotify::reset()
{
}

void GMailNotify::changed()
{
}

void GMailNotify::setJid(const XMPP::Jid& j)
{
	if(enabled_)
		task_->setJid(j);
}

void GMailNotify::done()
{
	if(task_->isNewMail() && pa_->notifyNewMail())
	{
	// jest nowa poczta
	pa_->processIncomingMessage(*task_->newMailInfo());
	}
//	cvi_->repaint();
}

bool GMailNotify::isEvent(const XMPP::Jid &reciver)
{
	if(enabled_)
		return (task_->isNewMail());
	return FALSE;
}

int GMailNotify::countEvents()
{
	if(enabled_)
		return (task_->newMailCount());
	return 0;
}

bool GMailNotify::isEnabled()
{
	return enabled_;
}

void GMailNotify::setEnabled(bool enable)
{
	enabled_ = enable;
	if(enabled_)
		init();
}

ContactViewItem *GMailNotify::cvi()
{
	return cvi_;
}

void GMailNotify::setCvi(ContactViewItem *i)
{
	cvi_ = i;
}

void GMailNotify::setPsiCon(PsiCon *psi)
{
}

void GMailNotify::process(const XMPP::Jid& j, const QString& s)
{
}

void GMailNotify::updated(const XMPP::Jid& jid)
{
}

void GMailNotify::change(const XMPP::Jid& jid)
{
}

void GMailNotify::set(bool state)
{
}

QString GMailNotify::name()
{
	return QString("Google Mail Notification");
}

QString GMailNotify::version()
{
	return QString("20070627");
}


Q_EXPORT_PLUGIN2(google_mailnotification, GMailNotify);
