/*
 * jinglevoicecaller.cpp
 * Copyright (C) 2006  Remko Troncon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
 
#include <qstring.h>
#include <qdom.h>
#define POSIX
#include "talk/base/sigslot.h"
#include "talk/xmpp/constants.h"
#include "talk/xmpp/jid.h"
#include "talk/xmllite/xmlelement.h"
#include "talk/xmllite/xmlprinter.h"
#include "talk/base/network.h"
#include "talk/p2p/base/session.h"
#include "talk/p2p/base/sessionmanager.h"
#include "talk/base/helpers.h"
#include "talk/p2p/client/basicportallocator.h"
#include "talk/p2p/base/sessionclient.h"
#include "talk/p2p/client/sessionsendtask.h"
#include "talk/p2p/client/httpportallocator.h"
#include "talk/base/physicalsocketserver.h"
#include "talk/base/thread.h"
#include "talk/base/socketaddress.h"

#ifdef jingle_voice
#include "talk/session/phone/call.h"
#include "talk/session/phone/phonesessionclient.h"
#include "voicecaller.h"
#endif

#ifdef jingle_ft
#include "talk/session/fileshare/fileshare.h"
#include <QDir>
#endif

#include "im.h"
#include "xmpp.h"
#include "xmpp_xmlcommon.h"
#include "jingle.h"
#include "psiaccount.h"

// Should change in the future
#define JINGLE_NS "http://www.google.com/session"
#define JINGLEINFO_NS "google:jingleinfo"

#define JINGLE_PHONE_NS "http://www.google.com/session/phone"
#define JINGLE_SHARE_NS "http://www.google.com/session/share"

// ----------------------------------------------------------------------------

class JingleSessionManager;

class JingleInfoTask : public Task
{
public:
	JingleInfoTask(Task* parent, JingleSessionManager *session);
	
	void init();

	void onGo();
	bool take(const QDomElement& x);

	std::vector<talk_base::SocketAddress> stunList();
	std::vector<std::string> relayList();
	std::string relayToken();

signals:
	 void jingleInfoReady();

private:
	std::vector<talk_base::SocketAddress> stun_list_;
	std::vector<std::string> relay_list_;
	std::string relay_token_;
	
	JingleSessionManager* session_;

	bool parse_stun(const QDomElement &x);
	void parse_relay(const QDomElement &x);
};

JingleInfoTask::JingleInfoTask(Task* parent, JingleSessionManager* session) : Task(parent), session_(session) {}
	
void JingleInfoTask::init()
{
	std::vector<talk_base::SocketAddress> tmp_stun_list;
	tmp_stun_list.push_back(talk_base::SocketAddress("stun.l.google.com", 19302));
	tmp_stun_list.push_back(talk_base::SocketAddress("stun1.l.google.com", 19302));
	tmp_stun_list.push_back(talk_base::SocketAddress("stun2.l.google.com", 19302));
	tmp_stun_list.push_back(talk_base::SocketAddress("stun3.l.google.com", 19302));
	tmp_stun_list.push_back(talk_base::SocketAddress("stun4.l.google.com", 19302));
	stun_list_ = tmp_stun_list;
	session_->jingleInfoReady();
}

void JingleInfoTask::onGo() {
	QDomElement iq = createIQ(doc(), "get", client()->jid().full(), id());
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", JINGLEINFO_NS);
	iq.appendChild(query);
	send(iq);
}
	
bool JingleInfoTask::take(const QDomElement& x) {
	if(!iqVerify(x, "", id()))
		return false;

	if(x.attribute("type") == "result") {
		bool found;
		QDomElement query = findSubTag(x, "query", &found);
		QDomElement stun = findSubTag(query, "stun", &found);
		if(found)
			parse_stun(stun);
		QDomElement relay = findSubTag(query, "relay", &found);
		if(found)
			parse_relay(relay);
		
		session_->jingleInfoReady();
		setSuccess();
	}
	else {
		setError(x);
	}
	return true;
}

std::vector<talk_base::SocketAddress> JingleInfoTask::stunList()
{
	return stun_list_;
}

std::vector<std::string> JingleInfoTask::relayList()
{
	return relay_list_;
}

std::string JingleInfoTask::relayToken()
{
	return relay_token_;
}

bool JingleInfoTask::parse_stun(const QDomElement &x) {
	stun_list_.clear();
	for(QDomNode n = x.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement a = n.toElement();
		if(a.isNull())
			continue;
		if((a.tagName() == "server") && (a.attribute("udp") != "") && (a.attribute("host") != ""))
		{
			stun_list_.push_back(talk_base::SocketAddress(a.attribute("host").ascii(), a.attribute("udp").toInt()));
		}
	}
}
	
void JingleInfoTask::parse_relay(const QDomElement& x) {
	relay_list_.clear();
	for(QDomNode n = x.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement a = n.toElement();
		if(a.isNull())
			continue;
		if(a.tagName() == "token")
		{
			QString token;
			XMLHelper::readEntry(x,"token",&token);
			relay_token_ = token.ascii();
		}
		if((a.tagName() == "server") && (a.attribute("host") != ""))
		{
			relay_list_.push_back(a.attribute("host").ascii());
		}
	}
}

// ----------------------------------------------------------------------------

/**
 * \class JingleIQResponder
 * \brief A task that responds to jingle candidate queries with an empty reply.
 */
