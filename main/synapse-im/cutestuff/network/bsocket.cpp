/*
 * bsocket.cpp - QSocket wrapper based on Bytestream with SRV DNS support
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

#include <QTcpSocket>
#include <QHostAddress>
#include <QMetaType>

#include "bsocket.h"

#include "qjdns.h"

#ifdef BS_DEBUG
#include <stdio.h>
#endif

#define READBUFSIZE 65536

// CS_NAMESPACE_BEGIN

class QTcpSocketSignalRelay : public QObject
{
	Q_OBJECT
public:
	QTcpSocketSignalRelay(QTcpSocket *sock, QObject *parent = 0)
	:QObject(parent)
	{
		qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
		connect(sock, SIGNAL(hostFound()), SLOT(sock_hostFound()), Qt::QueuedConnection);
		connect(sock, SIGNAL(connected()), SLOT(sock_connected()), Qt::QueuedConnection);
		connect(sock, SIGNAL(disconnected()), SLOT(sock_disconnected()), Qt::QueuedConnection);
		connect(sock, SIGNAL(readyRead()), SLOT(sock_readyRead()), Qt::QueuedConnection);
		connect(sock, SIGNAL(bytesWritten(qint64)), SLOT(sock_bytesWritten(qint64)), Qt::QueuedConnection);
		connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(sock_error(QAbstractSocket::SocketError)), Qt::QueuedConnection);
	}

signals:
	void hostFound();
	void connected();
	void disconnected();
	void readyRead();
	void bytesWritten(qint64);
	void error(QAbstractSocket::SocketError);

public slots:
	void sock_hostFound()
	{
		emit hostFound();
	}

	void sock_connected()
	{
		emit connected();
	}

	void sock_disconnected()
	{
		emit disconnected();
	}

	void sock_readyRead()
	{
		emit readyRead();
	}

	void sock_bytesWritten(qint64 x)
	{
		emit bytesWritten(x);
	}

	void sock_error(QAbstractSocket::SocketError x)
	{
		emit error(x);
	}
};

class BSocket::Private
{
public:
	Private()
	{
		qsock = 0;
		qsock_relay = 0;
	}

	QTcpSocket *qsock;
	QTcpSocketSignalRelay *qsock_relay;
	int state;

	QJDns jdns;
	QList<QJDns::Record> servers;
	QHash<int,int> type;
	QString host;
	int port;
};

BSocket::BSocket(QObject *parent)
:ByteStream(parent)
{
	d = new Private;

	if(!d->jdns.init(QJDns::Unicast, QHostAddress::Any))
		qDebug("Could not bind to DNS\n");
	connect(&d->jdns, SIGNAL(resultsReady(int, const QJDns::Response &)), SLOT(jdns_done(int, const QJDns::Response &)));
	connect(&d->jdns, SIGNAL(error(int, QJDns::Error)), SLOT(jdns_error(int, QJDns::Error)));
	d->jdns.setNameServers(QJDns::systemInfo().nameServers);
	
	reset();
}

BSocket::~BSocket()
{
	reset(true);
	delete d;
}

void BSocket::reset(bool clear)
{
	if(d->qsock) {
		delete d->qsock_relay;
		d->qsock_relay = 0;

		delete d->qsock;
		d->qsock = 0;
	}
	else {
		if(clear)
			clearReadBuffer();
	}

	d->servers.clear();
	d->state = Idle;
}

void BSocket::ensureSocket()
{
	if(!d->qsock) {
		d->qsock = new QTcpSocket;
#if QT_VERSION >= 0x030200
		d->qsock->setReadBufferSize(READBUFSIZE);
#endif
		d->qsock_relay = new QTcpSocketSignalRelay(d->qsock);
		connect(d->qsock_relay, SIGNAL(hostFound()), SLOT(qs_hostFound()));
		connect(d->qsock_relay, SIGNAL(connected()), SLOT(qs_connected()));
		connect(d->qsock_relay, SIGNAL(disconnected()), SLOT(qs_closed()));
		connect(d->qsock_relay, SIGNAL(readyRead()), SLOT(qs_readyRead()));
		connect(d->qsock_relay, SIGNAL(bytesWritten(qint64)), SLOT(qs_bytesWritten(qint64)));
		connect(d->qsock_relay, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(qs_error(QAbstractSocket::SocketError)));
	}
}

void BSocket::connectToHost(const QString &host, quint16 port)
{
	reset(true);
	d->host = host;
	d->port = port;
	QHostAddress addr;
	if(addr.setAddress(host)) {
		printf("con1\n");
		d->state = Connecting;
		do_connect();
	} else {
		printf("con2\n");
		d->state = HostLookup;
		d->type[d->jdns.queryStart( d->host.toLatin1(), QJDns::A)] = QJDns::A;
	}
}

void BSocket::connectToServer(const QString &srv, const QString &type)
{
	reset(true);
	d->state = HostLookup;
	d->type[d->jdns.queryStart( QString("_%1._tcp.%2").arg(type).arg(d->host).toLatin1(), QJDns::A)] = QJDns::A;
}

int BSocket::socket() const
{
	if(d->qsock)
		return d->qsock->socketDescriptor();
	else
		return -1;
}

void BSocket::setSocket(int s)
{
	reset(true);
	ensureSocket();
	d->state = Connected;
	d->qsock->setSocketDescriptor(s);
}

int BSocket::state() const
{
	return d->state;
}

bool BSocket::isOpen() const
{
	if(d->state == Connected)
		return true;
	else
		return false;
}

void BSocket::close()
{
	if(d->state == Idle)
		return;

	if(d->qsock) {
		d->qsock->close();
		d->state = Closing;
		if(d->qsock->bytesToWrite() == 0)
			reset();
	}
	else {
		reset();
	}
}

void BSocket::write(const QByteArray &a)
{
	if(d->state != Connected)
		return;
#ifdef BS_DEBUG
	QString s = QString::fromUtf8(a);
	fprintf(stderr, "BSocket: writing [%d]: {%s}\n", a.size(), s.latin1());
#endif
	d->qsock->write(a.data(), a.size());
}

QByteArray BSocket::read(int bytes)
{
	QByteArray block;
	if(d->qsock) {
		int max = bytesAvailable();
		if(bytes <= 0 || bytes > max)
			bytes = max;
		block.resize(bytes);
		d->qsock->read(block.data(), block.size());
	}
	else
		block = ByteStream::read(bytes);

#ifdef BS_DEBUG
	QString s = QString::fromUtf8(block);
	fprintf(stderr, "BSocket: read [%d]: {%s}\n", block.size(), s.latin1());
#endif
	return block;
}

int BSocket::bytesAvailable() const
{
	if(d->qsock)
		return d->qsock->bytesAvailable();
	else
		return ByteStream::bytesAvailable();
}

int BSocket::bytesToWrite() const
{
	if(!d->qsock)
		return 0;
	return d->qsock->bytesToWrite();
}

QHostAddress BSocket::address() const
{
	if(d->qsock)
		return d->qsock->localAddress();
	else
		return QHostAddress();
}

quint16 BSocket::port() const
{
	if(d->qsock)
		return d->qsock->localPort();
	else
		return 0;
}

QHostAddress BSocket::peerAddress() const
{
	if(d->qsock)
		return d->qsock->peerAddress();
	else
		return QHostAddress();
}

quint16 BSocket::peerPort() const
{
	if(d->qsock)
		return d->qsock->peerPort();
	else
		return 0;
}

void BSocket::jdns_done(int id, const QJDns::Response &resp)
{
	if(d->type[id] == QJDns::Srv) {
		srv_done(id, resp);
	} else if((d->type[id] == QJDns::A) || (d->type[id] == QJDns::Aaaa)) {
		ndns_done(id, resp);
	}
}

void BSocket::jdns_error(int id, QJDns::Error e)
{
	QString str;
	if(e == QJDns::ErrorGeneric)
		str = "Generic";
	else if(e == QJDns::ErrorNXDomain) {
		str = "NXDomain";
	} else if(e == QJDns::ErrorTimeout)
		str = "Timeout";
	else if(e == QJDns::ErrorConflict)
		str = "Conflict";
#ifdef BS_DEBUG
	fprintf(stderr, "BSocket[%d]: Error resolving hostname : %s\n", id, qPrintable(str));
#endif
	error(ErrHostNotFound);
}

void BSocket::srv_done(int id, const QJDns::Response &resp)
{
	d->servers = resp.answerRecords;
	tryNext();
}

void BSocket::ndns_done(int id, const QJDns::Response &resp)
{
	d->host = resp.answerRecords[0].address.toString();
	d->state = Connecting;
	do_connect();
}

void BSocket::tryNext()
{
	d->host = d->servers.first().name;
	d->port = d->servers.first().port;
	d->servers.removeFirst();
	do_connect();
}

void BSocket::do_connect()
{
#ifdef BS_DEBUG
	fprintf(stderr, "BSocket: Connecting to %s:%d\n", d->host.latin1(), d->port);
#endif
	ensureSocket();
	d->qsock->connectToHost(d->host, d->port);
}

void BSocket::qs_hostFound()
{
}

void BSocket::qs_connected()
{
	d->state = Connected;
#ifdef BS_DEBUG
	fprintf(stderr, "BSocket: Connected.\n");
#endif
	connected();
}

void BSocket::qs_closed()
{
	if(d->state == Closing)
	{
#ifdef BS_DEBUG
		fprintf(stderr, "BSocket: Delayed Close Finished.\n");
#endif
		reset();
		delayedCloseFinished();
	}
}

void BSocket::qs_readyRead()
{
	readyRead();
}

void BSocket::qs_bytesWritten(qint64 x64)
{
	int x = x64;
#ifdef BS_DEBUG
	fprintf(stderr, "BSocket: BytesWritten [%d].\n", x);
#endif
	bytesWritten(x);
}

void BSocket::qs_error(QAbstractSocket::SocketError x)
{
	if(x == QTcpSocket::RemoteHostClosedError) {
#ifdef BS_DEBUG
		fprintf(stderr, "BSocket: Connection Closed.\n");
#endif
		reset();
		connectionClosed();
		return;
	}

#ifdef BS_DEBUG
	fprintf(stderr, "BSocket: Error.\n");
#endif

	// connection error during SRV host connect?  try next
	if(d->state == HostLookup && (x == QTcpSocket::ConnectionRefusedError || x == QTcpSocket::HostNotFoundError)) {
		tryNext();
		return;
	}

	reset();
	if(x == QTcpSocket::ConnectionRefusedError)
		error(ErrConnectionRefused);
	else if(x == QTcpSocket::HostNotFoundError)
		error(ErrHostNotFound);
	else
		error(ErrRead);
}

#include "bsocket.moc"

// CS_NAMESPACE_END
