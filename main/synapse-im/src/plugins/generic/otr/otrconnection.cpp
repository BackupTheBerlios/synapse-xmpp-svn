/*
 * OtrConnection.cpp - manages the otr connection.
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


#include "otrconnection.h"

// the userstate contains keys and known fingerprints
OtrlUserState userstate;
	
// pointers to callback functions
OtrlMessageAppOps ui_ops;


/** 
 * constructor to initialize the libotr
 */
OtrConnection::OtrConnection() {
	protocolString = "prpl-jabber";
	OTRL_INIT;
	userstate = otrl_userstate_create();
	ui_ops.policy = (*OtrConnection::cb_policy);
	ui_ops.create_privkey = (*OtrConnection::cb_create_privkey);
	ui_ops.is_logged_in = (*OtrConnection::cb_is_logged_in);
	ui_ops.inject_message = (*OtrConnection::cb_inject_message);
	ui_ops.notify = (*OtrConnection::cb_notify);
	ui_ops.display_otr_message = (*OtrConnection::cb_display_otr_message);
	ui_ops.update_context_list = (*OtrConnection::cb_update_context_list);
	ui_ops.protocol_name = (*OtrConnection::cb_protocol_name);
	ui_ops.protocol_name_free = (*OtrConnection::cb_protocol_name_free);
	ui_ops.new_fingerprint = (*OtrConnection::cb_new_fingerprint);
	ui_ops.write_fingerprints = (*OtrConnection::cb_write_fingerprints);
	ui_ops.gone_secure = (*OtrConnection::cb_gone_secure);
	ui_ops.gone_insecure = (*OtrConnection::cb_gone_insecure);
	ui_ops.still_secure = (*OtrConnection::cb_still_secure);
	ui_ops.log_message = (*OtrConnection::cb_log_message);
}


/**
 * deconstructor
 */
OtrConnection::~OtrConnection() {
	otrl_userstate_free(userstate);
}


/**
 * Read private keys and fingerprints into the userstate.
 */
void OtrConnection::loadConfig() {
	keysFile = plugin->homeDir() + "/otr.keys";
	fingerprintFile = plugin->homeDir() + "/otr.fingerprints";

	otrl_privkey_read(userstate, keysFile.toStdString().c_str());
	otrl_privkey_read_fingerprints(userstate, fingerprintFile.toStdString().c_str(), NULL, NULL);
}


/**
 * Encrypt a message bevore sending. 
 */
const char* OtrConnection::encryptMessage(const char* from, const char* to, const char* message) {
	char* encMessage = NULL;
	gcry_error_t err;
	err = otrl_message_sending(
          userstate,
          &ui_ops,
          this,
          from,
          "prpl-jabber",
          to,
          message,
          NULL,
          &encMessage,
          NULL,
          NULL);
	if (encMessage != NULL) {
		message = encMessage;
	}
	return message;
}


/**
 * Decrypt a received message.
 */
const char* OtrConnection::decryptMessage(const char* from, const char* to, const char* cryptedMessage) {
	int ignore_message = 0;
	char *newMessage = NULL;

    	ignore_message = otrl_message_receiving(
    		userstate, 
	    	&ui_ops,
    		this,
       		to,
  		"prpl-jabber",
        	from,
        	cryptedMessage,
        	&newMessage,
        	NULL,
        	NULL,
        	NULL);
	if (ignore_message == 1) {
		// internal protocol message. show user what kind of message was received.
		QString msg;
		OtrlMessageType type = otrl_proto_message_type(cryptedMessage);
		if (type == OTRL_MSGTYPE_NOTOTR) msg = "no OTR Message";
		else if (type == OTRL_MSGTYPE_TAGGEDPLAINTEXT) msg= "OTR TaggedPlaintexMessage";
		else if (type == OTRL_MSGTYPE_QUERY) msg= "OTR QueryMessage";
		else if (type == OTRL_MSGTYPE_DH_COMMIT) msg= "OTR DH-Commit Message";
		else if (type == OTRL_MSGTYPE_DH_KEY) msg= "OTR DH-Key Message";
		else if (type == OTRL_MSGTYPE_REVEALSIG) msg= "OTR Reveal Signature Message";
		else if (type == OTRL_MSGTYPE_SIGNATURE) msg= "OTR Signature Message";
		else if (type == OTRL_MSGTYPE_V1_KEYEXCH) msg= "OTR Version 1 Key Exchange Message";
		else if (type == OTRL_MSGTYPE_DATA) msg ="OTR Data Message";
		else if (type == OTRL_MSGTYPE_ERROR) msg ="OTR Error Message";
		else if (type == OTRL_MSGTYPE_UNKNOWN) msg= "OTR Unknown Message";
		else msg= "Unknown Message Type";
		
		msg.insert(0, "Received ");
		QString state = "[" + getMessageStateString(to, from) + "]";
		msg.append(" " + state);	
		OtrlMessageState* stateId;
		stateId = getMessageState(to, from);
		if (stateId != NULL && *stateId == OTRL_MSGSTATE_ENCRYPTED) {
			msg.append("\nsessionId: " + getSessionId(to, from));
		}
		char* retMsg = (char*) malloc( msg.length() + 1 );
		strcpy(retMsg, msg.toStdString().c_str());
		return retMsg;
	}
	else if (ignore_message == 0) {
		if (newMessage != NULL) {
			// replace message
			return newMessage;
		}
		else {
			// no otr message
			char* retMsg = (char*) malloc(strlen(cryptedMessage)+1);
			strcpy(retMsg, cryptedMessage);
			return retMsg;
		}
	}
	return NULL;	
}