class JingleIQResponder : public XMPP::Task
{
public:
	JingleIQResponder(XMPP::Task *parent) : Task(parent) {};
	~JingleIQResponder() {};

	bool take(const QDomElement& e) {
		if(e.tagName() != "iq")
			return false;
		QDomElement first = e.firstChild().toElement();
		if (!first.isNull() && first.attribute("xmlns") == JINGLE_NS) {
			return true;
		}
		return false;
	}
};

// ----------------------------------------------------------------------------

//class JingleFileTransfer {
//public:
JingleFileTransfer::JingleFileTransfer(cricket::FileShareSession* s) : session_(s)
{
}

JingleFileTransfer::~JingleFileTransfer()
{
}
	
XMPP::Jid JingleFileTransfer::peer() const {
	return XMPP::Jid(session_->jid().BareJid().Str().c_str());
};
	
QString JingleFileTransfer::fileName() const {
	return description();
}
	
QString JingleFileTransfer::description() const {
	QString description;
	if (session_->manifest()->size() == 1)
		description = QString("'%1'").arg(session_->manifest()->item(0).name.c_str());
	else if (session_->manifest()->GetFileCount() && session_->manifest()->GetFolderCount())
		description = QString("'%1 files and %2 directories'").arg(session_->manifest()->GetFileCount()).arg(session_->manifest()->GetFolderCount());
	else if (session_->manifest()->GetFileCount())
		description = QString("'%1 files'").arg(session_->manifest()->GetFileCount());
	else if (session_->manifest()->GetFolderCount())
		description = QString("'%1 directories'").arg(session_->manifest()->GetFolderCount());
	return description;
}

qlonglong JingleFileTransfer::fileSize() const {
	size_t filesize;
	if (!session_->GetTotalSize(filesize))
		filesize = -1;
	return filesize;
}
	
void JingleFileTransfer::accept(qlonglong, qlonglong)
{
	session_->Accept();
}

void JingleFileTransfer::reject()
{
	session_->Decline();
}

void JingleFileTransfer::cancel()
{
	session_->Cancel();
}
	
// ----------------------------------------------------------------------------

/**
 * \brief A class for handling signals from libjingle.
 */
class JingleSessionSlots : public sigslot::has_slots<>
{
public:
	JingleSessionSlots(JingleSessionManager *manager);
	~JingleSessionSlots();
#ifdef jingle_ft
	void fileShareSessionCreated(cricket::FileShareSession *);
	void ft_stateChanged(cricket::FileShareState state);
	void progressChanged(cricket::FileShareSession* sess);
	void resampleImage(std::string, int, int, talk_base::HttpTransaction* trans);
#endif
#ifdef jingle_voice
    	void callCreated(cricket::Call *);
	void callDestroyed(cricket::Call *);
	void voice_stateChanged(cricket::Call *call, cricket::Session *session, cricket::Session::State state);
#endif
	void signalingReady();
	void sendStanza(const buzz::XmlElement *stanza);
private:
	JingleSessionManager* manager_;
	JingleFileTransfer* ft_session_;
	cricket::FileShareSession* session_;
};
 
