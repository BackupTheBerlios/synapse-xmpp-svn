#include "linklocal.h"
#include <QTimer>

static QByteArray randomArray(int size)
{
	QByteArray a(size,'\0');
	for(int n = 0; n < size; ++n)
		a[n] = (char)(256.0*rand()/(RAND_MAX+1.0));
	return a;
}

static QString genId()
{
	return QCA::Hash("sha1").hashToString(randomArray(128));
}

enum {
	Idle,
	Connecting,
	WaitVersion,
	WaitTLS,
	NeedParams,
	Dequeue,
	Active,
	Closing
};

LinkLocal::Stream::Stream()
{
	state = Idle;
	proto = new CoreProtocol();
	proto->reset();
}

LinkLocal::Stream::Stream(Jid jidLocal, BSocket *bs) : jidLocal_(jidLocal), bs_(bs), root(NULL) {
	state = Idle;
	inUse = true;
	proto = new CoreProtocol();
	proto->reset();
	proto->startClientIn(genId(),true);
	connect(bs_, SIGNAL(readyRead()), SLOT(bs_readyRead()));
}

LinkLocal::Stream::Stream(Jid jidLocal, QString addr, quint16 port) : jidLocal_(jidLocal), root(NULL) {
	state = Idle;
	bs_ = new BSocket;
	inUse = true;
	autoDelete();
	proto = new CoreProtocol();
	proto->reset();
	connect(bs_, SIGNAL(connected()), SLOT(connected()));
	connect(bs_, SIGNAL(error(int)), SLOT(error(int)));
	bs_->connectToHost(addr,port);
}

LinkLocal::Stream::~Stream() 
{
	delete bs_;
	delete proto;
	delete root;
}

void LinkLocal::Stream::connected() {
	QByteArray spare = bs_->read();
	proto->startClientOut(jidLocal_, false, false, false, false, true);
	connect(bs_, SIGNAL(readyRead()), SLOT(bs_readyRead()));
	processNext();
}

QDomDocument & LinkLocal::Stream::doc() const
{
	return proto->doc;
}

QString LinkLocal::Stream::baseNS() const
{
	return NS_CLIENT;
}

bool LinkLocal::Stream::old() const {
	return proto->old;
}

bool LinkLocal::Stream::compare(QHostAddress ra, quint16 rp) {
	return ((bs_->peerAddress() == ra) && (bs_->peerPort() == rp));
}

void LinkLocal::Stream::close() {
	if(state == Active) {
		state = Closing;
		proto->shutdown();
		processNext();
		emit cleanUp(this);
		deleteLater();
	}
	else if(state != Idle && state != Closing) {
//		reset();
	}
}

bool LinkLocal::Stream::stanzaAvailable() const
{
	return (!in.isEmpty());
}

Stanza LinkLocal::Stream::read()
{
	if(in.isEmpty())
		return Stanza();
	else {
		Stanza *sp = in.first();
		Stanza s = *sp;
		in.removeFirst();
		delete sp;
		return s;
	}
}

void LinkLocal::Stream::write(const Stanza &s)
{
	inUse = true;
	if(state == Active && !waiting.isEmpty()) {
		proto->sendStanza(s.element());
		processNext();
	} else {
		waiting.append(s);
	}
}

void LinkLocal::Stream::ready()
{
	while(!waiting.isEmpty()) {
		inUse = true;
		Stanza s = waiting.first();
		proto->sendStanza(s.element());
		processNext();
		waiting.removeFirst();
	}
}

void LinkLocal::Stream::autoDelete()
{
	if(!inUse) {
		close();
	} else {
		inUse = false;
		QTimer::singleShot(1000*60*5, this, SLOT(autoDelete()));
	}
}

void LinkLocal::Stream::error(int)
{
	while(!waiting.isEmpty()) {
		Stanza s = waiting.first();
		Jid recp = s.to();
		s.setTo(s.from());
		s.setFrom(recp);
		s.setType("error");
		s.setError(Stanza::Error(Stanza::Error::Cancel, Stanza::Error::ServiceUnavailable, "Cound not connect to recipient!"));
		in.append(new Stanza(s));
		waiting.removeFirst();
	}
	doReadyRead();
	emit cleanUp(this);
	deleteLater();
}

int LinkLocal::Stream::errorCondition() const
{
	return -1;
}

QString LinkLocal::Stream::errorText() const
{
	return QString();
}

QDomElement LinkLocal::Stream::errorAppSpec() const
{
	//return ;
}

void LinkLocal::Stream::doReadyRead()
{
	while(stanzaAvailable()) {
//		printf("stanzaAvailable\n");
		Stanza s = read();
		s.setTo(Jid(s.to().user()+"@local"));
		readyRead(s);
	}
}

void LinkLocal::Stream::processNext()
{
	while(1) {
//		printf("################# %s\n", (proto->c2c_local) ? "Client" : "Server" );
		bool ok = proto->processStep();
		if(!ok && proto->need!=CoreProtocol::NSASLMechs && state!=WaitTLS) {
//			printf("pased: %d\n", ok ? 1 : 0);
			int need = proto->need;
//			printf("need: %d\n", need);
			if(need == CoreProtocol::NSASLMechs) {
				QStringList list;
				proto->setSASLMechList(list);
				return;
			}

			if(!in.isEmpty()) {
// 				printf("in not empty\n");
				QTimer::singleShot(0, this, SLOT(doReadyRead()));
			}

		return;
		}

		int event = proto->event;
// 		printf("event: %d\n", event);
		switch(event) {
			case CoreProtocol::EError: {
// 				printf("Error! Code=%d\n", proto->errorCode);
//				handleError();
				return;
			}
			case CoreProtocol::ESend: {
				QByteArray a = proto->takeOutgoingData();
				QByteArray cs(a.size()+1,'\0');
				memcpy(cs.data(), a.data(), a.size());
// 				printf("Need Send: {%s}\n", cs.data());
				bs_->write(a);
				break;
			}
			case CoreProtocol::ERecvOpen: {
// 				printf("Break (RecvOpen)\n");
				if(proto->old) {
					state = WaitVersion;
// 					printf("WarnOldVersion\n");
					return;
				}
				break;
			}
			case CoreProtocol::EFeatures: {
// 				printf("Break (Features)\n");
				if(state == WaitTLS)
					state = Active;
				else
					state = WaitTLS;
				break;
			}
			case CoreProtocol::ESASLSuccess: {
// 				printf("Break SASL Success\n");
				break;
			}
			case CoreProtocol::EReady: {
// 				printf("Done!\n");
				state = Active;
				ready();
				break;
			}
			case CoreProtocol::EPeerClosed: {
//				printf("DocumentClosed\n");
//				reset();
//				connectionClosed();
				return;
			}
			case CoreProtocol::EStanzaReady: {
//				printf("StanzaReady\n");
				// store the stanza for now, announce after processing all events
				Stanza s = createStanza(proto->recvStanza());
				if(s.isNull())
					break;
				in.append(new Stanza(s));
				break;
			}
			case CoreProtocol::EStanzaSent: {
//				printf("StanzasSent\n");
//				stanzaWritten();
//				if(!self)
//					return;
				break;
			}
			case CoreProtocol::EClosed: {
//				printf("Closed\n");
//				reset();
//				delayedCloseFinished();
				return;
			}
		}
	}
}

void LinkLocal::Stream::bs_readyRead() {
	QByteArray ba = bs_->read(1024);
//	printf("incoming: %s\n", QString(ba).ascii());
	proto->addIncomingData(ba);
	processNext();
}
