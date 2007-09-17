#ifndef UPNP_DEVICE_H
#define UPNP_DEVICE_H

//s#define UPNP_WANIP "urn:schemas-upnp-org:service:WANIPConnection:1"
//#define UPNP_WANPPP "urn:schemas-upnp-org:service:WANPPPConnection:1"

#include "socks.h"
#include <QObject>
#include <QList>
#include <QHostAddress>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QHttp>

#include "upnp.h"
class SIMUPNP;

class SIMUPNP::Device : QObject {
	Q_OBJECT
public:
	Device(SIMUPNP *upnp);
	~Device();

	void get();
	void post(QString &soap, QString &soapAction);

	bool isReady();

	QString url();
	void setUrl(const QString &);

	QString controlUrl();
	int port();
	void setPort(int);
	QString hostname();
	void setHostname(const QString &);
	QString serviceType();
	

public slots:

	void getExternalIP();

	void on_upnp_xml();
	void on_upnp_get_external_ip_response();

public:
	QString url_;
	QString hostname_;
	int port_;
	QHttp http_;
	QTcpSocket *sock_;
	QString controlUrl_;
	QString serviceType_;
	SIMUPNP *upnp_;
	int externIPtry_;
};

#endif
