#ifndef UPNP_PORT_H
#define UPNP_PORT_H

//#define UPNP_WANIP "urn:schemas-upnp-org:service:WANIPConnection:1"
//#define UPNP_WANPPP "urn:schemas-upnp-org:service:WANPPPConnection:1"

#include "socks.h"
#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QMutex>

#include "upnp.h"
//#include "upnp_device.h"
//class SIMUPNP;
class SIMUPNP::Device;

class SIMUPNP::Port : QObject {
	Q_OBJECT
public:
	Port(SIMUPNP::Device *_dev, const QString &_type);
	~Port();

	void post(QString &soap, QString &soapAction);

	void unmap();
	bool mapped();
	
	bool inUse();
	void setInUse(bool);

	quint16 port();
	const QString &type();

public slots:
	void map();

	void on_map_response();

private:
	QTcpSocket *sock_;
	QString type_;
	quint16 port_;
	SIMUPNP::Device *dev;
	bool mapped_;
	bool inUse_;
};

#endif
