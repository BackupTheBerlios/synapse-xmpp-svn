/*
 * serverinfomanager.cpp
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

#include "serverinfomanager.h"
#include "xmpp_tasks.h"

using namespace XMPP;

ServerInfoManager::ServerInfoManager(Client* client) : client_(client)
{
	amp_ = NULL;
	deinitialize();
	connect(client_, SIGNAL(rosterRequestFinished(bool, int, const QString &)), SLOT(initialize()));
	connect(client_, SIGNAL(disconnected()), SLOT(deinitialize()));
}

void ServerInfoManager::reset()
{
	hasPEP_ = false;
	if(amp_ != NULL)
		delete amp_;
	amp_ = NULL;
	hasGoogleMailNotify_ = false;
	hasGoogleArchive_ = false;
	hasMessageArchiving_ = false;
	multicastService_ = QString();
}

void ServerInfoManager::initialize()
{
	JT_DiscoInfo *jt = new JT_DiscoInfo(client_->rootTask());
	connect(jt, SIGNAL(finished()), SLOT(disco_finished()));
	jt->get(client_->jid().domain());
	jt->go(true);
}

void ServerInfoManager::deinitialize()
{
	reset();
	emit featuresChanged();
}

const QString& ServerInfoManager::multicastService() const
{
	return multicastService_;
}

bool ServerInfoManager::hasGoogleMailNotify() const
{
	return hasGoogleMailNotify_;
}

bool ServerInfoManager::hasPEP() const
{
	return hasPEP_;
}

bool ServerInfoManager::hasAMP(QString f) const
{
	bool t = false;
	if (amp_ != NULL)
	{
	    if (f.isEmpty())
		    t = true;
	    else
		    t = amp_->test(f);
	}
	return t;
}

bool ServerInfoManager::hasGoogleArchive() const
{
	return hasGoogleArchive_;
}

#ifdef XEP-0136
bool ServerInfoManager::hasMessageArchiving() const
{
	return hasMessageArchiving_;
}
#endif

void ServerInfoManager::disco_finished()
{
	JT_DiscoInfo *jt = (JT_DiscoInfo *)sender();
	if (jt->success()) {
		// Features
		Features f = jt->item().features();
		if (f.canMulticast())
			multicastService_ = client_->jid().domain();
		// TODO: Remove this, this is legacy
		if (f.test(QStringList("http://jabber.org/protocol/pubsub#pep")))
			hasPEP_ = true;
			
		if (f.test(QStringList("google:mail:notify")))
			hasGoogleMailNotify_ = true;

		if (f.test(QStringList("http://jabber.org/protocol/archive#save")) || f.test(QStringList("http://jabber.org/protocol/archive#otr")))
			hasGoogleArchive_ = true;

#ifdef XEP-0136
		if(f.test(QStringList("http://www.xmpp.org/extensions/xep-0136.html#ns-auto")) || f.test(QStringList("http://www.xmpp.org/extensions/xep-0136.html#ns-manage")) || f.test(QStringList("http://www.xmpp.org/extensions/xep-0136.html#ns-pref")) || f.test(QStringList("http://www.xmpp.org/extensions/xep-0136.html#ns-manual")))
			hasMessageArchiving_ = true;
#endif

		// Identities
		DiscoItem::Identities is = jt->item().identities();
		foreach(DiscoItem::Identity i, is) {
			if (i.category == "pubsub" && i.type == "pep")
				hasPEP_ = true;
		}
		
		if (f.test(QStringList("http://jabber.org/protocol/amp")))
		{
			if(amp_ == NULL)
				amp_ = new JT_AMP(client_->rootTask());
			amp_->set(client_->jid());
			amp_->onGo();
		}
		emit featuresChanged();
	}
}
