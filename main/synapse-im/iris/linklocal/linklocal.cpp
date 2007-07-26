#include <QtCore/QByteRef>
//#include <QDBus>
//#include <QDBusAbstractInterface>
//#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QMap>
#include <QList>
#include <QUrl>

#include "linklocal.h"
#include "xmpp_jid.h"
#include "xmpp_status.h"

#ifndef QLIST_METATYPE
#define QLIST_METATYPE
Q_DECLARE_METATYPE(QList<QByteArray>)
#endif

using namespace XMPP;

class LinkLocal::Resolve : public QObject {
	Q_OBJECT
public:
	Resolve(org::freedesktop::Avahi::Server *srv) : srv_(srv), con(QDBusConnection::systemBus()) {
		qDBusRegisterMetaType<QList<QByteArray> >();
	}
	
	void start(int interface, int protocol, QString name, QString type, QString domain)
	{
		QDBusReply<QDBusObjectPath> rep = srv_->ServiceResolverNew(interface,protocol,name,type,domain,-1,(unsigned int)8);
		if(!rep.isValid()) {
//			printf("name: %s\nmsg: %s\n",rep.error().name().ascii(), rep.error().message().ascii());
			return;
		}
		sr_ = new org::freedesktop::Avahi::ServiceResolver(QString("org.freedesktop.Avahi"), rep.value().path(),  con, this);
		connect(sr_, SIGNAL(Found(int,int,const QString &,const QString &,const QString &,const QString &, int, const QString &,ushort , const QList<QByteArray>&, uint)), this, SLOT(found(int,int,const QString &,const QString &,const QString &,const QString &, int, const QString &,ushort , const QList<QByteArray>&, uint)));
		connect(sr_, SIGNAL(Failure(const QString &)), this, SLOT(error(const QString &)));
	}
public slots:
	void found(int,int,const QString &name,const QString &type,const QString &domain,const QString &host, int aprotocol, const QString &address,ushort port, const QList<QByteArray>& txt, uint) {
		printf("addrsss: %s\n", address.ascii());
		emit done(name, domain, host, port, txt);
	}

	void error(const QString &) {
		QList<QByteArray>t;
		ushort u = 0;
		emit done( QString(), QString(), QString(), u, t);
	}

signals:
	void done(const QString &,const QString &,const QString &, ushort &, const QList<QByteArray>& );

private:
	org::freedesktop::Avahi::Server *srv_;
	org::freedesktop::Avahi::ServiceResolver *sr_;
	QDBusConnection con;
};

LinkLocal::LinkLocal() : service_("org.freedesktop.Avahi"), root_("/"), port_("5298"), con(QDBusConnection::systemBus()) 
{
//	printf("linkLocal\n");
	qDBusRegisterMetaType<QList<QByteArray> >();
	server_ = NULL;
	entryGroup_ = NULL;
	discovery_ = NULL;
	nick_ = "";
	registred_ = false;
	stream_ = new LinkLocal::Stream();
	connect(&c2c, SIGNAL(connectionReady(int)), this, SLOT(c2c_connectionReady(int)));
}

LinkLocal::~LinkLocal()
{
	delete server_;
	delete entryGroup_;
	delete discovery_;
}

bool LinkLocal::reset()
{
	delete server_;
	delete entryGroup_;
	delete discovery_;
	registred_ = false;
	server_ = new org::freedesktop::Avahi::Server(service_, root_, con, this);
	QDBusReply<QDBusObjectPath> rep = server_->EntryGroupNew();
	if(!rep.isValid()) {
//		printf("name: %s\nmsg: %s\n",rep.error().name().ascii(), rep.error().message().ascii());
		return false;
	}
	entryGroup_ = new org::freedesktop::Avahi::EntryGroup(service_, rep.value().path(), con, this);
	return true;
}

bool LinkLocal::registerSrv(const QList<QByteArray> &txt) 
{
	entryGroup_->AddService(-1, -1, (unsigned int)0,jid_.bare(), QString("_presence._tcp"), QString(""), QString(""), (unsigned short int)(QVariant(port_).toInt()), (const QList<QByteArray> &)txt);
//	printf("registred\n");
	entryGroup_->Commit();
//	printf("commited\n");
	return true;
}

