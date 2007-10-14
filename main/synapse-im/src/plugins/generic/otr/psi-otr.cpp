/*
 * psi-otr.cpp - off-the-record messaging plugin for psi
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


#include "psi-otr.h"

/**
 * The default constructor.
 */
PsiOtrPlugin::PsiOtrPlugin() {
}


/*
 * Return the name of the plugin. The name will be shown in a list of 
 * available plugins.
 */
QString PsiOtrPlugin::name() const {
	return "Off-the-Record Messaging";
}


/**
 * A short name of the plugin. 
 */
QString PsiOtrPlugin::shortName() const {
	return "psi-otr";
}


/**
 * Version of the plugin.
 */
QString PsiOtrPlugin::version() const {
	return "0.1";
}


/**
 * The method is called from the PluginManager after loading the plugin. 
 * The Method creates a OtrConnection object. The object loads its configuration
 * from own config files and from the Psi config file.
 */
void PsiOtrPlugin::init() {
	otrConnection = new OtrConnection();
	otrConnection->setPlugin(this);
	otrConnection->loadConfig();
}


/**
 * Process an incoming text-only message. The message is passed to the
 * OtrConnection.
 */
QString PsiOtrPlugin::incomingMessage( const QString& fromJid,
			      const QString& toJid, 
			      const QString& message) {
	
	const char* decrypted = otrConnection->decryptMessage(
				removeResourceFromJid(fromJid).toStdString().c_str(),
				removeResourceFromJid(toJid).toStdString().c_str(), 
				message.toStdString().c_str());
	if (decrypted != NULL && strlen(decrypted)>0) {
		//if message was encrypted then convert msg back from UTF8. 
		// because decryptMessage() removes the otr tag (special whitespace and tab collon)
		// we remove all whitespaces and tabs and can compare the messages in the following
		QString decMsg = QString(decrypted).replace("\x20","").replace("\x09","");
		QString origMsg = QString(message).replace("\x20","").replace("\x09","");
		if (decMsg.compare(origMsg) == 0) 
			return QString(decrypted);
		else 
			return QString::fromUtf8(decrypted);
	}

	return QString();
}	


/**
 * Process an incoming html message. The message is passed to the
 * OtrConnection. The decrypted message will be parsed as QDomElement.
 */
QDomElement PsiOtrPlugin::incomingMessage( const QString& fromJid,
			      const QString& toJid, 
			      const QDomElement& html) {

	QString htmlStr;
	QTextStream ts( &htmlStr );
	html.save(ts, 0);

	QString decMsg = incomingMessage(fromJid, toJid, htmlStr);
	QString newHtml(decMsg);
	newHtml = "<body>" + newHtml + "</body>";
	QDomDocument* doc = new QDomDocument();
	int errorLine, errorColumn;
	QString errorText;
	if (! doc->setContent(newHtml, true, &errorText, &errorLine, &errorColumn) ) {
		qWarning() << "Error during parsing xml: line:" << errorLine <<
			" column:" << errorColumn << " " << errorText << endl
			<< "--" << endl << newHtml << "\n--";
		return QDomElement();
	}
	return doc->documentElement().cloneNode(true).toElement();
}


/**
 * Encrypt a message from a user. It's necessary to remove the resource string
 * from the JIDs, because the keys and fingerprints are stored without resource. 
 */
QString PsiOtrPlugin::outgoingMessage(const QString& fromJid, 
					const QString& toJid, 
					const QString& message) {
	const char* encrypted = otrConnection->encryptMessage(
				removeResourceFromJid(fromJid).toStdString().c_str(),
				removeResourceFromJid(toJid).toStdString().c_str(), 
				QString(message.toUtf8()).toStdString().c_str());
	return QString(encrypted);
}


/**
 * Sends a message from the Account with the JID fromJid. The method is called from
 * the OtrConnection to send messages during key-exchange.
 */
void PsiOtrPlugin::sendMessage(const char* fromJid, const char* toJid, const char* message) {
	QString reply=QString("<message type=\"chat\" to=\"%1\"><body>%2</body></message>").
			arg(toJid).arg(message);
	emit sendStanza(fromJid, reply);
}
	

/**
 * Removes the resource from a given JID. 
 * Example:
 * removeResourceFromJid("user@jabber.org/Home")
 * returns "user@jabber.org"
 */
QString PsiOtrPlugin::removeResourceFromJid(const QString& aJid) {
	QString addr = aJid;
	int pos = aJid.indexOf("/");
	if (pos > -1) {
		addr.truncate(pos);
		return addr;
	}
	return aJid;
}


/**
 * Returns the configurations widget to the PluginManager.
 */
QWidget* PsiOtrPlugin::options() {
	configDialog = new ConfigDlg(otrConnection);
	connect(configDialog, SIGNAL(savePsiOption(QString, QVariant)),
		this, SLOT(savePsiOption(QString, QVariant)));

	
	configDialog->setOtrPolicy(getPsiOption(PSI_CONFIG_POLICY).toInt());
	return configDialog;
} 


/**
 * Gets the home-directory of Psi from the PluginManager.
 */
QString PsiOtrPlugin::homeDir() {
	QString dir;
	emit getHomeDir(dir);
	return dir;
}


/**
 * Save a option in the Psi configuration file.
 */
void PsiOtrPlugin::savePsiOption(QString option, QVariant value ) {
	emit setGlobalOption(option, value);
}


/**
 * Get a value from the Psi configuration file.
 */
QVariant PsiOtrPlugin::getPsiOption(const QString& option) {
	QVariant value;
	emit getGlobalOption(option, value);
	return value;
}


Q_EXPORT_PLUGIN2(psiOtrPlugin, PsiOtrPlugin)

