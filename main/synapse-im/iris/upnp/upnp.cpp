#include "upnp.h"
#include "upnp_device.h"
#include "upnp_port.h"

#include <QHttp>
#include <QDataStream>
#include <QString>
#include <QTimer>
#include <QTcpSocket>

static bool isLocal(quint32 ip) {
	return ((ip & 0xff000000) == 0x0a000000 || (ip & 0xfff00000) == 0xac100000 || (ip & 0xffff0000) == 0xc0a80000);
}

SIMUPNP::SIMUPNP(SocksServer *_serv)// : localPort_(8200), externalPort_(8200), protocol_("TCP")
{
	printf("SIMUPNP\n");
	upnp = new QUdpSocket();
	serv = _serv;
	ports_.clear();
	SIMUPNP::instance_  = this;
	devices.clear();
	rebind();
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
		QTimer::singleShot( 20000, this, SLOT(timeout()));
	}
}

void SIMUPNP::timeout()
{
	if (ports_.begin() != ports_.end())
		return;

	new Port(NULL, "TCP"); // fix for main port of file transfer
	new Port(NULL, "TCP");
	new Port(NULL, "UDP");
}

void SIMUPNP::on_reply()
{
	if (discoveryDone) {
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
		return;
	}
	int y = resp.find("\n",x);

	dev->setUrl(resp.mid(x, y-(x+1)));
	if (dev->url().isEmpty()) {
		printf("delete dev\n");
		delete dev;
		return;
	}

	bool inList = false;
	//check if it is not already on a list
	QList<Device*>::iterator it1;
	for ( it1 = devices.begin(); it1 != devices.end(); ++it1) {
		Device *dev1 = *it1;
		if (dev1->url().compare(dev->url()) == 0)
			inList = true;
	}

	if (!inList) {
		QString url(dev->url());
		x = url.find(":",6);
		y = url.find("/",x);
		dev->setPort((url.mid(x+1, y-(x+1))).toInt());
		dev->setHostname(url.mid(7, x-7));
		devices.append(dev);
		dev->get();
	}

	if (retry_ >= 4 && (devices.begin() != devices.end() ) && !discoveryDone)
	{
		discoveryDone = true;

		QList<Device*>::iterator it2;
		for ( it2 = devices.begin(); it2 != devices.end(); ++it2) {
			Device *dev2 = *it2;
			//printf("dev: %s\n", dev2->url().toAscii().data());
			new Port(dev2, "TCP"); // fix for main port of file transfer
			new Port(dev2, "TCP");
			new Port(dev2, "UDP");
		}
	}
}

void SIMUPNP::registerPort(SIMUPNP::Port *port)
{
	if((ports_.begin() == ports_.end()) && (port->type().compare("TCP")==0)) {
		serv->stop();
		serv->listen(port->port(),true);
		port->setInUse(true);
	}
	ports_.append(port);

	QList<Port*>::iterator it;
	int i=0;
	for( it=ports_.begin(); it != ports_.end(); ++it) {
		Port *p1 = *it;
		i++;
		if(p1)
			printf("allocated ports: %d : %d : %s\n", i, p1->port(), p1->type().toAscii().data());
	}
}

void SIMUPNP::unregisterPort(SIMUPNP::Port *port)
{
	ports_.remove(port);
}

SocksServer *SIMUPNP::server()
{
	return serv;
}

quint16 SIMUPNP::getPort(int protocol)
{
	while(!discoveryDone) {
		//usleep(100);
		printf("error.. - should not happend\n");
		return 0;
	}

	QString proto;
	if (protocol == QAbstractSocket::TcpSocket)
		proto = "TCP";
	else if (protocol == QAbstractSocket::UdpSocket)
		proto = "UDP";

	Device *dev = NULL;
	QList<Device*>::iterator it1;
	for( it1=devices.begin(); it1 != devices.end(); ++it1)
		dev = *it1;

//	if(dev) {
//		printf("dev->url()\n", dev->url());
//	}
	new Port(dev,proto);

	QList<Port*>::iterator it2;
	Port *p1 = NULL;
	for( it2=ports_.begin(); it2 != ports_.end(); ++it2) {
		p1 = *it2;
		if(!p1->inUse() && p1->mapped() && p1->type().compare(proto) == 0) {
			p1->setInUse(true);
			break;
		}
	}

	if(p1) {
		return p1->port();
	} else
		return 0;
}

void SIMUPNP::freePort(int protocol, quint16 port)
{
	QString proto;
	if (protocol == QAbstractSocket::TcpSocket)
		proto = "TCP";
	else if (protocol == QAbstractSocket::UdpSocket)
		proto = "UDP";

	QList<Port*>::iterator it;
	Port *p1 = NULL;
	for( it=ports_.begin(); it != ports_.end(); ++it) {
		p1 = *it;
		if(p1->mapped() && p1->port() == port && p1->type().compare(proto) == 0) {
			p1->unmap();
			break;
		}
	}
}

void SIMUPNP::setListenAddress(QHostAddress addr)
{
	listen_addr_ = addr;
}

QHostAddress SIMUPNP::listenAddress()
{
	return listen_addr_;
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

QString &SIMUPNP::externalIP()
{
	return externalIP_;
}

SIMUPNP *SIMUPNP::instance()
{
//	if ( !instance_ )
//		instance_ = new PsiOptions();
	return instance_;
}

quint16 SIMUPNP::randomPort()
{
	quint16 ret = (quint16)rand();
	
	for (int i = 0; i < ports_.count(); ++i)
	{
		if(ports_.takeAt(i)->port() == ret)
			return randomPort();
	}
	
	return ret;
}

SIMUPNP* SIMUPNP::instance_ = NULL;

#include "upnp.moc"