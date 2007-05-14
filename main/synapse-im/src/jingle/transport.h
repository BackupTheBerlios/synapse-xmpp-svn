#include <QString>
#include <QObject>
#include <qdom.h>
#include "stun.h"
#include <sys/types.h>
#ifndef WIN32
    #include <unistd.h>
#endif
#include <fcntl.h>


#ifndef WIN32
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#else
	#include <winsock2.h>
#endif // WIN32
#ifndef TRANSPORT_H
#define TRANSPORT_H

class Transport : public QObject {
	Q_OBJECT
public:
	Transport();
	~Transport();

	bool getStunInfo();
	bool behindNat();

	QString firewallAddress();
	int firewallPort();

	QDomElement info(QDomElement &candidate);

signals:


private:
	int stunSrcPort;
	StunAddress4 stunSrvAddr;
	StunAddress4 stunSAddr;

	int firewallPort_;
	QString firewallAddr_;
};

#endif
