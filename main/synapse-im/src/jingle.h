/*
 * jingle.h
 * Copyright (C) 2007  Andrzej Wojcik
 * Copyright (C) 2006  Remko Troncon
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
 
#ifndef JINGLE_H
#define JINGLE_H

#include "talk/base/scoped_ptr.h"

#include <qmap.h>
//#include "talk/base/autodetectproxy.h"
//#include "talk/p2p/base/session.h"
//#include "talk/p2p/client/httpportallocator.h"
#ifdef jingle_voice
#include "voicecaller.h"
#endif
#ifdef jingle_ft
#include "filetransfer.h"
#endif
class PsiAccount;

namespace cricket {
	class HttpPortAllocator;
	class BasicPortAllocator;
	class SessionManager;
	class PhoneSessionClient;
	class Call;
#ifdef jingle_ft
	class FileShareSessionClient;
	class FileShareSession;
#endif
}

namespace talk_base {
	class NetworkManager;
	class Thread;
	class PhysicalSocketServer;
	class SocketAddress;
}

namespace XMPP {
	class Jid;
}
class JingleSessionSlots;
//class JingleCallSlots;
class JingleInfoTask;
using namespace XMPP;

class JingleFileTransfer : public QObject
{
	Q_OBJECT
	friend class JingleSessionSlots;
public:
	JingleFileTransfer(cricket::FileShareSession *);
	virtual ~JingleFileTransfer();
	
	virtual XMPP::Jid peer() const;
	virtual QString fileName() const;
	virtual qlonglong fileSize() const;
	virtual QString description() const;
	
	virtual void accept(qlonglong offset=0, qlonglong length=0);
	virtual void reject();
	virtual void cancel();

signals:
	void progressChanged(qlonglong,const QString&);
	
private:
	cricket::FileShareSession* session_;
};
#ifndef jingle_voice
class JingleSessionManager : public QObject
{
#else
class JingleSessionManager : public VoiceCaller
{
#endif
	Q_OBJECT

	friend class JingleSessionSlots;

public:
	JingleSessionManager(PsiAccount* account);
	~JingleSessionManager();
	
protected slots:	
	void initialize();
	void deinitialize();
public:
#ifdef jingle_voice
	virtual bool calling(const Jid&);

	virtual void call(const Jid&);
	virtual void accept(const Jid&);
	virtual void reject(const Jid&);
	virtual void terminate(const Jid&);
#endif
#ifdef jingle_ft
signals:
	void incomingFileTransfer(JingleFileTransfer*);
#endif
protected:
	void registerCall(const Jid&, cricket::Call*);
	void removeCall(const Jid&);

public slots:
	void sendStanza(const QString& stanza);
	void receiveStanza(const QString&);
	void jingleInfoReady();

private:
	bool initialized_;
	static talk_base::PhysicalSocketServer *socket_server_;
	static talk_base::Thread *thread_;
	static talk_base::NetworkManager *network_manager_;
	static cricket::BasicPortAllocator *port_allocator_;
//	static cricket::HttpPortAllocator *port_allocator_;
	static talk_base::SocketAddress *stun_addr_;
	cricket::SessionManager *session_manager_;
#ifdef jingle_voice
	cricket::PhoneSessionClient *phone_client_;
#endif
#ifdef jingle_ft
	cricket::FileShareSessionClient *file_share_session_client_;
#endif
	JingleInfoTask *jit;
	JingleSessionSlots *sslots_;
	QMap<QString,cricket::Call*> calls_;
	XMPP::Client* client_;
};

#include <QProgressDialog>

class JingleFileTransferProgressDialog : public QProgressDialog
{
	Q_OBJECT

public:
	JingleFileTransferProgressDialog(JingleFileTransfer* ft) : QProgressDialog(NULL,Qt::WDestructiveClose), ft_(ft) {
		connect(ft,SIGNAL(progressChanged(qlonglong,const QString&)),SLOT(update(qlonglong,const QString&)));
		connect(this,SIGNAL(canceled()),SLOT(cancel()));
		setLabelText("Initializing");
		setRange(0,(int) ft->fileSize());
	}
	
public slots:
	void cancel() {
		ft_->cancel();
		QProgressDialog::cancel();
	}

protected slots:
	void update(qlonglong progress, const QString& name) {
		setLabelText(QString(tr("Transfering %1")).arg(name));
		setValue(progress);
	}

private:
	JingleFileTransfer* ft_;
};

#endif
