#include <QtCore/QByteRef>
#include <QHostInfo>
#include <QMap>
#include <QList>
#include <QUrl>

#include "linklocal.h"
#include "xmpp_jid.h"
#include "xmpp_status.h"

using namespace XMPP;

LinkLocal::LinkLocal() : port_("5298")
{
	nick_ = "";
	txt_id = 0;
	stream_ = new LinkLocal::Stream();
	connect(&c2c, SIGNAL(connectionReady(int)), this, SLOT(c2c_connectionReady(int)));
}

LinkLocal::~LinkLocal()
{
}

bool LinkLocal::reset()
{
	jdns.shutdown();
	txt_id = 0;
	return true;
}

void LinkLocal::jdns_resultsReady(int x, const QJDns::Response &resp)
{
	QList<QJDns::Record>::const_iterator it;
	for(it = resp.answerRecords.begin(); it != resp.answerRecords.end(); ++it) {
		const QJDns::Record *r = &(*it);
		QString rname(r->name);
		Jid jid = Jid(rname.left( rname.indexOf(".") ));
		if (jid.isEmpty())
			jid = Jid( QString(r->owner).left( r->owner.indexOf(".") ) );
		if (r->type == QJDns::Ptr) {
#ifdef LL_DEBUG
			printf(" --> Ptr : %s\n", jid.bare().toAscii().data());
#endif
			ContactData *cd = new ContactData();
			cd->jid = Jid(jid);
			contacts_[jid.bare()] = cd;
			jdns.queryStart(QString("_presence._tcp.local.")/*.arg(jid.bare())*/.toLatin1(),QJDns::Srv);
			jdns.queryStart(QString("%1.local.").arg(jid.domain()).toLatin1(),QJDns::A);
			jdns.queryStart(QString("%1._presence._tcp.local.").arg(jid.bare()).toLatin1(),QJDns::Txt);
		} else if (r->type == QJDns::Txt) {
#ifdef LL_DEBUG
			printf(" --> Txt : %s\n", jid.bare().toAscii().data());
#endif
			ContactData *cd = contacts_[jid.bare()];
			if (cd) {
				PresenceData data;
				Q_FOREACH(QByteArray x, r->texts) {
					int pos=x.indexOf("=");
					if (pos==-1)
						data[x] = QByteArray();
					else
						data[x.mid(0,pos)] = x.mid(pos+1,x.size()-pos);
				}
				cd->presence = data;
				if(jid.bare().compare(jid_.bare()) != 0) {
				RosterItem ri(jid);
				ri.setName(data["nick"]);
				ri.setSubscription(Subscription::Both);
#ifdef LL_DEBUG
				printf("Resolved: %s %d\n", jid.bare().toAscii().data(), 0);
#endif
				emit rosterItemUpdated(ri);
				}
				Status s(data["status"],data["msg"],0,true);
				if(!data["ext"].isEmpty())
					s.setCapsExt(data["ext"]);
				if(!data["node"].isEmpty())
					s.setCapsNode(data["node"]);
				if(!data["ver"].isEmpty())
					s.setCapsVersion(data["ver"]);
				if(!data["phsh"].isEmpty())
					s.setPhotoHash(data["phsh"]);
				
				emit presence(Jid(jid.bare()+"/Local"), s);
			}
		} else if (r->type == QJDns::Srv) {
#ifdef LL_DEBUG
			printf(" --> Srv : %s\n", jid.bare().toAscii().data());
#endif
			ContactData *cd = contacts_[jid.bare()];
			if (cd) {
				cd->port = r->port;
			}
		} else if (r->type == QJDns::A) {
#ifdef LL_DEBUG
			printf(" --> A : %s\n", jid.bare().toAscii().data());
#endif
			QMap<QString,ContactData*>::iterator it;
			for(it = contacts_.begin(); it != contacts_.end(); ++it)
				if ( (*it)->jid.domain().compare(jid.bare()) == 0 )
					(*it)->addr = r->address;
		}
	}
}

void LinkLocal::jdns_published(int id)
{
#ifdef LL_DEBUG
	printf("[%d] Published\n", id);
#endif
}

void LinkLocal::jdns_error(int id, QJDns::Error e)
{
	QString str;
	if(e == QJDns::ErrorGeneric)
		str = "Generic";
	else if(e == QJDns::ErrorNXDomain)
		str = "NXDomain";
	else if(e == QJDns::ErrorTimeout)
		str = "Timeout";
	else if(e == QJDns::ErrorConflict)
		str = "Conflict";
#ifdef LL_DEBUG
	printf("[%d] Error: %s\n", id, qPrintable(str));
#endif
	jdns.shutdown();
}

QString LinkLocal::getHostname()
{
	return QHostInfo::localHostName();
}

bool LinkLocal::isConnected()
{
	return (txt_id!=0);
}

LinkLocal::Stream *LinkLocal::stream()
{
	return stream_;
}

