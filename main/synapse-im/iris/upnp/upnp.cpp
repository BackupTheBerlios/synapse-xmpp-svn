#include "upnp.h"

#include <QHttp>
#include <QDataStream>
#include <QString>
#include <QTimer>
#include <QTcpSocket>

bool isLocal(quint32 ip) {
	return ((ip & 0xff000000) == 0x0a000000 || (ip & 0xfff00000) == 0xac100000 || (ip & 0xffff0000) == 0xc0a80000);
}

class SIMUPNP::Device : QObject {
	Q_OBJECT
public:
	Device(SIMUPNP *upnp) : upnp_(upnp)
	{
		sock = new QTcpSocket();
	}
	~Device() {};

	void get() {
		QString get = QString("GET ") + url + QString(" HTTP/1.1\r\n") + QString("Host: ") + hostname + ":" + QString("%1").arg(port) + "\r\n" + QString("User-Agent: Synapse-IM\r\nConnection: close\r\n\r\n");
		sock->disconnect();
		connect(sock, SIGNAL(disconnected()), this, SLOT(on_upnp_xml()));
		serviceType = "";
		sock->connectToHost(hostname, port);

		sock->waitForConnected(10000);
		upnp_->setListenAddress(sock->localAddress());
		sock->write(get.toUtf8());
	}

	void post(QString &soap, QString &soapAction) {
		QString header = QString("POST ") + controlUrl + QString(" HTTP/1.1\r\n") + QString("Host: ") + hostname + QString(":") + QString("%1").arg(port) + QString("\r\n") + QString("User-Agent: Synapse-IM\r\n")  + QString("Content-Type: text/xml; charset=\"utf-8\"\r\n") + QString("Content-Length: ") + QString("%1").arg(soap.size()) + QString("\r\nConnection: close\r\n") + QString("Soapaction: \"") + serviceType + QString("#") + soapAction + QString("\"\r\n\r\n") + soap;

		sock->waitForConnected(10000);
		sock->write(header.toUtf8());
	}

	void mapPort() {
		sock->connectToHost(hostname, port);
		connect(sock, SIGNAL(disconnected()), this, SLOT(on_upnp_map_response()));
		QString soap_action = "AddPortMapping";
		QString soap;
		soap = QString( "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" " ) + QString( "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\n") + QString("<s:Body><u:") + soap_action + QString(" xmlns:u=\"") + serviceType + QString("\">\n");

		soap = soap + "<NewRemoteHost></NewRemoteHost>\n" + "<NewExternalPort>" + QString("%1").arg(upnp_->externalPort()) + "</NewExternalPort>\n" + "<NewProtocol>" + upnp_->protocol() + "</NewProtocol>\n" + "<NewInternalPort>" + QString("%1").arg(upnp_->localPort()) + "</NewInternalPort>\n" + "<NewInternalClient>" + upnp_->listenAddress().toString() + "</NewInternalClient>\n" + "<NewEnabled>1</NewEnabled>\n" + "<NewPortMappingDescription>" + upnp_->userAgent() + "</NewPortMappingDescription>\n" + "<NewLeaseDuration>" + upnp_->leaseDuration() + "</NewLeaseDuration>\n";
		soap = soap + "</u:" + soap_action + ">\n</s:Body>\n</s:Envelope>\r\n";

		post(soap,soap_action);
	}

