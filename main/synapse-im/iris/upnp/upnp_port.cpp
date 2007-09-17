#include "upnp_device.h"
#include "upnp_port.h"
#include "upnp.h"

#include <QHttp>
#include <QDataStream>
#include <QString>
#include <QTimer>
#include <QTcpSocket>

SIMUPNP::Port::Port(Device *_dev, const QString &_type)
{
	mapped_ = (_dev == NULL);
	type_ = _type;
	sock_ = new QTcpSocket();
	inUse_ = false;
	dev = _dev;
	port_ = SIMUPNP::instance()->randomPort();
	if(!mapped_)
		map();
	else
		SIMUPNP::instance()->registerPort(this);
}

SIMUPNP::Port::~Port()
{
	delete sock_;
}

void SIMUPNP::Port::post(QString &soap, QString &soapAction)
{
	QString header;
	header += QString("POST %1").arg(dev->controlUrl()) + QString(" HTTP/1.1\r\n");
	header += QString("Host: %1:%2\r\n").arg(dev->hostname()).arg(QString("%1").arg(dev->port()));
	header += QString("User-Agent: Synapse-IM\r\n");
	header += QString("Content-Type: text/xml; charset=\"utf-8\"\r\n");
	header += QString("Content-Length: %1\r\n").arg(soap.size());
	header += QString("Connection: close\r\n");
	header += QString("Soapaction: \"%1#%2\"\r\n\r\n").arg(dev->serviceType()).arg(soapAction);
	header += soap;

	sock_->waitForConnected(10000);
	sock_->write(header.toUtf8());
}

void SIMUPNP::Port::map()
{
	if(mapped_)
		return;

	while(!dev->isReady()) {
		QTimer::singleShot(100, this, SLOT(map()));
		return;
	}

	sock_->connectToHost(dev->hostname(), dev->port());
	connect(sock_, SIGNAL(disconnected()), this, SLOT(on_map_response()));
	QString soap_action = "AddPortMapping";
	QString soap;
	soap += QString( "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">");
	soap += QString("<s:Body>");
	soap += QString("<u:%1 xmlns:u=\"%2\">").arg(soap_action).arg(dev->serviceType());

	soap += QString("<NewRemoteHost></NewRemoteHost>");
	soap += QString("<NewExternalPort>%1</NewExternalPort>").arg(port_);
	soap += QString("<NewProtocol>%1</NewProtocol>").arg(type_);
	soap += QString("<NewInternalPort>%1</NewInternalPort>").arg(port_);
	soap += QString("<NewInternalClient>%1</NewInternalClient>").arg(SIMUPNP::instance()->listenAddress().toString());
	soap += QString("<NewEnabled>1</NewEnabled>");
	soap += QString("<NewPortMappingDescription>%1</NewPortMappingDescription>").arg(SIMUPNP::instance()->userAgent());
	soap += QString("<NewLeaseDuration>%1</NewLeaseDuration>").arg(SIMUPNP::instance()->leaseDuration());
	soap += QString("</u:%1></s:Body></s:Envelope>\r\n").arg(soap_action);

	post(soap,soap_action);
}

void SIMUPNP::Port::unmap()
{
	if(!mapped_)
		return;

	sock_->connectToHost(dev->hostname(), dev->port());
	connect(sock_, SIGNAL(disconnected()), this, SLOT(on_upnp_map_response()));
	QString soap_action = "DeletePortMapping";

	QString soap;
	soap += QString("<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">");
	soap += QString("<s:Body>");
	soap += QString("<u:%1 xmlns:u=\"%2\">").arg(soap_action).arg(dev->serviceType());

	soap += QString("<NewRemoteHost></NewRemoteHost>");
	soap += QString("<NewExternalPort>%1</NewExternalPort>").arg(port_);
	soap += QString("<NewProtocol>%1</NewProtocol>").arg(type_);
	soap += QString("</u:%1></s:Body></s:Envelope>\r\n").arg(soap_action);

	post(soap, soap_action);
}

void SIMUPNP::Port::on_map_response()
{
	QString doc(sock_->readAll());
	sock_->disconnect();

	if (doc.left(15).compare("HTTP/1.1 200 OK") != 0) {
		qDebug("UPNP::Port::on_map_resonse() : Bad response\n");
//		upnp_->server()->listen(upnp_->externalPort(),true);
		deleteLater();
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
			qWarning("UPNP::Port::on_map_resonse() : Only permanent leases supported\n");
			return;
		} else if (errorCode == 718) {
			// conflict in mapping, try next external port
			qWarning("UPNP::Device::on_upnp_map_resonse() : Port already used\n");
			port_++;
// 			upnp_->setLocalPort(upnp_->localPort() + 1);
			map();
			return;
		} else if (errorCode == 0) {
			qDebug("UPNP::Port::on_map_resonse() : unknown error\n");
			deleteLater();
			return;
		}
	}

	mapped_ = !mapped_;
	if(mapped_)
		SIMUPNP::instance()->registerPort(this);
	else
		SIMUPNP::instance()->unregisterPort(this);
}

bool SIMUPNP::Port::inUse()
{
	return inUse_;
}

void SIMUPNP::Port::setInUse(bool in)
{
	inUse_ = in;
}

quint16 SIMUPNP::Port::port()
{
	return port_;
}

QString SIMUPNP::Port::type()
{
	return type_;
}

bool SIMUPNP::Port::mapped()
{
	return mapped_;
}

#include "upnp_port.moc"