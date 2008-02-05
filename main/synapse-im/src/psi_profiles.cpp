/*
 * profiles.cpp - deal with profiles
 * Copyright (C) 2001-2003  Justin Karneges
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

#include "profiles.h"
#include "common.h"
#include "applicationinfo.h"
#include <qdir.h>
#include <qfileinfo.h>
#include <qdom.h>

#include <qapplication.h>
//Added by qt3to4:
#include <QTextStream>
#include <QtCrypto>
#include <QList>

#include "eventdlg.h"
#include "chatdlg.h"
#include "pgputil.h"
#include "xmpp_xmlcommon.h"
#include "xmpp_status.h"
#include "fancylabel.h"
#include "advwidget.h"
#include "psioptions.h"
#include "varlist.h"
#include "atomicxmlfile.h"
#include "psitoolbar.h"
#include "optionstree.h"

using namespace XMPP;
using namespace XMLHelper;

#define PROXY_NONE       0
#define PROXY_HTTPS      1
#define PROXY_SOCKS4     2
#define PROXY_SOCKS5     3

template<typename T, typename F>
void migrateEntry(const QDomElement& element, const QString& entry, const QString& option, F f)
{
	bool found;
	findSubTag(element, entry, &found);
	if (found) {
		T value;
		f(element, entry, &value);
		PsiOptions::instance()->setOption(option, value);
	}
}

void migrateIntEntry(const QDomElement& element, const QString& entry, const QString& option)
{
	migrateEntry<int>(element, entry, option, readNumEntry);
}

void migrateBoolEntry(const QDomElement& element, const QString& entry, const QString& option)
{
	migrateEntry<bool>(element, entry, option, readBoolEntry);
}

void migrateSizeEntry(const QDomElement& element, const QString& entry, const QString& option)
{
	migrateEntry<QSize>(element, entry, option, readSizeEntry);
}

void migrateStringEntry(const QDomElement& element, const QString& entry, const QString& option)
{
	migrateEntry<QString>(element, entry, option, readEntry);
}

void migrateStringList(const QDomElement& element, const QString& entry, const QString& option)
{
	migrateEntry<QStringList>(element, entry, option, xmlToStringList);
}

void migrateColorEntry(const QDomElement& element, const QString &entry, const QString &option)
{
	migrateEntry<QColor>(element, entry, option, readColorEntry);
}

void migrateRectEntry(const QDomElement& element, const QString &entry, const QString &option)
{
	migrateEntry<QRect>(element, entry, option, readRectEntry);
}



UserAccount::UserAccount()
{
	reset();
}

void UserAccount::reset()
{
	name = "Default";
	opt_enabled = TRUE;
	opt_auto = FALSE;
	customAuth = FALSE;
	req_mutual_auth = FALSE;
	legacy_ssl_probe = TRUE;
	security_level = QCA::SL_None;
	ssl = SSL_Auto;
	jid = "";
	pass = "";
	opt_pass = FALSE;
	port = 5222;
	opt_host = FALSE;
	host = "";
	opt_automatic_resource = TRUE;
	resource = "Psi";
	priority = 5;
	opt_keepAlive = TRUE;
	allow_plain = XMPP::ClientStream::AllowPlainOverTLS;
	opt_compress = FALSE;
	opt_log = TRUE;
	opt_amp = FALSE;
	opt_newMail = FALSE;
	opt_login_as = FALSE;
	opt_login_status = "";
	opt_login_message = "";
	opt_reconn = FALSE;
	opt_connectAfterSleep = false;
	opt_ignoreSSLWarnings = false;
	opt_ignoreHostMismatch = false;

	keybind.clear();

	roster.clear();
}

UserAccount::~UserAccount()
{
}

void UserAccount::fromOptions(OptionsTree *o, QString base)
{
	optionsBase = base;
	
	reset();

	opt_enabled = o->getOption(base + ".enabled").toBool();
	opt_auto = o->getOption(base + ".auto").toBool();
	opt_keepAlive = o->getOption(base + ".keep-alive").toBool();
	opt_compress = o->getOption(base + ".compress").toBool();
	req_mutual_auth = o->getOption(base + ".require-mutual-auth").toBool();
	legacy_ssl_probe = o->getOption(base + ".legacy-ssl-probe").toBool();
	opt_automatic_resource = o->getOption(base + ".automatic-resource").toBool();
	opt_log = o->getOption(base + ".log").toBool();
	opt_amp = o->getOption(base + ".advanced-message-processing").toBool();
	opt_login_as = o->getOption(base + ".auto-status.enable").toBool();
	opt_login_status = o->getOption(base + ".auto-status.status").toString();
	opt_login_message = o->getOption(base + ".auto-status.message").toString();
	opt_newMail = o->getOption(base + ".new-mail-notification").toBool();
	opt_reconn = o->getOption(base + ".reconn").toBool();
	opt_ignoreSSLWarnings = o->getOption(base + ".ignore-SSL-warnings").toBool();
	
	// FIX-ME: See FS#771
	if (o->getChildOptionNames().contains(base + ".connect-after-sleep")) {
		opt_connectAfterSleep = o->getOption(base + ".connect-after-sleep").toBool();
	}
	else {
		o->setOption(base + ".connect-after-sleep", opt_connectAfterSleep);
	}
	
	name = o->getOption(base + ".name").toString();
	jid = o->getOption(base + ".jid").toString();

	customAuth = o->getOption(base + ".custom-auth.use").toBool();
	authid = o->getOption(base + ".custom-auth.authid").toString();
	realm = o->getOption(base + ".custom-auth.realm").toString();

	// read password (we must do this after reading the jid, to decode properly)
	QString tmp = o->getOption(base + ".password").toString();
	if(!tmp.isEmpty()) {
		opt_pass = TRUE;
		pass = decodePassword(tmp, jid);
	}
	
	opt_host = o->getOption(base + ".use-host").toBool();
	security_level = o->getOption(base + ".security-level").toInt();
	
	tmp = o->getOption(base + ".ssl").toString();
	if (tmp == "no") {
		ssl = SSL_No;
	} else if (tmp == "yes") {
		ssl = SSL_Yes;
	} else if (tmp == "auto") {
		ssl = SSL_Auto;
	} else if (tmp == "legacy") {
		ssl = SSL_Legacy;
	} else {
		ssl = SSL_Yes;
	}
	
	host = o->getOption(base + ".host").toString();
	port = o->getOption(base + ".port").toInt();
	
	resource = o->getOption(base + ".resource").toString();
	priority = o->getOption(base + ".priority").toInt();
	
	QString pgpSecretKeyID = o->getOption(base + ".pgp-secret-key-id").toString();
	if (!pgpSecretKeyID.isEmpty()) {
		QCA::KeyStoreEntry e = PGPUtil::instance().getSecretKeyStoreEntry(pgpSecretKeyID);
		if (!e.isNull())
		pgpSecretKey = e.pgpSecretKey();
	}
	
	tmp = o->getOption(base + ".allow-plain").toString();
	if (tmp == "never") {
		allow_plain = XMPP::ClientStream::NoAllowPlain;
	} else if (tmp == "always") {
		allow_plain = XMPP::ClientStream::AllowPlain;
	} else if (tmp == "over encryped") {
		allow_plain = XMPP::ClientStream::AllowPlainOverTLS;
	} else {
		allow_plain = XMPP::ClientStream::NoAllowPlain;		
	}

#ifdef LINKLOCAL
	if(name=="Link-Local")
		return;
#endif

	QStringList rosterCache = o->getChildOptionNames(base + ".roster-cache", true, true);
	foreach(QString rbase, rosterCache) {
		RosterItem ri;
		ri.setJid(Jid(o->getOption(rbase + ".jid").toString()));
		ri.setName(o->getOption(rbase + ".name").toString());
		Subscription s;
		s.fromString(o->getOption(rbase + ".subscription").toString());
		ri.setSubscription(s);
		ri.setAsk(o->getOption(rbase + ".ask").toString());
		ri.setGroups(o->getOption(rbase + ".groups").toStringList());
		roster += ri;
	}

	groupState.clear();
	QVariantList states = o->mapKeyList(base + ".group-state");
	foreach(QVariant k, states) {
		GroupData gd;
		QString sbase = o->mapLookup(base + ".group-state", k);
		gd.open = o->getOption(sbase + ".open").toBool();
		gd.rank = o->getOption(sbase + ".rank").toInt();
		groupState.insert(k.toString(), gd);
	}
	
	proxyID = o->getOption(base + ".proxy-id").toString();

	keybind.fromOptions(o, base + ".pgp-key-bindings");

	dtProxy = o->getOption(base + ".bytestreams-proxy").toString();
}

void UserAccount::toOptions(OptionsTree *o, QString base)
{
	if (base.isEmpty()) {
		base = optionsBase;
	}
	// clear old data away
	o->removeOption(base, true);
	
	o->setOption(base + ".enabled", opt_enabled);
	o->setOption(base + ".auto", opt_auto);
	o->setOption(base + ".keep-alive", opt_keepAlive);
	o->setOption(base + ".compress", opt_compress);
	o->setOption(base + ".require-mutual-auth", req_mutual_auth);
	o->setOption(base + ".legacy-ssl-probe", legacy_ssl_probe);
	o->setOption(base + ".automatic-resource", opt_automatic_resource);
	o->setOption(base + ".log", opt_log);
	o->setOption(base + ".advanced-message-processing", opt_amp);
	o->setOption(base + ".auto-status.enable", opt_login_as);
	o->setOption(base + ".auto-status.status", opt_login_status);
	o->setOption(base + ".auto-status.message", opt_login_message);
	o->setOption(base + ".new-mail-notification", opt_newMail);
	o->setOption(base + ".reconn", opt_reconn);
	o->setOption(base + ".connect-after-sleep", opt_connectAfterSleep);
	o->setOption(base + ".ignore-SSL-warnings", opt_ignoreSSLWarnings);

	o->setOption(base + ".name", name);
	o->setOption(base + ".jid", jid);

	o->setOption(base + ".custom-auth.use", customAuth);
	o->setOption(base + ".custom-auth.authid", authid);
	o->setOption(base + ".custom-auth.realm", realm);

	if(opt_pass) {
		o->setOption(base + ".password", encodePassword(pass, jid));
	} else {
		o->setOption(base + ".password", "");
	}
	o->setOption(base + ".use-host", opt_host);
	o->setOption(base + ".security-level", security_level);
	switch (ssl) {
		case SSL_No:
			o->setOption(base + ".ssl", "no");
			break;
		case SSL_Yes:
			o->setOption(base + ".ssl", "yes");
			break;
		case SSL_Auto:
			o->setOption(base + ".ssl", "auto");
			break;
		case SSL_Legacy:
			o->setOption(base + ".ssl", "legacy");
			break;
		default:
			qFatal("unknown ssl enum value in UserAccount::toOptions");
	}
	o->setOption(base + ".host", host);
	o->setOption(base + ".port", port);
	o->setOption(base + ".resource", resource);
	o->setOption(base + ".priority", priority);
	if (!pgpSecretKey.isNull()) {
		o->setOption(base + ".pgp-secret-key-id", pgpSecretKey.keyId());
	} else {
		o->setOption(base + ".pgp-secret-key-id", "");
	}
	switch (allow_plain) {
		case XMPP::ClientStream::NoAllowPlain:
			o->setOption(base + ".allow-plain", "never");
			break;
		case XMPP::ClientStream::AllowPlain:
			o->setOption(base + ".allow-plain", "always");
			break;
		case XMPP::ClientStream::AllowPlainOverTLS:
			o->setOption(base + ".allow-plain", "over encryped");
			break;
		default:
			qFatal("unknown allow_plain enum value in UserAccount::toOptions");
	}
	
	int idx = 0;
	foreach(RosterItem ri, roster) {
		QString rbase = base + ".roster-cache.a" + QString::number(idx++);
		o->setOption(rbase + ".jid", ri.jid().full());
		o->setOption(rbase + ".name", ri.name());
		o->setOption(rbase + ".subscription", ri.subscription().toString());
		o->setOption(rbase + ".ask", ri.ask());
		o->setOption(rbase + ".groups", ri.groups());
	}
	
	// now we check for redundant entries
	QStringList groupList;
	QSet<QString> removeList;
	groupList << "/\\/" + name + "\\/\\"; // account name is a very 'special' group

	// special groups that should also have their state remembered
	groupList << qApp->translate("ContactProfile", "General");
	groupList << qApp->translate("ContactProfile", "Agents/Transports");

	// first, add all groups' names to groupList
	foreach(RosterItem i, roster) {
		groupList += i.groups();
	}

	// now, check if there's groupState name entry in groupList
	foreach(QString group, groupState.keys()) {
		if (!groupList.contains(group)) {
			removeList << group;
		}
	}

	// remove redundant groups
	foreach(QString group, removeList) {
		groupState.remove( group );
	}

	// and finally, save the data
	foreach(QString group, groupState.keys()) {
		QString groupBase = o->mapPut(base + ".group-state", group);
		o->setOption(groupBase + ".open", groupState[group].open);
		o->setOption(groupBase + ".rank", groupState[group].rank);
	}
		
	o->setOption(base + ".proxy-id", proxyID);

	keybind.toOptions(o, base + ".pgp-key-bindings");
	o->setOption(base + ".bytestreams-proxy", dtProxy.full());
	
}

UserProfile::UserProfile()
	:OptionsTree()
{
	groupStates.clear();
	for(int i=0; i<5; i++) {
		lastStates[i].type = XMPP::Status::Offline;
		lastStates[i].status = "";
	}
	setParent(QCoreApplication::instance());

}

UserProfile::~UserProfile()
{
	if(!file.isEmpty()) {
		removeOption("user-profile", true);
		toOptions(this, "user-profile");
		saveOptions(file, "user-profile", ApplicationInfo::optionsNS(), ApplicationInfo::version());
	}
}

bool UserProfile::fromOptions(OptionsTree *o, QString base)
{
	groupStates.clear();
	QVariantList gstates = o->mapKeyList(base + ".groups");
	foreach(QVariant k, gstates) {
		QString sbase = o->mapLookup(base + ".groups", k);
		groupStates.insert(k.toString(), o->getOption(sbase + ".open").toBool());
	}

	QVariantList st = o->mapKeyList(base + ".last-states");
	int i = 0;
	foreach(QVariant k, st) {
		QString sbase = o->mapLookup(base + ".last-states", k);
		lastStates[i].status = o->getOption(sbase + ".message").toString();
		lastStates[i].type = (XMPP::Status::Type) o->getOption(sbase + ".type").toInt();
	}

	return true;
}

bool UserProfile::toOptions(OptionsTree *o, QString base)
{
	QMap<QString,bool>::iterator it = groupStates.begin();
	while(it != groupStates.end()) {
		QString groupBase = o->mapPut(base + ".groups", it.key());
		o->setOption(groupBase + ".open", it.value());
		it++;
	}
	for(int i=0; i<5; i++) {
		QString stBase = o->mapPut(base + ".last-states", i);
		o->setOption(stBase + ".message", lastStates[i].status);
		o->setOption(stBase + ".type", lastStates[i].type);
	}
	return true;
}

/*QMap<QString,bool> UserProfile::getGroupStates()
{
	return groupStates;
}*/