LinkLocal::Stream *LinkLocal::ensureStream(Jid j)
{
	if(contacts_[j.bare()] != NULL)
	{
		QHostAddress addr = contacts_[j.bare()]->addr;
		quint16 port = contacts_[j.bare()]->port;
		LinkLocal::Stream *stream = 0;
		QList<LinkLocal::Stream*>::iterator it;
		for( it = streams_.begin(); it != streams_.end(); ++it)
		{
			if((*it)->compare(addr,port)) {
				stream = *it;
				return stream;
			}
		}

		BSocket *bs = new BSocket;
		if(stream == 0) {
			stream = new LinkLocal::Stream(jid_, addr.toString(), port);
			connect(stream, SIGNAL(readyRead(Stanza)), this, SIGNAL(readyRead(Stanza)));
			connect(stream, SIGNAL(cleanUp(LinkLocal::Stream*)), this, SLOT(removeStream(LinkLocal::Stream*)));
			streams_.append(stream);
			return stream;
		}
	}
	return NULL;
}

void LinkLocal::setPresence(const Status &s)
{
	QString localName = getHostname();
	if(s.isAvailable()) {
		QString status = ((s.show().isEmpty()) ? "avail" : s.show());
		status  = QString("status=") + ((status.compare("xa")) ? status : "away");

		QHostInfo ip = QHostInfo::fromName(localName);

//TXT record
		QList<QByteArray> text;
		text.append(status.toAscii());
		text.append("txtvers=1");
		text.append((QString("port.p2pj=")+port_).toAscii());
		if(!nick_.isEmpty())
			text.append("nick="+nick_.toAscii());
		text.append("msg="+s.status().toAscii());
		text.append("ext="+s.capsExt().toAscii());
		text.append("node="+s.capsNode().toAscii());
		text.append("ver="+s.capsVersion().toAscii());
		if(s.hasPhotoHash())
			text.append("phsh="+s.photoHash().toAscii());

		QJDns::Record txt;
		txt.owner = QString("%1@%2._presence._tcp.local.").arg(jid_.node()).arg(localName).toLatin1();
		txt.type = QJDns::Txt;
		txt.ttl = 4500;
		txt.haveKnown = true;
		txt.name = QString("%1@%2._presence._tcp.local.").arg(jid_.node()).arg(localName).toLatin1();
		txt.texts = text;

		if(txt_id==0) {
			if(!jdns.init(QJDns::Multicast, QHostAddress::Any)) {
				qDebug("Can't start multicast!\n");
				return;
			}
			c2c.listen(QVariant(port_).toInt());

			connect(&jdns, SIGNAL(resultsReady(int, const QJDns::Response &)), SLOT(jdns_resultsReady(int, const QJDns::Response &)));
			connect(&jdns, SIGNAL(published(int)), SLOT(jdns_published(int)));
			connect(&jdns, SIGNAL(error(int, QJDns::Error)), SLOT(jdns_error(int, QJDns::Error)));
			jdns.queryStart(QString("_presence._tcp.local.").toLatin1(),QJDns::Ptr);
//A record
			QJDns::Record a;
			a.owner = QString("%1.local.").arg(localName).toLatin1();
			a.type = QJDns::A;
			a.ttl = 4500;
			a.haveKnown = true;
			a.address = "10.0.0.2";//ip.addresses().first();
			jdns.publishStart(QJDns::Unique, a);

//SRV record
			QJDns::Record srv;
			srv.owner = QString("_presence._tcp.local.").toLatin1();
			srv.type = QJDns::Srv;
			srv.ttl = 4500;
			srv.haveKnown = true;
			srv.name = QString("%1@%2.local.").arg(jid_.node()).arg(localName).toLatin1();
			srv.priority = s.priority();
			srv.weight = 0;
			srv.port = QVariant(port_).toInt();
			jdns.publishStart(QJDns::Unique, srv);

//TXT record
			txt_id = jdns.publishStart(QJDns::Unique, txt);
//PTR record
			QJDns::Record ptr;
			ptr.owner = QString("_presence._tcp.local.").toLatin1();
			ptr.type = QJDns::Ptr;
			ptr.ttl = 4500;
			ptr.haveKnown = true;
			ptr.name = QString("%1@%2._presence._tcp.local.").arg(jid_.node()).arg(localName).toLatin1();
			jdns.publishStart(QJDns::Shared, ptr);

		} else
			jdns.publishUpdate(txt_id,txt);


	} else if(txt_id!=0) {
		Status st(s);
		st.setStatus("");
		emit presence(Jid(jid_.node()+"@"+localName+"/Local"), st);
		
		c2c.stop();
		reset();
	}
}

void LinkLocal::removeStream(LinkLocal::Stream *s)
{
	int i = 0;
	QList<LinkLocal::Stream*>::iterator it;
	for( it = streams_.begin(); it != streams_.end(); ++it)
	{
		if((*it) == s) {
			streams_.removeAt(i);
			delete s;
		}
		i++;
	}
}

void LinkLocal::c2c_connectionReady(int s)
{
	BSocket *bs = new BSocket;
	bs->setSocket(s);
	LinkLocal::Stream *stream = new LinkLocal::Stream(jid_, bs);
 	connect(stream, SIGNAL(readyRead(Stanza)), SIGNAL(readyRead(Stanza)));
	streams_.append(stream);
}

#include  "linklocal.moc"