/**
 * sets the pointer to the plugin
 */
void OtrConnection::setPlugin(PsiOtrPlugin* aPlugin) {
	this->plugin = aPlugin;
}


/**
 * Find the secure session id (ssid) for a context.
 */
QString OtrConnection::getSessionId(const char* thisJid, const char* remoteJid) {
	ConnContext* context;
	context = otrl_context_find(userstate, remoteJid, thisJid, protocolString,
				false, NULL, NULL, NULL);
	if (context != NULL) {
		QString firstHalf;
		QString secondHalf;

		for (unsigned int i =0; i<context->sessionid_len/2; i++) {
			firstHalf.append(QString::number(context->sessionid[i], 16));
		}
		for (unsigned int i =context->sessionid_len/2; i<context->sessionid_len; i++) {
			secondHalf.append(QString::number(context->sessionid[i], 16));
		}
		if (context->sessionid_half == OTRL_SESSIONID_FIRST_HALF_BOLD) {
			return QString("<b>" + firstHalf + "</b>" + secondHalf);
		}
		else {
			return QString(firstHalf + " <b> " + secondHalf + "</b>");
		}
	}
	return QString();
}


/**
 * Return the OtrlMessageState for a context. 
 * i.e. plaintext, encrypted, finished
 */
OtrlMessageState* OtrConnection::getMessageState(const char* thisJid, const char* remoteJid) {
	ConnContext* context;
	context = otrl_context_find(userstate, remoteJid, thisJid, protocolString,
				false, NULL, NULL, NULL);
	if (context != NULL) {
		return &(context->msgstate);
	}
	return NULL;
}


/**
 * Returns the messageState of a context in a human-readable string.
 */
QString OtrConnection::getMessageStateString(const char* thisJid, const char* remoteJid) {
	OtrlMessageState* state = getMessageState(thisJid, remoteJid);
	if (state != NULL) {
		if (*state == OTRL_MSGSTATE_ENCRYPTED) {
			return "encrypted";
		}
		else if (*state == OTRL_MSGSTATE_FINISHED) {
			return "finished";
		}
		else if (*state == OTRL_MSGSTATE_PLAINTEXT) {
			return "plaintext";
		}
	}
	return "unknown";
}


/**
 * Returns a list of all known fingerprints.
 *
 */
QList< Fprint > OtrConnection::getFingerprints() {
	QList< Fprint > fpList;
	ConnContext* context;
	Fingerprint* fingerprint;
	char hash[45];
	for (context = userstate->context_root; context != NULL; context = context->next) {
		fingerprint = context->fingerprint_root.next;
		while(fingerprint) {
			Fprint fpData;
			// my account
			fpData.setAccount(context->accountname); 
			// buddy username
			fpData.setUsername(context->username);
			// fingerprint
	    		fpData.setFingerprint(fingerprint->fingerprint);
			otrl_privkey_hash_to_human(hash, fingerprint->fingerprint);
		    	fpData.setFingerprintHuman(hash);
			// trust level 
			fpData.setTrust(fingerprint->trust);
			// message state
			fpData.setMessageState(getMessageStateString(
				context->accountname,
				context->username));
			fpList.append(fpData);	
			fingerprint = fingerprint->next;
		}
	}
	return fpList;
}


