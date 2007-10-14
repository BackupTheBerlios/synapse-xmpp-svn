#include <QObject>
#include <QtCore/QByteRef>
#include <QString>
#include <QMap>
#include <QList>
#include "xmpp_client.h"
#include "xmpp_rosteritem.h"
#include "xmpp_status.h"
#include "xmpp_resource.h"
#include "xmpp_stream.h"
#include "xmpp_message.h"

#include "servsock.h"
#include "bsocket.h"
#include "protocol.h"

#include "avahi_addressresolver.h"
#include "avahi_entrygroup.h"
#include "avahi_server.h"
#include "avahi_servicebrowser.h"
#include "avahi_serviceresolver.h"
#include "avahi_hostnameresolver.h"

using namespace XMPP;

typedef QMap<QString,QByteArray> PresenceData;

class LinkLocal : public QObject {
	Q_OBJECT
public:
	LinkLocal();
	~LinkLocal();

	bool reset();

	bool registerSrv(const QList<QByteArray> &txt);
	bool updateSrv(const QList<QByteArray> &txt);
	bool unregisterSrv();

	void startDiscovery();
	void stopDiscovery();
	QString getHostname();

	bool isConnected();

	class Stream;
	Stream *stream();
	Stream *ensureStream(Jid);

	void setPresence(const XMPP::Status &s);

public slots:
	void itemNew(int interface, int protocol, const QString &name, const QString &type, const QString &domain, unsigned int flags);
	void itemRemove(int interface, int protocol, const QString &name, const QString &type, const QString &domain, unsigned int flags);
	void update(const QString &name,const QString &domain,const QString &host, ushort &port, const QList<QByteArray>& txt);
	void error(const QString &msg);
	void allForNow();
	void c2c_connectionReady(int s);
	void removeStream(LinkLocal::Stream *);

signals:
	void rosterItemUpdated(const RosterItem &);
	void presence(const Jid &, const Status &);
	void readyRead(Stanza);

private:
	class Resolve;
	class ContactData;
	LinkLocal::Stream *stream_;

public:
	org::freedesktop::Avahi::Server *server_;
	org::freedesktop::Avahi::EntryGroup *entryGroup_;
	org::freedesktop::Avahi::ServiceBrowser *discovery_;
	QDBusConnection con;
	bool registred_;
	QString port_;
	Jid jid_;
	QString nick_;
	const QString service_;
	const QString root_;
	QMap<QString,ContactData*> contacts_;
	QList<Stream*> streams_;
	ServSock c2c;
};


class LinkLocal::Stream : public XMPP::Stream {
	Q_OBJECT
public:
	Stream();
	Stream(Jid jidLocal, BSocket *bs);
	Stream(Jid jidLocal, QString addr, quint16 port);
	~Stream();

	QDomDocument & doc() const;

	QString baseNS() const;
	bool old() const;

	bool compare(QHostAddress,quint16);

	void ready();
	void close();

	Stanza read();
	void write(const Stanza &s);

	bool stanzaAvailable() const;

	int errorCondition() const;
	QString errorText() const;
	QDomElement errorAppSpec() const;

signals:
	void incomingXml(const QString &s);
	void outgoingXml(const QString &s);

	void readyRead(Stanza);
	void cleanUp(LinkLocal::Stream*);

public slots:
	void bs_readyRead();
	void connected();
	void doReadyRead();
	void error(int);
	void autoDelete();

private:
	int state;
	BSocket *bs_;
	Jid jidLocal_;
	Jid jidRemote_;
	CoreProtocol *proto;
	QList<Stanza*> in;
	QList<Stanza> waiting;
	bool active_;
	Task *root;
	bool inUse;

	void processNext();
};

class LinkLocal::ContactData : public QObject {
	Q_OBJECT
public:
	Jid jid;
	QHostAddress addr;
	quint16 port;
	PresenceData presence;
public slots:
	void setAddress(int, int,const QString&, int, const QString &address, uint) {
		printf("address: %s\n",  address.toAscii().data());
		addr = QHostAddress(address);
	}
};