LastStatus *UserProfile::getLastStatus(int i)
{
	return &lastStates[i];
}

void UserProfile::setLastStatus(int status, QString msg)
{
	int i = 0;
	for(i = 4; i > 0; i--)
		lastStates[i] = lastStates[i-1];
	lastStates[i].type = (XMPP::Status::Type)status;
	lastStates[i].status = msg;

	removeOption("user-profile", true);
	toOptions(this, "user-profile");

	saveOptions(file, "user-profile", ApplicationInfo::optionsNS(), ApplicationInfo::version());
}

bool UserProfile::isGroupOpen(const QString &name)
{
	return groupStates[name];
}

void UserProfile::setGroupOpen(const QString &name, bool b)
{
	printf("%s : %d\n", name.toAscii().data(), b ? 1 : 0);
	groupStates[name] = b;
}

void UserProfile::load(QString filename)
{
	file = filename;
	loadOptions(file, "options", ApplicationInfo::optionsNS());
	fromOptions(this, "user-profile");
}

UserProfile* UserProfile::instance()
{
	if ( !instance_ )
		instance_ = new UserProfile();
	return instance_;
}

UserProfile* UserProfile::instance_ = NULL;

QString pathToProfile(const QString &name)
{
	return ApplicationInfo::profilesDir() + "/" + name;
}

