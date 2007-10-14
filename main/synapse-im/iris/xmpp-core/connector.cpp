/*
 * connector.cpp - establish a connection to an XMPP server
 * Copyright (C) 2003  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
  TODO:

  - Test and analyze all possible branches

  XMPP::AdvancedConnector is "good for now."  The only real issue is that
  most of what it provides is just to work around the old Jabber/XMPP 0.9
  connection behavior.  When XMPP 1.0 has taken over the world, we can
  greatly simplify this class.  - Sep 3rd, 2003.
*/

#include "xmpp.h"

#include <qpointer.h>
#include <qca.h>
#include <QList>
#include <QUrl>
#include "safedelete.h"
#include <libidn/idna.h>

#include <QHash>
#include "jdns.h"

#include "bsocket.h"
#include "httpconnect.h"
#include "httppoll.h"
#include "socks.h"

//#define XMPP_DEBUG

using namespace XMPP;

//----------------------------------------------------------------------------
// Connector
//----------------------------------------------------------------------------
Connector::Connector(QObject *parent)
:QObject(parent)
{
	setUseSSL(false);
	setPeerAddressNone();
}

Connector::~Connector()
{
}

bool Connector::useSSL() const
{
	return ssl;
}

bool Connector::havePeerAddress() const
{
	return haveaddr;
}

QHostAddress Connector::peerAddress() const
{
	return addr;
}

quint16 Connector::peerPort() const
{
	return port;
}

void Connector::setUseSSL(bool b)
{
	ssl = b;
}

void Connector::setPeerAddressNone()
{
	haveaddr = false;
	addr = QHostAddress();
	port = 0;
}

void Connector::setPeerAddress(const QHostAddress &_addr, quint16 _port)
{
	haveaddr = true;
	addr = _addr;
	port = _port;
}


//----------------------------------------------------------------------------
// AdvancedConnector::Proxy
//----------------------------------------------------------------------------
AdvancedConnector::Proxy::Proxy()
{
	t = None;
	v_poll = 30;
}

AdvancedConnector::Proxy::~Proxy()
{
}

int AdvancedConnector::Proxy::type() const
{
	return t;
}

QString AdvancedConnector::Proxy::host() const
{
	return v_host;
}

quint16 AdvancedConnector::Proxy::port() const
{
	return v_port;
}

QString AdvancedConnector::Proxy::url() const
{
	return v_url;
}

QString AdvancedConnector::Proxy::user() const
{
	return v_user;
}

QString AdvancedConnector::Proxy::pass() const
{
	return v_pass;
}

int AdvancedConnector::Proxy::pollInterval() const
{
	return v_poll;
}

void AdvancedConnector::Proxy::setHttpConnect(const QString &host, quint16 port)
{
	t = HttpConnect;
	v_host = host;
	v_port = port;
}

void AdvancedConnector::Proxy::setHttpPoll(const QString &host, quint16 port, const QString &url)
{
	t = HttpPoll;
	v_host = host;
	v_port = port;
	v_url = url;
}

void AdvancedConnector::Proxy::setSocks(const QString &host, quint16 port)
{
	t = Socks;
	v_host = host;
	v_port = port;
}

void AdvancedConnector::Proxy::setUserPass(const QString &user, const QString &pass)
{
	v_user = user;
	v_pass = pass;
}

void AdvancedConnector::Proxy::setPollInterval(int secs)
{
	v_poll = secs;
}


//----------------------------------------------------------------------------
// AdvancedConnector
//----------------------------------------------------------------------------
enum { Idle, Connecting, Connected };
class AdvancedConnector::Private
{
public:
	int mode;
	ByteStream *bs;
	QJDns jdns;

	QString server;
	QString opt_host;
	int opt_port;
	bool opt_probe, opt_ssl;
	Proxy proxy;

	QString host;
	int port;
	QList<QJDns::Record> servers;
	QHash<int, int> type;
	int errorCode;

	bool multi, using_srv;
	bool will_be_ssl;
	int probe_mode;

	bool aaaa;
	SafeDelete sd;
};

AdvancedConnector::AdvancedConnector(QObject *parent)
:Connector(parent)
{
	d = new Private;
	d->bs = 0;
	d->opt_probe = false;
	d->opt_ssl = false;
	cleanup();
	if(!d->jdns.init(QJDns::Unicast, QHostAddress::Any))
		qDebug("Could not bind to DNS\n");
	connect(&d->jdns, SIGNAL(resultsReady(int, const QJDns::Response &)), SLOT(jdns_done(int, const QJDns::Response &)));
	connect(&d->jdns, SIGNAL(error(int, QJDns::Error)), SLOT(jdns_error(int, QJDns::Error)));
	d->jdns.setNameServers(QJDns::systemInfo().nameServers);
	d->errorCode = 0;
}