	void unmapPort() {
		sock->connectToHost(hostname, port);
		connect(sock, SIGNAL(disconnected()), this, SLOT(on_upnp_map_response()));
		QString soap_action = "DeletePortMapping";

		QString soap = QString("<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" ") + "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">" + "<s:Body><u:" + soap_action + " xmlns:u=\"" + serviceType + "\">";

		soap = soap + "<NewRemoteHost></NewRemoteHost>" + "<NewExternalPort>" + QString("%1").arg(upnp_->externalPort()) + "</NewExternalPort>" + "<NewProtocol>" + upnp_->protocol() + "</NewProtocol>";
		soap = soap + "</u:" + soap_action + "></s:Body></s:Envelope>";

		post(soap, soap_action);
	}

public slots:
	void getExternalIP() {
		connect(sock, SIGNAL(disconnected()), this, SLOT(on_upnp_get_external_ip_response()));
		if (externIPtry_ == 0) {
			sock->connectToHost(hostname, port);
			QString soap_action = "GetExternalIPAddress";
			QString soap;

			soap = QString( "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" " ) + QString( "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\n") + QString("<s:Body><u:") + soap_action + QString(" xmlns:u=\"") + serviceType + QString("\">\n");

			soap = soap + "</u:" + soap_action + ">\n</s:Body>\n</s:Envelope>\r\n";
			post(soap,soap_action);
		} else if (externIPtry_ == 1) {
			sock->connectToHost("checkip.dyndns.org", 80);

			sock->waitForConnected(10000);
			QString header("GET / HTTP/1.1\r\nHost: checkip.dyndns.org\r\nConnection: close\r\n\r\n");
			sock->write(header.toUtf8());
		}
	}

	void on_upnp_xml()
	{
		QString doc(sock->readAll());
		//delete sock;
		sock->disconnect();

		if (doc.left(15).compare("HTTP/1.1 200 OK") != 0) {
			qDebug("UPNP::Device::on_upnp_xml() : Bad response\n");
			return;
		}

		int i = doc.find( UPNP_WANIP );
		if(i!=-1) {
			serviceType = UPNP_WANIP;
		} else {
			qWarning("UPNP::Device::on_upnp_xml() : No WANIP service type.\n");
			i = doc.find( UPNP_WANIP );

			if (i!=-1) {
				serviceType = UPNP_WANPPP;
			} else {
				qWarning("UPNP::Device::on_upnp_xml() : No WANPPP service type.\n");
				return;
			}
		}
		int j = doc.find("<controlURL>", i) + sizeof("<controlURL>");
		i = doc.find("</controlURL>",j);
		controlUrl = doc.mid(j-1, i - j +1);

		mapPort();
	}

	void on_upnp_get_external_ip_response()
	{
		QString doc(sock->readAll());
		sock->disconnect();

		QString externIP;
		externIPtry_++;

		if (doc.left(15).compare("HTTP/1.1 200 OK") != 0) {
			qWarning("UPNP::Device::on_upnp_get_external_ip_response() : Bad response\n");
			if ( externIPtry_ > 2 ) {
				upnp_->server()->listen(upnp_->externalPort(),true);
				return;
			}
		} else {

			if (externIPtry_ == 1) {
				int j = doc.find("<NewExternalIPAddress>") + sizeof("<NewExternalIPAddress>");
				int i = doc.find("</NewExternalIPAddress>",j-2);
				externIP = doc.mid(j-1, i - j +1);
			} else if (externIPtry_ == 2) {
				int j = doc.find("Address: ") + sizeof("Address: ");
				int i = doc.find("</body>",j);
				externIP = doc.mid(j-1, i - j +1);
			}
		}

		if (externIP.isEmpty()) {
			if (externIPtry_ == 2)
				upnp_->server()->listen(upnp_->externalPort(),true);
			else {
				getExternalIP();
			}
		} else {
			upnp_->setExternalIP(externIP);
		}
	}

	void on_upnp_map_response()
	{
		QString doc(sock->readAll());
		sock->disconnect();

		if (doc.left(15).compare("HTTP/1.1 200 OK") != 0) {
			qDebug("UPNP::Device::on_upnp_map_resonse() : Bad response\n");
			upnp_->server()->listen(upnp_->externalPort(),true);
			return;
		}

		if (doc.find("UPnPError") != -1)
		{
			//Error
			int j = doc.find("<errorCode>") + sizeof("<errorCode>");
			int i = doc.find("</errorCode>",j);
			int errorCode = doc.mid(j-1, i - j +1).toInt();
			if (errorCode == 725) {
				// only permanent leases supported
				qWarning("UPNP::Device::on_upnp_map_resonse() : Only permanent leases supported\n");
			} else if (errorCode == 718) {
				// conflict in mapping, try next external port
				qWarning("UPNP::Device::on_upnp_map_resonse() : Port already used\n");
				upnp_->setExternalPort(upnp_->externalPort() + 1);
				upnp_->setLocalPort(upnp_->localPort() + 1);
				mapPort();
				return;
			} else if (errorCode == 0) {
				qDebug("UPNP::Device::on_upnp_map_resonse() : unknown error\n");
			}
		}
		externIPtry_ = 0;
		QTimer::singleShot(1000, this, SLOT(getExternalIP()));
		upnp_->server()->listen(upnp_->externalPort(),true);
	}

public:
	QString url;
	QString hostname;
	int port;
	QHttp http;
	QTcpSocket *sock;
	QString controlUrl;
	QString serviceType;
	SIMUPNP *upnp_;
	int externIPtry_;
};

SIMUPNP::SIMUPNP(SocksServer *_serv) : localPort_(8200), externalPort_(8200), protocol_("TCP")
{
	upnp = new QUdpSocket();
	serv = _serv;
};

SIMUPNP::~SIMUPNP()
{
};

void SIMUPNP::rebind(QHostAddress listen_addr)
{
	if (listen_addr.protocol() == QAbstractSocket::IPv4Protocol && !listen_addr.isNull())
	{
		local_addr_ = listen_addr;
		if(!isLocal(local_addr_.toIPv4Address())) {
//			error_ = 1;// NotLocalIP;
			qDebug("UPNP::rebind() : Listen address is not local ip.\n");
			return;
		}
	} else {
		local_addr_ = QHostAddress("0.0.0.0");
	}

	if(serv->isActive() && listen_addr_ == local_addr_)
		return;

	serv->stop();
	retry_ = 0;
	discoveryDone = false;
	discoverDevice();
}

void SIMUPNP::unbind()
{
	serv->stop();
	for (int i = 0; i < devices.count(); ++i)
	{
		devices.takeAt(i)->unmapPort();
	}
}

void SIMUPNP::discoverDevice()
{
	upnp->bind();
	connect(upnp, SIGNAL(readyRead()), this, SLOT(on_reply()));

	retry_ = 0;
	send_request();
}

void SIMUPNP::send_request()
{
	const char msearch[] = 
		"M-SEARCH * HTTP/1.1\r\n"
		"HOST: 239.255.255.250:1900\r\n"
		"ST:upnp:rootdevice\r\n"
		"MAN:\"ssdp:discover\"\r\n"
		"MX:3\r\n"
		"\r\n\r\n";

	if(retry_ < 9) {
		++retry_;
		upnp->writeDatagram(msearch, sizeof(msearch) - 1, QHostAddress("239.255.255.250"), 1900);
		QTimer::singleShot( 250*retry_, this, SLOT(send_request()));
	} else if (retry_ == 9) {
		QTimer::singleShot( 10000, this, SLOT(timeout()));
	}
}

void SIMUPNP::timeout()
{
	if (!devices.isEmpty())
		return;
	serv->listen(externalPort_,true);
}

void SIMUPNP::on_reply()
{
	mutex.lock();
	if (discoveryDone) {
		mutex.unlock();
		return;
	}

	QByteArray datagram;
	datagram.resize(upnp->pendingDatagramSize());
	Device *dev = new Device(this);
	QHostAddress sender;
	quint16 senderPort;

        upnp->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
	
	QString resp(datagram);
	int x = resp.find("http://",6);
	if (x == -1) {
		qDebug("UPNP::on_reply() : Bad response\n");
		mutex.unlock();
		if( retry_ == 9)
			serv->listen(externalPort_,true);
		
		return;
	}
	int y = resp.find("\n",x);

	dev->url = resp.mid(x, y-(x+1));
	if (dev->url.isEmpty()) {
		delete dev;
		mutex.unlock();
		if( retry_ == 9)
			serv->listen(externalPort_,true);
		return;
	}

	bool inList = false;
	//check if it is not already on a list
	for (int i = 0; i < devices.count(); ++i)
		if (devices.takeAt(i)->url.compare(dev->url) == 0)
			inList = true;

	if (!inList) {
		x = dev->url.find(":",6);
		y = dev->url.find("/",x);
		dev->port = (dev->url.mid(x+1, y-(x+1))).toInt();
		dev->hostname = dev->url.mid(7, x-7);
		devices.append(dev);
	}

	if (retry_ >= 4 && !devices.isEmpty() && !discoveryDone)
	{
		discoveryDone = true;
		for (int i = 0; i < devices.count(); ++i)
		{
			devices.takeAt(i)->get();
		}
	}  else if (retry_ == 9 && devices.isEmpty()) {
		serv->listen(externalPort_,true);
	}
	mutex.unlock();
}

SocksServer *SIMUPNP::server()
{
	return serv;
}

void SIMUPNP::setExternalPort(quint16 port)
{
	externalPort_ = port;
}

quint16 SIMUPNP::externalPort()
{
	return externalPort_;
}

void SIMUPNP::setProtocol(QString &_protocol)
{
	protocol_ = _protocol;
}

QString SIMUPNP::protocol()
{
	return protocol_;
}

void SIMUPNP::setLocalPort(quint16 port)
{
	localPort_ = port;
}

quint16 SIMUPNP::localPort()
{
	return localPort_;
}

void SIMUPNP::setListenAddress(QHostAddress addr)
{
	listen_addr_ = addr;
}

QHostAddress SIMUPNP::listenAddress()
{
	return listen_addr_;
	//return serv->address();
}

QString SIMUPNP::userAgent()
{
	return "Synapse-IM";
}

QString SIMUPNP::leaseDuration()
{
	return "0";
}

void SIMUPNP::setExternalIP(QString &externIP)
{
	externalIP_ = externIP;
}

QString SIMUPNP::externalIP()
{
	return externalIP_;
}

#include "upnp.moc"