/**
 * Returns a hash of private keys in the format accountJID -> fingerprint.
 */
QHash<QString, QString>* OtrConnection::getPrivateKeys() {
	QHash<QString, QString>* privKeyList = new QHash<QString, QString>();
	ConnContext* context;
	for (context = userstate->context_root; context != NULL; context = context->next) {
		char fingerprint_buf[45];
		char* fingerprint = otrl_privkey_fingerprint(userstate,
			fingerprint_buf, context->accountname, protocolString);
		if (fingerprint != NULL && strlen(fingerprint) > 1) {
			privKeyList->insert(QString(context->accountname),
					    QString(fingerprint));

		}
	}
	return privKeyList;
}


/**
 * Return the model for private keys with a this pointer
 */
PrivKeyTableModel* OtrConnection::getPrivateKeysModel() {
	return new PrivKeyTableModel(this);
}


/**
 * Delete the private key for a account.
 */
void OtrConnection::deletePrivateKey(QString account) {
	OtrlPrivKey* key = otrl_privkey_find(userstate, account.toStdString().c_str(),
		protocolString);
	if (key != NULL) {
		otrl_privkey_forget(key);
	}
	//TODO: write changed otr.keys
}


/**
 * Delete a known fingerprint.
 */
void OtrConnection::deleteFingerprint(unsigned char* fpHash) {
	qWarning() << "löschen";
	ConnContext* context;
	Fingerprint* fp;
//	char fpHuman[45];
//	otrl_privkey_hash_to_human(fpHuman, fpHash);
	for (context = userstate->context_root; context != NULL; context = context->next) {
		fp = otrl_context_find_fingerprint(context, 
						   fpHash, 
						   0, NULL);
		if (fp != NULL) {
			otrl_context_forget_fingerprint(fp, 1);
			break;
		}
	}
	write_fingerprints();
}


/**
 * Read private keys from a file into the userstate.
 */
void OtrConnection::readPrivateKeys(char* keyFile) {
	otrl_privkey_read(userstate, keyFile);
}


/**
 * Read fingerprints from the file into the userstate.
 */
void OtrConnection::readFingerprints(char* file) {
	otrl_privkey_read_fingerprints(userstate, file, NULL, NULL);
}


/**
 * set fingerprint verified/not verified
 */
void OtrConnection::verifyFingerprint(unsigned char* fp, bool verified) {
	ConnContext* context;
	Fingerprint* fingerprint;
	for (context = userstate->context_root; context != NULL; context = context->next) {
		fingerprint = otrl_context_find_fingerprint(context, 
						            fp, 
						            0, NULL);
		if (verified) {
			otrl_context_set_trust(fingerprint, "verified");
		}
		else {
			otrl_context_set_trust(fingerprint, "");
		}
	}
	write_fingerprints();
}







/***  implemented callback functions for libotr ***/

/* Return the OTR policy for the given context. */
OtrlPolicy OtrConnection::policy(ConnContext *context) {
	Q_UNUSED(context);
	OtrlPolicy pol;
	pol = OTRL_POLICY_NEVER; // default

	int policyValue = plugin->getPsiOption(PSI_CONFIG_POLICY).toInt();
	if (policyValue == 0 ) {
		pol = OTRL_POLICY_NEVER; // otr disabled
	}
	if (policyValue == 1 ) {
		pol = OTRL_POLICY_MANUAL; // otr enabled, session started manual
	}
	if (policyValue == 2 ) {
		pol = OTRL_POLICY_OPPORTUNISTIC; // automatically initiate private messaging
	}
	if (policyValue == 3 ) {
		pol = OTRL_POLICY_ALWAYS; // require private messaging 
	}
	return pol;
}
	

/* Create a private key for the given accountname/protocol if
 * desired. */