bool LinkLocal::updateSrv(const QList<QByteArray> &txt) 
{
	entryGroup_->UpdateServiceTxt(-1, -1, (unsigned int)0, jid_.bare(), QString("_presence._tcp"), QString(""), (const QList<QByteArray> &)txt);
	entryGroup_->Commit();
	return true;
}

bool LinkLocal::unregisterSrv()
{
	QList<QVariant> argumentList;
	entryGroup_->Reset();
	entryGroup_->Free();
	return true;
}

void LinkLocal::startDiscovery()
{
	QDBusReply<QDBusObjectPath> rep = server_->ServiceBrowserNew(-1, -1, QString("_presence._tcp"), QString(""), (unsigned int)0);
	if(!rep.isValid()) {
//		printf("name: %s\nmsg: %s\n",rep.error().name().ascii(), rep.error().message().ascii());
		return;
	}
//	printf("new ServiceBrowser()\n");
	discovery_ = new org::freedesktop::Avahi::ServiceBrowser(service_, rep.value().path(), con, this);
	connect(discovery_, SIGNAL(ItemNew(int, int, const QString&, const QString&, const QString&, unsigned int)), this, SLOT(itemNew(int, int, const QString&, const QString&, const QString&, unsigned int)));
	connect(discovery_, SIGNAL(ItemRemove(int, int, const QString&, const QString&, const QString&, unsigned int)), this, SLOT(itemRemove(int, int, const QString&, const QString&, const QString&, unsigned int)));
	connect(discovery_, SIGNAL(Failure(const QString &)), this, SLOT(error(const QString &)));
	connect(discovery_,SIGNAL(AllForNow()),this,SLOT(allForNow()));
}

void LinkLocal::stopDiscovery()
{
	discovery_->Free();
}


QString LinkLocal::getHostname()
{
	return server_->GetHostName();
}

bool LinkLocal::isConnected()
{
	return registred_;
}

LinkLocal::Stream *LinkLocal::stream()
{
	return stream_;
}

LinkLocal::Stream *LinkLocal::ensureStream(Jid j)
{
	if(contacts_[j.bare()] != NULL)
	{
//		printf("Contact found!\n");
		QHostAddress addr = contacts_[j.bare()]->addr;
		quint16 port = contacts_[j.bare()]->port;
//		printf("address: %s\n",addr.toString().ascii());
//		printf("port: %d\n", port);
//		printf("searching for opened session..\n");
		LinkLocal::Stream *stream = 0;
		for(LinkLocal::Stream *stmp = streams_.first(); stmp; stmp = streams_.next())
		{
			if(stmp->compare(addr,port)) {
				stream = stmp;
				return stream;
			}
		}

		BSocket *bs = new BSocket;
		if(stream == 0) {
//			printf("opening session..\n");
// tmp		BSocket *bs = new BSocket;
//		bs->connectToHost( to.domain() needs to be resolved by Avahi ,port_ will be recived);
//		bs->connectToHost( "10.0.0.2" needs to be resolved by Avahi ,QVariant(d->port_).toInt()will be recived);
			stream = new LinkLocal::Stream(jid_, addr.toString(), port);
//			stream = new LinkLocal::Stream(jid_, "10.0.0.2", QVariant(port_).toInt());
			connect(stream, SIGNAL(readyRead(Stanza)), this, SIGNAL(readyRead(Stanza)));
			connect(stream, SIGNAL(cleanUp(LinkLocal::Stream*)), this, SLOT(removeStream(LinkLocal::Stream*)));
//			printf("bs: %d\n", bs->state());
			streams_.append(stream);
			return stream;
		}
	}
	return NULL;
}

