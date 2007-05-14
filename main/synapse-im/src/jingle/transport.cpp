#include <QString>
#include "transport.h"
#include "udp.h"

Transport::Transport()
{
	std::string srv = "stun.l.google.com";
	stunParseServerName((char*)srv.data(), stunSrvAddr);
//	stunSrvAddr.port = 3489;
	//stunSrvAddr.addr = ntohl("192.245.12.229");
	stunSrvAddr.port = 19302;
//	stunSrvPort = 3489;
	stunSrcPort  = stunRandomPort();
}

Transport::~Transport()
{
}

bool Transport::getStunInfo()
{
	StunAddress4 mappedAddr;
	struct in_addr in;
	char* addr;

	bool verbose = false;
	bool hairpin = false;
	bool presPort = false;

	stunSAddr.port = stunSrcPort;

	printf("Nat type: %d\n", stunNatType(stunSrvAddr, verbose, &presPort, &hairpin, stunSrcPort, &stunSAddr));

	int fd1 = stunOpenSocket(stunSrvAddr, &mappedAddr, stunSrcPort);
	bool ok = (fd1 == -1 || fd1 == INVALID_SOCKET) ? false : true;
	if (ok) {
		closesocket(fd1);
		firewallPort_ = mappedAddr.port;
		// Convert ipv4 address to host byte ordering
		in.s_addr = ntohl (mappedAddr.addr);
		addr = inet_ntoa(in);
		firewallAddr_ = QString(addr);
		printf("STUN Firewall: [%s:%d]\n", firewallAddr_.ascii(), firewallPort_);
		return true;
	} else {
		printf("Opening a stun socket pair failed\n");
	}
  return false;
}

bool Transport::behindNat()
{
	return getStunInfo();
}

QString Transport::firewallAddress()
{
	return firewallAddr_;
}

int Transport::firewallPort()
{
	return firewallPort_;
}

QDomElement Transport::info(QDomElement &candidate)
{
	getStunInfo();
	candidate.setAttribute("component", "1");
	candidate.setAttribute("foundation", "1");
	candidate.setAttribute("generation", "0");
	candidate.setAttribute("ip", firewallAddr_);
	candidate.setAttribute("network", "0");
	candidate.setAttribute("port", firewallPort_);
	candidate.setAttribute("priority", "9909");
	candidate.setAttribute("protocol", "udp");
	candidate.setAttribute("pwd", "asd88fgpdd777uzjYhagZg");
	candidate.setAttribute("type", "srflx");
	candidate.setAttribute("ufrag", "8hhy");
	return candidate;
}

/*void Transport::Init(Jid local, Jid remote) {
//	state = Initial;
//	local_ = local;
//	remote_ = remote;
}*/

//void Transport::start(uint32_t ip, int port, int codec)
//{
//	emit startRTPTransfer(ip, port, codec);
//	active_ = true;
//}

//void Transport::stop()
//{
//	emit stopRTPTransfer();
//	active_ = false;
//}
