/*
 * antievil.cpp - anti evil scanner task
 * Copyright (C) 2007-04-01  Maciej Niedzielski
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "antievil.h"
#include "psioptions.h"
#include "xmpp_xmlcommon.h"

using namespace XMPP;


/**
 * \class AntiEvil
 * \brief XEP-0076 implementation
 *
 * This tasks protects from evil
 */


AntiEvil::Stats AntiEvil::s;

AntiEvil::AntiEvil(Task *parent)
:Task(parent)
{
}

AntiEvil::~AntiEvil()
{
}

bool AntiEvil::take(const QDomElement &e)
{
	if (!PsiOptions::instance()->getOption("options.anti-evil.enable").toBool())
		return false;

	bool evil = false;
	for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if (!i.isNull() && i.tagName() == "evil" && i.attribute("xmlns") == "http://jabber.org/protocol/evil") {
			//qDebug("evil stanza received");
			evil = true;
			break;
		}
	}

	++s.scanned;
	emit s.scannedNext(s.scanned);

	if (!evil)
		return false;

	++s.blocked;
	s.lastBlockedFrom = e.attribute("from");
	s.lastBlockedTime = QDateTime::currentDateTime();
	emit s.blockedNext(s.blocked, s.lastBlockedFrom, s.lastBlockedTime);

	return true;
}