JingleSessionSlots::JingleSessionSlots(JingleSessionManager *manager) : manager_(manager)
{
}

JingleSessionSlots::~JingleSessionSlots()
{
}

void JingleSessionSlots::sendStanza(const buzz::XmlElement *stanza)
{
	QString st(stanza->Str().c_str());
	st.replace("<sta:","<");
	st.replace("</sta:","<");
	st.replace("<cli:","<");
	st.replace("</cli:","</");
	st.replace("<:cli=","=");
	st.replace("<xmlns:sta","xmlns");
	manager_->sendStanza(st);
}

void JingleSessionSlots::signalingReady()
{
	manager_->session_manager_->OnSignalingReady();
}

#ifdef jingle_ft
void JingleSessionSlots::fileShareSessionCreated(cricket::FileShareSession *session)
{
	session_ = session;
	session_->SignalState.connect(this, &JingleSessionSlots::ft_stateChanged);
	session_->SignalNextFile.connect(this, &JingleSessionSlots::progressChanged);
	session_->SignalUpdateProgress.connect(this, &JingleSessionSlots::progressChanged);
	session_->SignalResampleImage.connect(this, &JingleSessionSlots::resampleImage);
// tmp
#ifdef Q_WS_MAC
	QDir home(QDir::homeDirPath() + "/Desktop");
#else
	QDir home = QDir::home();
#endif
	QDir dir(home.path() + "/googletalk_files");
	if(!dir.exists())
		home.mkdir("googletalk_files");
	session_->SetLocalFolder(dir.path().toStdString());
	ft_session_ = new JingleFileTransfer(session_);
}

void JingleSessionSlots::ft_stateChanged(cricket::FileShareState state)
{
	switch(state) {
		case cricket::FS_OFFER:
			emit manager_->incomingFileTransfer(ft_session_);
			break;
			
		case cricket::FS_TRANSFER:
			qDebug("Tranfer started");
			break;
			
		case cricket::FS_COMPLETE:
			qDebug("Tranfer complited");
			break;
			
		case cricket::FS_LOCAL_CANCEL:
		case cricket::FS_REMOTE_CANCEL:
			qDebug("FS_CANCEL");
			break;
		case cricket::FS_FAILURE:
			qDebug("FS_FAILURE");
			break;
	}
}

void JingleSessionSlots::progressChanged(cricket::FileShareSession* sess)
{
	size_t progress;
	std::string itemName;
	if (sess->GetProgress(progress) && sess->GetCurrentItemName(&itemName)) {
		emit ft_session_->progressChanged((qlonglong)progress,QString(itemName.c_str()));
	}
}

void JingleSessionSlots::resampleImage(std::string, int, int, talk_base::HttpTransaction* trans)
{
	session_->ResampleComplete(NULL, trans, false);
}

#endif

#ifdef jingle_voice
void JingleSessionSlots::callCreated(cricket::Call *call)
{
	qDebug("JingleClientSlots: Call created");
	call->SignalSessionState.connect(this, &JingleSessionSlots::voice_stateChanged);
}

void JingleSessionSlots::callDestroyed(cricket::Call *call)
{
	qDebug("JingleClientSlots: Call destroyed");
	XMPP::Jid jid(call->sessions()[0]->remote_name().c_str());
	if (manager_->calling(jid)) {
		qDebug(QString("Removing unterminated call to %1").arg(jid.full()));
		manager_->removeCall(jid);
		emit manager_->terminated(jid);
	}
}

