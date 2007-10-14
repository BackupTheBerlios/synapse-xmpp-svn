/*
 * fprint.h - Fingerprint table entry. 
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

#ifndef  FINGERPRINT_INC
#define  FINGERPRINT_INC

#include<QObject>
#include<QtGui>

class QString;
class QStandardItem;

class Fprint {

	private:
	// fingerprint in a binary format
	unsigned char* fingerprint; 

	// JID of the own account
	QString account;

	// Owner of the fingerprint
	QString username;

	// the fingerprint in a human-readable format
	QString fingerprintHuman;

	// the level of trust
	QString trust;

	// the messageState of the context (i.e. plaintext, encrypted, finished)
	QString messageState;


	public:
	Fprint() {
	};

	~Fprint() {
	}
	
	/* getter */
	const unsigned char* getFingerprint() const { return fingerprint; }
	QString getAccount() const { return account; };
	QString getUsername() { return username; };
	QString getFingerprintHuman() const { return fingerprintHuman; }
	QString getTrust() { return trust; }
	QString getMessageState() { return messageState; }
	
	/* setter */
	void setFingerprint(unsigned char* aFingerprint) {
		fingerprint = aFingerprint; }

	void setAccount(QString aAccount) {
		account = aAccount; }

	void setUsername(QString aUsername) {
		username = aUsername; }

	void setFingerprintHuman(QString aFingerprintHuman) {
		fingerprintHuman = aFingerprintHuman; }

	void setTrust(QString aTrust) {
		trust = aTrust; }

	void setMessageState(QString aMessageState) {
		messageState = aMessageState; }

	// get all data in a row
	QList<QStandardItem* > row() const {
		QList<QStandardItem*> r;
		r.append(new QStandardItem(account));
		r.append(new QStandardItem(username));
		r.append(new QStandardItem(fingerprintHuman));
		r.append(new QStandardItem(trust));
		r.append(new QStandardItem(messageState));
		return r;
	}

};

#endif   /* ----- #ifndef FINGERPRINT_INC  ----- */