void OtrConnection::create_privkey(const char *accountname, const char *protocol) {
	char fingerprint[45];

	QLabel* l = new QLabel("\n   Please wait, while generating key for " + QString(accountname) + "   \n");
	l->show();
	QCoreApplication::processEvents();
	QCoreApplication::processEvents();

	otrl_privkey_generate(userstate, keysFile.toStdString().c_str(), accountname, protocol);
	QCoreApplication::processEvents();
	l->hide();
	QCoreApplication::processEvents();

	if (otrl_privkey_fingerprint(userstate, fingerprint, accountname, protocol) == NULL) {
		qWarning() << "Failed to generate private key\n";
		exit(1);
	}
	
	QMessageBox* d = new QMessageBox(QMessageBox::Information,
					 "psi-otr",
					 "The fingerprint for " + QString(accountname) + " is\n" + QString(fingerprint),
					 QMessageBox::Ok,
					 NULL,
					 Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	d->exec();
}		      
		            

/* Report whether you think the given user is online.  Return 1 if
 * you think he is, 0 if you think he isn't, -1 if you're not sure.
 * If you return 1, messages such as heartbeats or other
 * notifications may be sent to the user, which could result in "not
 * logged in" errors if you're wrong. 
 */
int OtrConnection::is_logged_in(const char *accountname, const char *protocol, const char *recipient) {
	Q_UNUSED(accountname); Q_UNUSED(protocol); Q_UNUSED(recipient);
	return -1;
}
		            
		            
/* Send the given IM to the given recipient from the given
 * accountname/protocol. */
void OtrConnection::inject_message(const char *accountname, const char *protocol, 
					const char *recipient, const char *message) {
	Q_UNUSED(protocol);
	plugin->sendMessage(accountname, recipient, message);
}
		            

/* Display a notification message for a particular accountname /
 * protocol / username conversation. */
void OtrConnection::notify(OtrlNotifyLevel level, const char *accountname, const char *protocol,
		    const char *username, const char *title,
		    const char *primary, const char *secondary) {
	Q_UNUSED(accountname);
	Q_UNUSED(protocol);
	Q_UNUSED(username);

	QMessageBox::Icon messageBoxIcon;
	if (level == OTRL_NOTIFY_ERROR ) {
		messageBoxIcon = QMessageBox::Critical;	
	}
	else if (level == OTRL_NOTIFY_WARNING) {
		messageBoxIcon = QMessageBox::Warning;
	}
	else {
		messageBoxIcon = QMessageBox::Information;
	}

	QMessageBox* d = new QMessageBox(messageBoxIcon,
					 "psi-otr: " + QString(title),
					 QString(primary) + "\n" + QString(secondary),
					 QMessageBox::Ok,
					 NULL,
					 Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	d->exec();
}
		    
	
/* Display an OTR control message for a particular accountname /
 * protocol / username conversation.  Return 0 if you are able to
 * successfully display it.  If you return non-0 (or if this
 * function is NULL), the control message will be displayed inline,
 * as a received message, or else by using the above notify()
 * callback. */
int OtrConnection::display_otr_message(const char *accountname, const char *protocol, const char *username, const char *msg) {
	Q_UNUSED(accountname);
	Q_UNUSED(protocol);
	Q_UNUSED(username);
	Q_UNUSED(msg);
	
	//QMessageBox* d = new QMessageBox(QMessageBox::Information,
	//				 "psi-otr",
	//				 "OTR control message for account: " + QString(accountname) + " protocol: " + QString(protocol) + " username: " + QString(username) +"\n" + QString(msg), 
	//				 QMessageBox::Yes | QMessageBox::No,
	//				 NULL,
	//				 Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	//d->exec();
	return -1;
}
		            
	
/* When the list of ConnContexts changes (including a change in
 * state), this is called so the UI can be updated. */
void OtrConnection::update_context_list() {
}
	
	
/* Return a newly-allocated string containing a human-friendly name
 * for the given protocol id */
const char* OtrConnection::protocol_name(const char *protocol) {
	Q_UNUSED(protocol);
	char* p = "prpl-jabber";
	return p;
}
	
	
/* Deallocate a string allocated by protocol_name */
void OtrConnection::protocol_name_free(const char *protocol_name) {
	Q_UNUSED(protocol_name);
}

	
/* A new fingerprint for the given user has been received. */
void OtrConnection::new_fingerprint(OtrlUserState us, const char *accountname, const char *protocol,
				const char *username, unsigned char fingerprint[20]) {
	Q_UNUSED(us);
	Q_UNUSED(protocol);
	char fpHuman[45];
	otrl_privkey_hash_to_human(fpHuman, fingerprint);
	QMessageBox* d = new QMessageBox(QMessageBox::Information,
					 "psi-otr",
					 QString(accountname) + " has received new fingerprint from " + QString(username) + ":\n" + QString(fpHuman),
					 QMessageBox::Ok,
					 NULL,
					 Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	d->exec();
}
				
	
/* The list of known fingerprints has changed.  Write them to disk. */
void OtrConnection::write_fingerprints() {
	otrl_privkey_write_fingerprints(userstate, fingerprintFile.toStdString().c_str());
}
	
	
/* A ConnContext has entered a secure state. */
void OtrConnection::gone_secure(ConnContext *context) {
	Q_UNUSED(context);
}
	
	
/* A ConnContext has left a secure state. */
void OtrConnection::gone_insecure(ConnContext *context) {
	Q_UNUSED(context);
}
	
	
/* We have completed an authentication, using the D-H keys we
 * already knew.  is_reply indicates whether we initiated the AKE. */
void OtrConnection::still_secure(ConnContext *context, int is_reply) {
	Q_UNUSED(context);
	Q_UNUSED(is_reply);
}
	
	
/* Log a message.  The passed message will end in "\n". */
void OtrConnection::log_message(const char *message) {
	Q_UNUSED(message);
}

/*** ***/






/*** static wrapper functions ***/
	
OtrlPolicy OtrConnection::cb_policy(void *opdata, ConnContext *context) {
	return static_cast<OtrConnection*>(opdata)->policy(context);
}
	
void OtrConnection::cb_create_privkey(void *opdata, const char *accountname,
		            const char *protocol) {
	static_cast<OtrConnection*>(opdata)->create_privkey(accountname, protocol);
}		      
		            	
int OtrConnection::cb_is_logged_in(void *opdata, const char *accountname,
		            const char *protocol, const char *recipient) {
	return static_cast<OtrConnection*>(opdata)->is_logged_in(accountname, protocol, recipient);
}

void OtrConnection::cb_inject_message(void *opdata, const char *accountname,
		            const char *protocol, const char *recipient, const char *message) {
	static_cast<OtrConnection*>(opdata)->inject_message(accountname, protocol, recipient, message);
}

void OtrConnection::cb_notify(void *opdata, OtrlNotifyLevel level,
		    const char *accountname, const char *protocol,
		    const char *username, const char *title,
		    const char *primary, const char *secondary) {
	static_cast<OtrConnection*>(opdata)->notify(level, accountname, protocol, username, title, primary, secondary);
}
	
int OtrConnection::cb_display_otr_message(void *opdata, const char *accountname,
		            const char *protocol, const char *username, const char *msg) {
	return static_cast<OtrConnection*>(opdata)->display_otr_message(accountname, protocol, username, msg);
}
		           
void OtrConnection::cb_update_context_list(void *opdata) {
	static_cast<OtrConnection*>(opdata)->update_context_list();
}
		
const char* OtrConnection::cb_protocol_name(void *opdata, const char *protocol) {
	return static_cast<OtrConnection*>(opdata)->protocol_name(protocol);
}
	
void OtrConnection::cb_protocol_name_free(void *opdata, const char *protocol_name) {
	static_cast<OtrConnection*>(opdata)->protocol_name(protocol_name);
}

void OtrConnection::cb_new_fingerprint(void *opdata, OtrlUserState us,
		        const char *accountname, const char *protocol,
				const char *username, unsigned char fingerprint[20]) {
	static_cast<OtrConnection*>(opdata)->new_fingerprint(us, accountname, protocol, username, fingerprint);
}
			
void OtrConnection::cb_write_fingerprints(void *opdata) {
	static_cast<OtrConnection*>(opdata)->write_fingerprints();
}
	
void OtrConnection::cb_gone_secure(void *opdata, ConnContext *context) {
	static_cast<OtrConnection*>(opdata)->gone_secure(context);
}

void OtrConnection::cb_gone_insecure(void *opdata, ConnContext *context) {
	static_cast<OtrConnection*>(opdata)->gone_insecure(context);
}
	
void OtrConnection::cb_still_secure(void *opdata, ConnContext *context, int is_reply) {
	static_cast<OtrConnection*>(opdata)->still_secure(context, is_reply);
}
	
void OtrConnection::cb_log_message(void *opdata, const char *message) {
	static_cast<OtrConnection*>(opdata)->log_message(message);
}
/*** ***/