void JingleSessionSlots::voice_stateChanged(cricket::Call *call, cricket::Session *session, cricket::Session::State state) 
{
	qDebug(QString("jinglevoicecaller.cpp: Jid (%1) State changed (%2)").arg(session->remote_name().c_str()).arg(state));
	// Why is c_str() stuff needed to make it compile on OS X ?
	XMPP::Jid *jid = new XMPP::Jid(session->remote_name().c_str());

	if (state == cricket::Session::STATE_INIT) { }
	else if (state == cricket::Session::STATE_SENTINITIATE) { 
		manager_->registerCall(*jid,call);
	}
	else if (state == cricket::Session::STATE_RECEIVEDINITIATE) {
		manager_->registerCall(*jid,call);
		emit manager_->incoming(*jid);
	}
	else if (state == cricket::Session::STATE_SENTACCEPT) { }
	else if (state == cricket::Session::STATE_RECEIVEDACCEPT) {
		emit manager_->accepted(*jid);
	}
	else if (state == cricket::Session::STATE_SENTMODIFY) { }
	else if (state == cricket::Session::STATE_RECEIVEDMODIFY) {
		qWarning(QString("jinglevoicecaller.cpp: RECEIVEDMODIFY not implemented yet (was from %1)").arg((*jid).full()));
	}
	else if (state == cricket::Session::STATE_SENTREJECT) { }
	else if (state == cricket::Session::STATE_RECEIVEDREJECT) {
		manager_->removeCall(*jid);
		emit manager_->rejected(*jid);
	}
	else if (state == cricket::Session::STATE_SENTREDIRECT) { }
	else if (state == cricket::Session::STATE_SENTTERMINATE) {
		manager_->removeCall(*jid);
		emit manager_->terminated(*jid);
	}
	else if (state == cricket::Session::STATE_RECEIVEDTERMINATE) {
		manager_->removeCall(*jid);
		emit manager_->terminated(*jid);
	}
	else if (state == cricket::Session::STATE_INPROGRESS) {
//		manager_->in_progress(jid);
	}
	delete jid;
}

#endif

