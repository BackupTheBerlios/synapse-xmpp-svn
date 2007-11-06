/*
	Synapse-IM - XMPP with Jingle Audio client
*/

#include "synapsetransmitter.h"

#include <QObject>
#include <QList>
#include <QUdpSocket>

#include "rtpipv4address.h"
#include "rtptimeutilities.h"
#include "rtprawpacket.h"
#include "rtpudpv4transmitter.h"

class PeerAddress {
public:
	PeerAddress(uint32_t _ip, quint16 _port) : port(_port), ip(_ip) {}
	QHostAddress ip;
	quint16 port;
};

class SynapseTransmitter::Private : public QObject
{
	Q_OBJECT
public:
	Private() {
		rtpPacketsCount = 0;
	}

	QUdpSocket sock;

public slots:
	void packetRecived()
	{
		incomingPackets.append(new QByteArray(sock.readAll()));
	}
	
public:
	QList<QByteArray*> incomingPackets;
	QList<PeerAddress*> peerHosts;
	int rtpPacketsCount;
};


SynapseTransmitter::SynapseTransmitter() : RTPTransmitter(0)
{
	d = new Private;
}

SynapseTransmitter::~SynapseTransmitter()
{
	delete d;
}

// The init function is there for initialization before any other threads
// may access the object (e.g. initialization of mutexes)
int SynapseTransmitter::Init(bool threadsafe)
{
	return 0;
}

int SynapseTransmitter::Create(size_t maxpacksize,const RTPTransmissionParams *transparams)
{
	QObject::connect(&d->sock, SIGNAL(readyRead()), d, SLOT(packetRecived()));
	d->sock.bind(QHostAddress::Any, ((RTPSynapseTransmissionParams*)transparams)->GetPortbase(), QUdpSocket::DontShareAddress);
	return 0;
}

void SynapseTransmitter::Destroy()
{
}

// The user MUST delete the returned instance when it is no longer needed
RTPTransmissionInfo * SynapseTransmitter::GetTransmissionInfo()
{
	return 0;
}

// If the buffersize ins't large enough, the transmitter must fill in the
// required length in 'bufferlength'
// If the size is ok, bufferlength is adjusted so that it indicates the
// amount of bytes in the buffer that are part of the hostname.
// The buffer is NOT null terminated!
int SynapseTransmitter::GetLocalHostName(u_int8_t *buffer,size_t *bufferlength)
{
	strncpy( (char*)buffer, "localhost", *bufferlength );
	if ( *bufferlength < strlen("localhost") )
		*bufferlength = strlen("localhost");
	return 0;
}

bool SynapseTransmitter::ComesFromThisTransmitter(const RTPAddress *addr)
{
	return false;
}

size_t SynapseTransmitter::GetHeaderOverhead()
{
	return 10;
}

int SynapseTransmitter::Poll()
{
	return 0;
}

int SynapseTransmitter::WaitForIncomingData(const RTPTime &delay, bool *dataavailable)
{
	return 0;
}

int SynapseTransmitter::AbortWait()
{
	return 0;
}

int SynapseTransmitter::SendRTPData(const void *data,size_t len)
{

//	if ( d->mediaChannel->network_interface() ) {
//		d->mediaChannel->network_interface()->SendPacket(data,len);
//	}
	//d->iface->SendPacket(data, len);
	for(QList<PeerAddress*>::iterator it = d->peerHosts.begin(); it != d->peerHosts.end(); ++it)
	{
		printf("sending to : %s:%d\n",(*it)->ip.toString().toAscii().data(), (int)(*it)->port);
		d->sock.writeDatagram((const char*)data, len, (*it)->ip, (*it)->port);
		d->sock.flush();
	}
	d->rtpPacketsCount++;
	return 0;
}

int SynapseTransmitter::SendRTCPData(const void *data,size_t len)
{
	return 0;
}

void SynapseTransmitter::ResetPacketCount()
{
	d->rtpPacketsCount = 0;
}

u_int32_t SynapseTransmitter::GetNumRTPPacketsSent()
{
	return d->rtpPacketsCount;
}

u_int32_t SynapseTransmitter::GetNumRTCPPacketsSent()
{
	return 0;
}

int SynapseTransmitter::AddDestination(const RTPAddress &addr)
{
	RTPIPv4Address *address = ((RTPIPv4Address*)&addr);
	d->peerHosts.append(new PeerAddress(address->GetIP(),address->GetPort()));
	return 0;
}

int SynapseTransmitter::DeleteDestination(const RTPAddress &addr)
{
	return 0;
}

void SynapseTransmitter::ClearDestinations() {}

bool SynapseTransmitter::SupportsMulticasting() { return false; }

int SynapseTransmitter::JoinMulticastGroup(const RTPAddress &addr) { return 0; }
int SynapseTransmitter::LeaveMulticastGroup(const RTPAddress &addr) { return 0; }
void SynapseTransmitter::LeaveAllMulticastGroups() {}

// Note: the list of addresses must be cleared when the receive mode is changed!
int SynapseTransmitter::SetReceiveMode(RTPTransmitter::ReceiveMode m) { return 0; }
int SynapseTransmitter::AddToIgnoreList(const RTPAddress &addr) { return 0; }
int SynapseTransmitter::DeleteFromIgnoreList(const RTPAddress &addr){ return 0; }
void SynapseTransmitter::ClearIgnoreList() {}
int SynapseTransmitter::AddToAcceptList(const RTPAddress &addr) { return 0; }
int SynapseTransmitter::DeleteFromAcceptList(const RTPAddress &addr) { return 0; }
void SynapseTransmitter::ClearAcceptList() {}
int SynapseTransmitter::SetMaximumPacketSize(size_t s) { return 0; }

bool SynapseTransmitter::NewDataAvailable()
{
	return !d->incomingPackets.isEmpty();
}

RTPRawPacket*  SynapseTransmitter::GetNextPacket()
{
	if ( !NewDataAvailable() )
		return 0;
    
	QByteArray *data = d->incomingPackets.takeFirst();
	d->incomingPackets.removeAll(data);

	int recvlen = data->size();
	u_int8_t *datacopy =  new u_int8_t[recvlen+1];
 	memcpy(datacopy, data->data() ,recvlen);

	//RTPIPv4Address *addr = new RTPIPv4Address();
	RTPTime curtime = RTPTime::CurrentTime();

	RTPRawPacket *packet = new  RTPRawPacket(datacopy,recvlen,0,curtime,true);

	delete data;

	return packet;
}

#include "synapsetransmitter.moc"