AdvancedConnector::~AdvancedConnector()
{
	cleanup();
	delete d;
}

void AdvancedConnector::cleanup()
{
	d->mode = Idle;
	d->type.clear();

	// destroy the bytestream, if there is one
	delete d->bs;
	d->bs = 0;

	d->multi = false;
	d->using_srv = false;
	d->will_be_ssl = false;
	d->probe_mode = -1;

	setUseSSL(false);
	setPeerAddressNone();
}

void AdvancedConnector::setProxy(const Proxy &proxy)
{
	if(d->mode != Idle)
		return;
	d->proxy = proxy;
}

void AdvancedConnector::setOptHostPort(const QString &host, quint16 _port)
{
	if(d->mode != Idle)
		return;
	d->opt_host = host;
	d->opt_port = _port;
}

void AdvancedConnector::setOptProbe(bool b)
{
	if(d->mode != Idle)
		return;
	d->opt_probe = b;
}

void AdvancedConnector::setOptSSL(bool b)
{
	if(d->mode != Idle)
		return;
	d->opt_ssl = b;
}

void AdvancedConnector::connectToServer(const QString &server)
{
	if(d->mode != Idle)
		return;
	if(server.isEmpty())
		return;

	d->errorCode = 0;
	d->mode = Connecting;
	d->aaaa = true;

	// Encode the servername
	d->server = QUrl::toAce(server);
	//char* server_encoded;
	//if (!idna_to_ascii_8z(server.utf8().data(), &server_encoded, 0)) {
	//	d->server = QString(server_encoded);
	//	free(server_encoded);
	//}
	//else {
	//	d->server = server;
	//}

	if(d->proxy.type() == Proxy::HttpPoll) {
		// need SHA1 here
		//if(!QCA::isSupported(QCA::CAP_SHA1))
		//	QCA::insertProvider(createProviderHash());

		HttpPoll *s = new HttpPoll;
		d->bs = s;
		connect(s, SIGNAL(connected()), SLOT(bs_connected()));
		connect(s, SIGNAL(syncStarted()), SLOT(http_syncStarted()));
		connect(s, SIGNAL(syncFinished()), SLOT(http_syncFinished()));
		connect(s, SIGNAL(error(int)), SLOT(bs_error(int)));
		if(!d->proxy.user().isEmpty())
			s->setAuth(d->proxy.user(), d->proxy.pass());
		s->setPollInterval(d->proxy.pollInterval());

		if(d->proxy.host().isEmpty())
			s->connectToUrl(d->proxy.url());
		else
			s->connectToHost(d->proxy.host(), d->proxy.port(), d->proxy.url());
	}
	else if (d->proxy.type() == Proxy::HttpConnect) {
		if(!d->opt_host.isEmpty()) {
			d->host = d->opt_host;
			d->port = d->opt_port;
		}
		else {
			d->host = server;
			d->port = 5222;
		}
		do_connect();
	}
	else {
		if(!d->opt_host.isEmpty()) {
			d->host = d->opt_host;
			d->port = d->opt_port;
			do_resolve();
		}
		else {
			d->multi = true;

			QPointer<QObject> self = this;
			srvLookup(d->server);
			if(!self)
				return;

			d->type[d->jdns.queryStart( QString("_xmpp-client._tcp.%1").arg(d->server).toLatin1(), QJDns::Srv)] = QJDns::Srv;
		}
	}
}

void AdvancedConnector::changePollInterval(int secs)
{
	if(d->bs && (d->bs->inherits("XMPP::HttpPoll") || d->bs->inherits("HttpPoll"))) {
		HttpPoll *s = static_cast<HttpPoll*>(d->bs);
		s->setPollInterval(secs);
	}
}

ByteStream *AdvancedConnector::stream() const
{
	if(d->mode == Connected)
		return d->bs;
	else
		return 0;
}

void AdvancedConnector::done()
{
	cleanup();
}

int AdvancedConnector::errorCode() const
{
	return d->errorCode;
}

void AdvancedConnector::do_resolve()
{
	d->type[d->jdns.queryStart(d->host.toLatin1(), QJDns::A)] = QJDns::A;
}

