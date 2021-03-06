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
	class Port;
	class Device;
	SIMUPNP(SocksServer *);
	~SIMUPNP();

	static SIMUPNP* instance();

	void rebind(QHostAddress listen_addr = QHostAddress());
	void unbind();
	void discoverDevice();

	void registerPort(SIMUPNP::Port*);
	void unregisterPort(SIMUPNP::Port*);

	SocksServer *server();
	void setListenAddress(QHostAddress);
	QHostAddress listenAddress();
	QString leaseDuration();
	QString userAgent();
	void setExternalIP(QString &);
	QString &externalIP();

	quint16 getPort(int protocol);
	void freePort(int protocol, quint16 port);
	quint16 randomPort();

public slots:
	void reset();

	void send_request();
	void timeout();
	void on_reply();

private:
	bool discoveryDone;
	int retry_;
	int error_;
	QUdpSocket *upnp;
	QList<Device*> devices;
	QHostAddress local_addr_;
	QHostAddress listen_addr_;
	SocksServer *serv;
//	QString protocol_;
	QString externalIP_;
	QMutex mutex;
	QList<Port*> ports_;
	static SIMUPNP* instance_;
};

#endif
