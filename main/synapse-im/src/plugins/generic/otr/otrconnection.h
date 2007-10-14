/*
 * OtrConnection.h - manages the otr-connection.
 *
 * Copyright (C) Timo Engel (timo-e@freenet.de), Berlin 2007.
 * This program was written as part of a diplom thesis advised by 
 * Prof. Dr. Ruediger Weis (PST Labor)
 * at the Technical University of Applied Sciences Berlin.
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


#ifndef OTRCONNECTION_H_
#define OTRCONNECTION_H_

#include <iostream>
#include <vector>
#include <string>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/privkey.h>

#ifdef __cplusplus
}
#endif

#include <QObject>
#include <QtGui>

#include "psi-otr.h"
#include "fprint.h"
#include "privkeytablemodel.h"

const QString PSI_CONFIG_POLICY = "plugins.psi-otr.otr-policy";

class PsiOtrPlugin;
class PrivKeyTableModel;

using namespace std;

class OtrConnection {

	public:
	// default constructor
	OtrConnection();

	// deconstructor
	~OtrConnection();

	// reads fingerprints from a file into the userstate
	void readFingerprints(char* file);

	// reads private keys from a file into the userstate
	void readPrivateKeys(char* file);

	// encrypt an outgoing message
	const char* encryptMessage(const char* from, const char* to, const char* message);

	// decrypt an incoming message
	const char* decryptMessage(const char* from, const char* to, const char* message);

	// sets the pointer to the plugin
	void setPlugin(PsiOtrPlugin* aPlugin);

	// returns a list of known fingerprints
	QList< Fprint > getFingerprints();

	// delete a fingerprint
	void deleteFingerprint(unsigned char* fp);

	// get private keys as model
	PrivKeyTableModel* getPrivateKeysModel();

	// get private keys as hash accountJid->key
	QHash<QString, QString>* getPrivateKeys();

	// delete the private key for a account.
	void deletePrivateKey(QString account);

	// load keys and fingerprints into the userstate
	void loadConfig();

	// set fingerprint verified/not verified
	void verifyFingerprint(unsigned char* fp, bool verified);



		
	/*** static otr callback wrapper-functions ***/
	static OtrlPolicy cb_policy(void *opdata, ConnContext *context);
	static void cb_create_privkey(void *opdata, const char *accountname, 
			const char *protocol);
	static int cb_is_logged_in(void *opdata, const char *accountname, 
			const char *protocol, const char *recipient);
	static void cb_inject_message(void *opdata, const char *accountname,
			const char *protocol, const char *recipient, const char *message);
	static void cb_notify(void *opdata, OtrlNotifyLevel level,
		    const char *accountname, const char *protocol,
		    const char *username, const char *title,
		    const char *primary, const char *secondary);
	static int cb_display_otr_message(void *opdata, const char *accountname,
			const char *protocol, const char *username, const char *msg);
	static void cb_update_context_list(void *opdata);
	static const char* cb_protocol_name(void *opdata, const char *protocol);
	static void cb_protocol_name_free(void *opdata, const char *protocol_name);
	static void cb_new_fingerprint(void *opdata, OtrlUserState us,
			const char *accountname, const char *protocol,
			const char *username, unsigned char fingerprint[20]);
	static void cb_write_fingerprints(void *opdata);
	static void cb_gone_secure(void *opdata, ConnContext *context);
	static void cb_gone_insecure(void *opdata, ConnContext *context);
	static void cb_still_secure(void *opdata, ConnContext *context, int is_reply);
	static void cb_log_message(void *opdata, const char *message);
	/******/
	

	/*** otr callback functions ***/
	OtrlPolicy policy(ConnContext *context);
	void create_privkey(const char *accountname, const char *protocol);
	int is_logged_in(const char *accountname, const char *protocol, const char *recipient);
	void inject_message(const char *accountname, const char *protocol, const char *recipient, const char *message);
	void notify(OtrlNotifyLevel level, const char *accountname, const char *protocol,
		    const char *username, const char *title,
		    const char *primary, const char *secondary);
	int display_otr_message(const char *accountname, const char *protocol, const char *username, const char *msg);
	void update_context_list();
	const char* protocol_name(const char *protocol);
	void protocol_name_free(const char *protocol_name);
	void new_fingerprint(OtrlUserState us, const char *accountname, const char *protocol,
			const char *username, unsigned char fingerprint[20]);
	void write_fingerprints();
	void gone_secure(ConnContext *context);
	void gone_insecure(ConnContext *context);
	void still_secure(ConnContext *context, int is_reply);
	void log_message(const char *message);
	/******/
	

	private:
	// the userstate contains keys and known fingerprints
	OtrlUserState userstate;
	
	// pointers to callback functions
	OtrlMessageAppOps ui_ops;	

	// Pointer to the plugin, to send messages through Psi.
	PsiOtrPlugin* plugin;

	// Name of the protocol used of various libotr functions.
	char* protocolString;

	// name of the file storing dsa-keys
	QString keysFile;

	// name of the file storing known fingerprints
	QString fingerprintFile;

	// find the secure session id (ssid) for a context	
	QString getSessionId(const char* thisJid, const char* remoteJid);

	// return the messageState of a context
	OtrlMessageState* getMessageState(const char* thisJid, const char* remoteJid);

	// returns the messageState in human-readable string.
	QString getMessageStateString(const char* thisJid, const char* remoteJid);
};

#endif /*OTRCONNECTION_H_*/