void AdvancedConnector::jdns_done(int id, const QJDns::Response &resp)
{
	if(d->type[id] == QJDns::Srv) {
		srv_done(id, resp);
	} else if((d->type[id] == QJDns::A) || (d->type[id] == QJDns::Aaaa)) {
		dns_done(id, resp);
	}
}

void AdvancedConnector::jdns_error(int id, QJDns::Error e)
{
	QString str;
	if(e == QJDns::ErrorGeneric)
		str = "Generic";
	else if(e == QJDns::ErrorNXDomain) {
		if(d->type[id] == QJDns::Srv) {
			srvResult(false);

#ifdef XMPP_DEBUG
			printf("srv_done1.1\n");
#endif
			// fall back to A record
			d->using_srv = false;
			d->host = d->server;
			if(d->opt_probe) {
#ifdef XMPP_DEBUG
				printf("srv_done1.1.1\n");
#endif
				d->probe_mode = 0;
				d->port = 5223;
				d->will_be_ssl = true;
			}
			else {
#ifdef XMPP_DEBUG
				printf("srv_done1.1.2\n");
#endif
				d->probe_mode = 1;
				d->port = 5222;
			}
			do_resolve();
			return;
		} else {
			str = "NXDomain";
		}
	} else if(e == QJDns::ErrorTimeout)
		str = "Timeout";
	else if(e == QJDns::ErrorConflict)
		str = "Conflict";
#ifdef XMPP_DEBUG
	printf("[%d] Error: %s\n", id, qPrintable(str));
#endif
	d->jdns.shutdown();
}

void AdvancedConnector::dns_done(int id, const QJDns::Response &resp)
{
	bool failed = false;
	QHostAddress addr;

	if(resp.answerRecords.isEmpty())
		failed = true;
	else
		addr = resp.answerRecords[0].address;

	if(failed) {
#ifdef XMPP_DEBUG
		printf("dns1\n");
#endif
		// using proxy?  then try the unresolved host through the proxy
		if(d->proxy.type() != Proxy::None) {
#ifdef XMPP_DEBUG
			printf("dns1.1\n");
#endif
			do_connect();
		}
		else if(d->using_srv) {
#ifdef XMPP_DEBUG
			printf("dns1.2\n");
#endif
			if(d->servers.isEmpty()) {
#ifdef XMPP_DEBUG
				printf("dns1.2.1\n");
#endif
				cleanup();
				d->errorCode = ErrConnectionRefused;
				error();
			}
			else {
#ifdef XMPP_DEBUG
				printf("dns1.2.2\n");
#endif
				tryNextSrv();
				return;
			}
		}
		else {
#ifdef XMPP_DEBUG
			printf("dns1.3\n");
#endif
			cleanup();
			d->errorCode = ErrHostNotFound;
			error();
		}
	}
	else {
#ifdef XMPP_DEBUG
		printf("dns2\n");
#endif
		d->host = addr.toString();
		do_connect();
	}
}

void AdvancedConnector::do_connect()
{
#ifdef XMPP_DEBUG
	printf("trying %s:%d\n", d->host.latin1(), d->port);
#endif
	int t = d->proxy.type();
	if(t == Proxy::None) {
#ifdef XMPP_DEBUG
		printf("do_connect1\n");
#endif
		BSocket *s = new BSocket;
		d->bs = s;
		connect(s, SIGNAL(connected()), SLOT(bs_connected()));
		connect(s, SIGNAL(error(int)), SLOT(bs_error(int)));
		s->connectToHost(d->host, d->port);
	}
	else if(t == Proxy::HttpConnect) {
#ifdef XMPP_DEBUG
		printf("do_connect2\n");
#endif
		HttpConnect *s = new HttpConnect;
		d->bs = s;
		connect(s, SIGNAL(connected()), SLOT(bs_connected()));
		connect(s, SIGNAL(error(int)), SLOT(bs_error(int)));
		if(!d->proxy.user().isEmpty())
			s->setAuth(d->proxy.user(), d->proxy.pass());
		s->connectToHost(d->proxy.host(), d->proxy.port(), d->host, d->port);
	}
	else if(t == Proxy::Socks) {
#ifdef XMPP_DEBUG
		printf("do_connect3\n");
#endif
		SocksClient *s = new SocksClient;
		d->bs = s;
		connect(s, SIGNAL(connected()), SLOT(bs_connected()));
		connect(s, SIGNAL(error(int)), SLOT(bs_error(int)));
		if(!d->proxy.user().isEmpty())
			s->setAuth(d->proxy.user(), d->proxy.pass());
		s->connectToHost(d->proxy.host(), d->proxy.port(), d->host, d->port);
	}
}

