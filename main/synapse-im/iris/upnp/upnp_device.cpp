#include "upnp_device.h"
#include "upnp_port.h"
#include "upnp.h"

#include <QHttp>
#include <QDataStream>
#include <QString>
#include <QTimer>
#include <QTcpSocket>

SIMUPNP::Device::Device(SIMUPNP *upnp) : upnp_(upnp)
{
	sock_ = new QTcpSocket();
}
SIMUPNP::Device::~Device() {};

void SIMUPNP::Device::get() {
	QString get = QString("GET %1 HTTP/1.1\r\n").arg(url_);
	get += QString("Host: %1:%2\r\n").arg(hostname_).arg(port_);
	get += QString("User-Agent: Synapse-IM\r\n");
	get += QString("Connection: close\r\n\r\n");
	sock_->disconnect();
	connect(sock_, SIGNAL(disconnected()), this, SLOT(on_upnp_xml()));
	serviceType_ = "";
	sock_->connectToHost(hostname_, port_);

	sock_->waitForConnected(10000);
	upnp_->setListenAddress(sock_->localAddress());
	sock_->write(get.toUtf8());
}

void SIMUPNP::Device::post(QString &soap, QString &soapAction) {
	QString header;
	header += QString("POST %1").arg(controlUrl_) + QString(" HTTP/1.1\r\n");
	header += QString("Host: %1:%2\r\n").arg(hostname_).arg(QString("%1").arg(port_));
	header += QString("User-Agent: Synapse-IM\r\n");
	header += QString("Content-Type: text/xml; charset=\"utf-8\"\r\n");
	header += QString("Content-Length: %1\r\n").arg(soap.size());
	header += QString("Connection: close\r\n");
	header += QString("Soapaction: \"%1#%2\"\r\n\r\n").arg(serviceType_).arg(soapAction);
	header += soap;

	sock_->waitForConnected(10000);
	sock_->write(header.toUtf8());
	sock_->flush();
}

void SIMUPNP::Device::getExternalIP() {
	connect(sock_, SIGNAL(disconnected()), this, SLOT(on_upnp_get_external_ip_response()));
	if (externIPtry_ == 0) {
		sock_->connectToHost(hostname_, port_);
		QString soap_action = "GetExternalIPAddress";
		QString soap;

		soap += QString( "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">");
		soap += QString("<s:Body>");
		soap += QString("<u:%1 xmlns:u=\"%2\">").arg(soap_action).arg(serviceType_);
		soap += QString("</u:%1></s:Body></s:Envelope>").arg(soap_action);
		post(soap,soap_action);

	} else if (externIPtry_ == 1) {
		sock_->connectToHost("checkip.dyndns.org", 80);

		sock_->waitForConnected(10000);
		QString header("GET / HTTP/1.1\r\n");
		header += QString("Host: checkip.dyndns.org\r\n");
		header += QString("Connection: close\r\n\r\n");
		sock_->write(header.toUtf8());
	}
}

void SIMUPNP::Device::on_upnp_xml()
{
	QString doc(sock_->readAll());
	//delete sock;
	sock_->disconnect();

	if (doc.left(15).compare("HTTP/1.1 200 OK") != 0) {
		qDebug("UPNP::Device::on_upnp_xml() : Bad response\n");
		return;
	}

	int i = doc.indexOf( UPNP_WANIP );
	if(i!=-1) {
		serviceType_ = UPNP_WANIP;
	} else {
		qWarning("UPNP::Device::on_upnp_xml() : No WANIP service type.\n");
		i = doc.indexOf( UPNP_WANIP );
			if (i!=-1) {
			serviceType_ = UPNP_WANPPP;
		} else {
			qWarning("UPNP::Device::on_upnp_xml() : No WANPPP service type.\n");
			return;
		}
	}
	int j = doc.indexOf("<controlURL>", i) + sizeof("<controlURL>");
	i = doc.indexOf("</controlURL>",j);
	controlUrl_ = doc.mid(j-1, i - j +1);

	QTimer::singleShot(100, this, SLOT(getExternalIP()));
}

void SIMUPNP::Device::on_upnp_get_external_ip_response()
{
	QString doc(sock_->readAll());
	sock_->disconnect();

	QString externIP;
	externIPtry_++;

	if (doc.left(15).compare("HTTP/1.1 200 OK") != 0) {
		qWarning("UPNP::Device::on_upnp_get_external_ip_response() : Bad response\n");
		if ( externIPtry_ > 2 ) {
			//upnp_->server()->listen(upnp_->externalPort(),true);
			return;
		}
	} else {

		if (externIPtry_ == 1) {
			int j = doc.indexOf("<NewExternalIPAddress>") + sizeof("<NewExternalIPAddress>");
			int i = doc.indexOf("</NewExternalIPAddress>",j-2);
			externIP = doc.mid(j-1, i - j +1);
		} else if (externIPtry_ == 2) {
			int j = doc.indexOf("Address: ") + sizeof("Address: ");
			int i = doc.indexOf("</body>",j);
			externIP = doc.mid(j-1, i - j +1);
		}
	}

	if (externIP.isEmpty()) {
		if (externIPtry_ == 2)
			//upnp_->server()->listen(upnp_->externalPort(),true);
			return;
		else {
			getExternalIP();
		}
	} else {
		upnp_->setExternalIP(externIP);
	}
}

const QString &SIMUPNP::Device::url()
{
	return url_;
}

void SIMUPNP::Device::setUrl(const QString &_url)
{
	url_ = _url;
}

const QString &SIMUPNP::Device::controlUrl()
{
	return controlUrl_;
}

int SIMUPNP::Device::port()
{
	return port_;
}

void SIMUPNP::Device::setPort(int _port)
{
	port_ = _port;
}

const QString SIMUPNP::Device::hostname()
{
	return hostname_;
}

void SIMUPNP::Device::setHostname(const QString &_host)
{
	hostname_ = _host;
}

const QString SIMUPNP::Device::serviceType()
{
	return serviceType_;
}

bool SIMUPNP::Device::isReady()

{
	return !controlUrl_.isEmpty();
}

#include "upnp_device.moc"
