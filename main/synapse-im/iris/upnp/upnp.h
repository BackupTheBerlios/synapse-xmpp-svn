#ifndef UPNP_H
#define UPNP_H

#define UPNP_WANIP "urn:schemas-upnp-org:service:WANIPConnection:1"
#define UPNP_WANPPP "urn:schemas-upnp-org:service:WANPPPConnection:1"

#include "socks.h"
#include <QObject>
#include <QList>
#include <QHostAddress>
#include <QUdpSocket>
#include <QMutex>

class SIMUPNP : QObject {
	Q_OBJECT
public:
	SIMUPNP(SocksServer *);
	~SIMUPNP();

	void rebind(QHostAddress listen_addr = QHostAddress());
	void unbind();
	void discoverDevice();

	SocksServer *server();
	void setExternalPort(quint16);
	quint16 externalPort();
	void setLocalPort(quint16);
	quint16 localPort();
	void setProtocol(QString &_protocol);
	QString protocol();
	void setListenAddress(QHostAddress);
	QHostAddress listenAddress();
	QString leaseDuration();
	QString userAgent();
	void setExternalIP(QString &);
	QString externalIP();

	class Device;

public slots:
	void send_request();
	void timeout();
	void on_reply();

private:
	bool discoveryDone;
	int retry_;
	int error_;
	QUdpSocket *upnp;
	QList<Device *> devices;
	int localPort_;
	int externalPort_;
	QHostAddress local_addr_;
	QHostAddress listen_addr_;
	SocksServer *serv;
	QString protocol_;
	QString externalIP_;
	QMutex mutex;
};

#endif