//-------------------------------------------------------------------------------
#ifndef jingle_voice
JingleSessionManager::JingleSessionManager(PsiAccount *acc)
{
#else
JingleSessionManager::JingleSessionManager(PsiAccount *acc) : VoiceCaller(acc)
{
#endif
	initialized_ = false;
	client_ = acc->client();
	connect(client_, SIGNAL(rosterRequestFinished(bool, int, const QString &)),SLOT(initialize()));
	connect(client_, SIGNAL(disconnected()),SLOT(deinitialize()));
}

JingleSessionManager::~JingleSessionManager()
{
}

void JingleSessionManager::initialize()
{
	if (initialized_)
		return;
	
	QString jid = ((ClientStream&) client_->stream()).jid().full();
	if (jid.isEmpty()) {
		qWarning("jingle.cpp: Empty JID");
		return;
	}
	
	buzz::Jid j(jid.ascii());
	
	if (socket_server_ == NULL) {
		cricket::InitRandom(j.Str().c_str(),j.Str().size());
		socket_server_ = new talk_base::PhysicalSocketServer();
		thread_ = new talk_base::Thread();
		talk_base::ThreadManager::SetCurrent(thread_);
 		stun_addr_ = new talk_base::SocketAddress("209.85.137.126",19302);
		network_manager_ = new talk_base::NetworkManager();
		port_allocator_ = new cricket::BasicPortAllocator((talk_base::NetworkManager*)(network_manager_), (talk_base::SocketAddress*)(stun_addr_), /* relay server */ NULL);
	}
	
	sslots_ = new JingleSessionSlots(this);
	session_manager_ = new cricket::SessionManager(port_allocator_, thread_); // lub NULL zamiast thread_
	session_manager_->SignalOutgoingMessage.connect(sslots_, &JingleSessionSlots::sendStanza);
	session_manager_->SignalRequestSignaling.connect(sslots_, &JingleSessionSlots::signalingReady);
	session_manager_->OnSignalingReady();
	
#ifdef jingle_ft
	file_share_session_client_ = new cricket::FileShareSessionClient(session_manager_, j, "psi");
	file_share_session_client_->SignalFileShareSessionCreate.connect(sslots_, &JingleSessionSlots::fileShareSessionCreated);
	session_manager_->AddClient(JINGLE_SHARE_NS, file_share_session_client_);
#endif

#ifdef jingle_voice
	phone_client_ = new cricket::PhoneSessionClient(j, session_manager_);
	phone_client_->SignalCallCreate.connect(sslots_, &JingleSessionSlots::callCreated);
	phone_client_->SignalCallDestroy.connect(sslots_, &JingleSessionSlots::callDestroyed);
	session_manager_->AddClient(JINGLE_PHONE_NS, phone_client_);
#endif
	thread_->Start();

	jit = new JingleInfoTask(client_->rootTask(),this);
	//jit->init();
	jit->go(true);
	
	connect(client_,SIGNAL(xmlIncoming(const QString&)),SLOT(receiveStanza(const QString&)));

	new JingleIQResponder(client_->rootTask());
	
	initialized_ = true;
}

void JingleSessionManager::deinitialize()
{
	if (!initialized_)
		return;
	
	disconnect(client_,SIGNAL(xmlIncoming(const QString&)),this,SLOT(receiveStanza(const QString&)));

// 	delete socket_server_;
	delete sslots_;

	initialized_ = false;
}

void JingleSessionManager::sendStanza(const QString& stanza)
{
	client_->send(stanza);
}

void JingleSessionManager::receiveStanza(const QString& stanza)
{
	QDomDocument doc;
	doc.setContent(stanza);
	
	QDomNode n = doc.documentElement().firstChild();
	bool ok = false;
	while (!n.isNull() && !ok) {
		QDomElement e = n.toElement();
		if (!e.isNull() && e.attribute("xmlns") == JINGLE_NS) {
			ok = true;
		}
		n = n.nextSibling();
	}
	
	if(!ok)
		return;
		
	buzz::XmlElement *e = buzz::XmlElement::ForStr(stanza.ascii());
	
	session_manager_->OnIncomingMessage(e);
}

void JingleSessionManager::jingleInfoReady()
{
	printf("jingleInfoReady()..\n");
	//port_allocator_->SetStunHosts(jit->stunList());
	//port_allocator_->SetRelayHosts(jit->relayList());
	//port_allocator_->SetRelayToken(jit->relayToken());
}

#ifdef jingle_voice

bool JingleSessionManager::calling(const Jid& jid)
{
	return calls_.contains(jid.full());
}

void JingleSessionManager::call(const Jid& jid)
{
	qDebug(QString("jingle.cpp: Calling %1").arg(jid.full()));
	cricket::Call *c = ((cricket::PhoneSessionClient*)(phone_client_))->CreateCall();
	c->InitiateSession(buzz::Jid(jid.full().ascii()),NULL);
	phone_client_->SetFocus(c);
}

void JingleSessionManager::accept(const Jid& j)
{
	qDebug("jingle.cpp: Accepting call");
	cricket::Call* call = calls_[j.full()];
	if (call != NULL) {
		call->AcceptSession(call->sessions()[0]);
		phone_client_->SetFocus(call);
	}
}

void JingleSessionManager::reject(const Jid& j)
{
	qDebug("jingle.cpp: Rejecting call");
	cricket::Call* call = calls_[j.full()];
	if (call != NULL) {
		call->RejectSession(call->sessions()[0]);
		calls_.remove(j.full());
	}
}

void JingleSessionManager::terminate(const Jid& j)
{
	qDebug(QString("jingle.cpp: Terminating call to %1").arg(j.full()));
	cricket::Call* call = calls_[j.full()];
	if (call != NULL) {
		call->Terminate();
		calls_.remove(j.full());
	}
}

void JingleSessionManager::registerCall(const Jid& jid, cricket::Call* call)
{
	qDebug("jingle.cpp: Registering call\n");
	if (!calls_.contains(jid.full())) {
		calls_[jid.full()] = call;
	}
	else {
		qWarning("jingle.cpp: Auto-rejecting call because another call is currently open");
		call->RejectSession(call->sessions()[0]);
	}
}

void JingleSessionManager::removeCall(const Jid& j)
{
	qDebug(QString("Jingle: Removing call to %1").arg(j.full()));
	calls_.remove(j.full());
}


#endif

talk_base::Thread* JingleSessionManager::thread_ = NULL;
talk_base::PhysicalSocketServer* JingleSessionManager::socket_server_ = NULL;
talk_base::NetworkManager* JingleSessionManager::network_manager_ = NULL;
cricket::BasicPortAllocator* JingleSessionManager::port_allocator_ = NULL;
talk_base::SocketAddress* JingleSessionManager::stun_addr_ = NULL;