void AdvancedConnector::tryNextSrv()
{
#ifdef XMPP_DEBUG
	printf("trying next srv\n");
#endif
	d->host = d->servers.first().name;
	d->port = d->servers.first().port;
	d->servers.removeFirst();
	do_resolve();
}

void AdvancedConnector::srv_done(int id, const QJDns::Response &resp)
{
	QPointer<QObject> self = this;
#ifdef XMPP_DEBUG
	printf("srv_done1\n");
#endif
	d->servers = resp.answerRecords;
	if(d->servers.isEmpty()) {
#ifdef XMPP_DEBUG
		printf("should not happend\n");
#endif
		return;
	}

	srvResult(true);
	if(!self)
		return;

	d->using_srv = true;
	tryNextSrv();
}

void AdvancedConnector::bs_connected()
{
	if(d->proxy.type() == Proxy::None) {
		QHostAddress h = (static_cast<BSocket*>(d->bs))->peerAddress();
		int p = (static_cast<BSocket*>(d->bs))->peerPort();
		setPeerAddress(h, p);
	}

	// only allow ssl override if proxy==poll or host:port
	if((d->proxy.type() == Proxy::HttpPoll || !d->opt_host.isEmpty()) && d->opt_ssl)
		setUseSSL(true);
	else if(d->will_be_ssl)
		setUseSSL(true);

	d->jdns.shutdown();
	d->mode = Connected;
	connected();
}

void AdvancedConnector::bs_error(int x)
{
	if(d->mode == Connected) {
		d->errorCode = ErrStream;
		error();
		return;
	}

	bool proxyError = false;
	int err = ErrConnectionRefused;
	int t = d->proxy.type();

#ifdef XMPP_DEBUG
	printf("bse1\n");
#endif

	// figure out the error
	if(t == Proxy::None) {
		if(x == BSocket::ErrHostNotFound)
			err = ErrHostNotFound;
		else
			err = ErrConnectionRefused;
	}
	else if(t == Proxy::HttpConnect) {
		if(x == HttpConnect::ErrConnectionRefused)
			err = ErrConnectionRefused;
		else if(x == HttpConnect::ErrHostNotFound)
			err = ErrHostNotFound;
		else {
			proxyError = true;
			if(x == HttpConnect::ErrProxyAuth)
				err = ErrProxyAuth;
			else if(x == HttpConnect::ErrProxyNeg)
				err = ErrProxyNeg;
			else
				err = ErrProxyConnect;
		}
	}
	else if(t == Proxy::HttpPoll) {
		if(x == HttpPoll::ErrConnectionRefused)
			err = ErrConnectionRefused;
		else if(x == HttpPoll::ErrHostNotFound)
			err = ErrHostNotFound;
		else {
			proxyError = true;
			if(x == HttpPoll::ErrProxyAuth)
				err = ErrProxyAuth;
			else if(x == HttpPoll::ErrProxyNeg)
				err = ErrProxyNeg;
			else
				err = ErrProxyConnect;
		}
	}
	else if(t == Proxy::Socks) {
		if(x == SocksClient::ErrConnectionRefused)
			err = ErrConnectionRefused;
		else if(x == SocksClient::ErrHostNotFound)
			err = ErrHostNotFound;
		else {
			proxyError = true;
			if(x == SocksClient::ErrProxyAuth)
				err = ErrProxyAuth;
			else if(x == SocksClient::ErrProxyNeg)
				err = ErrProxyNeg;
			else
				err = ErrProxyConnect;
		}
	}

	// no-multi or proxy error means we quit
	if(!d->multi || proxyError) {
		cleanup();
		d->errorCode = err;
		error();
		return;
	}

	if(d->using_srv && !d->servers.isEmpty()) {
#ifdef XMPP_DEBUG
		printf("bse1.1\n");
#endif
		tryNextSrv();
	}
	else if(!d->using_srv && d->opt_probe && d->probe_mode == 0) {
#ifdef XMPP_DEBUG
		printf("bse1.2\n");
#endif
		d->probe_mode = 1;
		d->port = 5222;
		d->will_be_ssl = false;
		do_connect();
	}
	else {
#ifdef XMPP_DEBUG
		printf("bse1.3\n");
#endif
		cleanup();
		d->errorCode = ErrConnectionRefused;
		error();
	}
}

void AdvancedConnector::http_syncStarted()
{
	httpSyncStarted();
}

void AdvancedConnector::http_syncFinished()
{
	httpSyncFinished();
}