QString pathToProfileConfig(const QString &name)
{
	return pathToProfile(name) + "/config.xml";
}

QStringList getProfilesList()
{
	QStringList list;

	QDir d(ApplicationInfo::profilesDir());
	if(!d.exists())
		return list;

	QStringList entries = d.entryList();
	for(QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
		if(*it == "." || *it == "..")
			continue;
		QFileInfo info(d, *it);
		if(!info.isDir())
			continue;

		list.append(*it);
	}

	list.sort();

	return list;
}

bool profileExists(const QString &_name)
{
	QString name = _name.lower();

	QStringList list = getProfilesList();
	for(QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
		if((*it).lower() == name)
			return TRUE;
	}
	return FALSE;
}

bool profileNew(const QString &name)
{
	if(name.isEmpty())
		return FALSE;

	// verify the string is sane
	for(int n = 0; n < (int)name.length(); ++n) {
		if(!name.at(n).isLetterOrNumber())
			return FALSE;
	}

	// make it
	QDir d(ApplicationInfo::profilesDir());
	if(!d.exists())
		return FALSE;
	QDir p(ApplicationInfo::profilesDir() + "/" + name);
	if(!p.exists()) {
	        if (!d.mkdir(name))
			return FALSE;
	}

	p.mkdir("history");
	p.mkdir("vcard");

	return TRUE;
}

bool profileRename(const QString &oldname, const QString &name)
{
	// verify the string is sane
	for(int n = 0; n < (int)name.length(); ++n) {
		if(!name.at(n).isLetterOrNumber())
			return FALSE;
	}

	// locate the folder
	QDir d(ApplicationInfo::profilesDir());
	if(!d.exists())
		return FALSE;
	if(!d.rename(oldname, name))
		return FALSE;

	return TRUE;
}

static bool folderRemove(const QDir &_d)
{
	QDir d = _d;

	QStringList entries = d.entryList();
	for(QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
		if(*it == "." || *it == "..")
			continue;
		QFileInfo info(d, *it);
		if(info.isDir()) {
			if(!folderRemove(QDir(info.filePath())))
				return FALSE;
		}
		else {
			//printf("deleting [%s]\n", info.filePath().latin1());
			d.remove(info.fileName());
		}
	}
	QString name = d.dirName();
	if(!d.cdUp())
		return FALSE;
	//printf("removing folder [%s]\n", d.filePath(name).latin1());
	d.rmdir(name);

	return TRUE;
}

bool profileDelete(const QString &path)
{
	QDir d(path);
	if(!d.exists())
		return TRUE;

	return folderRemove(QDir(path));
}

QString activeProfile;