void LinkLocal::setPresence(const Status &s)
{
	if(s.isAvailable()) {
		QString status = ((s.show().isEmpty()) ? "avail" : s.show());
		status  = QString("status=") + ((status.compare("xa")) ? status : "away");

//		printf("%s\n", status.ascii());

		QList<QByteArray> txt;
		txt.append(status.toAscii());
		txt.append("txtvers=1");
		txt.append((QString("port.p2pj=")+port_).toAscii());
		if(!nick_.isEmpty())
			txt.append("nick="+nick_.toAscii());
		txt.append("msg="+s.status().toAscii());
		txt.append("ext="+s.capsExt().toAscii());
		txt.append("node="+s.capsNode().toAscii());
		txt.append("ver="+s.capsVersion().toAscii());
		if(s.hasPhotoHash())
			txt.append("phsh="+s.photoHash().toAscii());
		if(!registred_) {
			c2c.listen(QVariant(port_).toInt());
			registred_ = registerSrv(txt);
			startDiscovery();
		} else
			updateSrv(txt);
	} else if(registred_) {
		c2c.stop();
		registred_ = !unregisterSrv();
		stopDiscovery();
		reset();
	}
}

/// Slots
void LinkLocal::itemNew(int interface, int protocol, const QString &name, const QString &type, const QString &domain, unsigned int flags)
	{
//	printf("itemNew: type=%s\n", type.ascii());
	if((type.compare("_presence._tcp") != 0) || (name.compare(jid_.bare()) == 0))
		return;
	Resolve *r = new Resolve(server_);
	connect(r, SIGNAL(done(const QString &,const QString &,const QString &, ushort &, const QList<QByteArray>&)), this, SLOT(update(const QString &,const QString &,const QString &, ushort &, const QList<QByteArray>&)));
	r->start(interface, protocol, name, type, domain);
}

void LinkLocal::itemRemove(int interface, int protocol, const QString &name, const QString &type, const QString &domain, unsigned int flags)
{
//	printf("itemRemove: type=%s\n", type.ascii());
	//free presenceData;!!
	if((type.compare("_presence._tcp") != 0) || (name.compare(jid_.bare()) == 0))
		return;
	Jid jid(name);
	Status s("Local","",0,false);
//	emit resourceUnavailable(jid, s);
	emit presence(jid, s);
}

void LinkLocal::update(const QString &name,const QString &domain,const QString &host, ushort &port, const QList<QByteArray>& txt)
{
//	printf("updating\n");
	PresenceData data;
	Q_FOREACH(QByteArray x, txt) {
		int pos=x.indexOf("=");
		if (pos==-1)
			data[x] = QByteArray();
		else
			data[x.mid(0,pos)] = x.mid(pos+1,x.size()-pos);
	}

	Jid jid(name);
	ContactData *cd = new ContactData;
	cd->jid.set(name);
	cd->port = port;
	cd->presence = data;
	contacts_[name] = cd;

	QDBusReply<QDBusObjectPath> rep = server_->HostNameResolverNew(-1,-1,host,-1,(uint)0);
	if(!rep.isValid()) {
//		printf("name: %s\nmsg: %s\n",rep.error().name().ascii(), rep.error().message().ascii());
		return;
	}
	org::freedesktop::Avahi::HostNameResolver *hr = new org::freedesktop::Avahi::HostNameResolver(QString("org.freedesktop.Avahi"), rep.value().path(),  con, this);
	connect(hr, SIGNAL(Found(int, int,const QString&, int, const QString&, uint)), cd, SLOT(setAddress(int, int,const QString&, int, const QString&, uint)));

	RosterItem ri(jid);
	ri.setName(data["nick"]);
	ri.setSubscription(Subscription::Both);
	printf("Resolved: %s %d\n", name.ascii(), port);
	emit rosterItemUpdated(ri);
	Status s(data["status"],data["msg"],0,true);
	if(!data["ext"].isEmpty())
		s.setCapsExt(data["ext"]);
	if(!data["node"].isEmpty())
		s.setCapsNode(data["node"]);
	if(!data["ver"].isEmpty())
		s.setCapsVersion(data["ver"]);
	if(!data["phsh"].isEmpty())
		s.setPhotoHash(data["phsh"]);
//	Resource r("Local",s);
//	emit resourceAvailable(jid, r);
//	printf("emit presence()\n");
	emit presence(Jid(jid.bare()+"/Local"), s);
}

void LinkLocal::removeStream(LinkLocal::Stream *s)
{
	streams_.remove((const LinkLocal::Stream*)s);
}

void LinkLocal::error(const QString &msg)
{
//	printf("error Discovery: %s\n", msg.ascii());
}

void LinkLocal::allForNow()
{
//	printf("allForNow\n");
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
