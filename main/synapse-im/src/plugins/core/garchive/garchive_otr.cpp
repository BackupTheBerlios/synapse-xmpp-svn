#include "garchive_otr.h"
#include "xmpp.h"
#include "xmpp_xmlcommon.h"
#include "psicon.h"
#include "psiaccount.h"

using namespace Otr;

OtrItem::OtrItem(const Jid& _jid, bool _on)
{
	jid_ = _jid;
	on_ = _on;
}

XMPP::Jid& OtrItem::jid()
{
	return jid_;
}

void OtrItem::setJid(const Jid& _jid)
{
	jid_ = _jid;
}

void OtrItem::setOn(bool _on)
{
	on_ = _on;
}

bool OtrItem::on()
{
	return on_;
}

OtrList::OtrList()
{
}

OtrList::Iterator OtrList::find(const XMPP::Jid& _jid)
{
	for(OtrList::Iterator it = begin(); it != end(); it++)
	{
		if((*it).jid().compare(_jid.bare()))
			return it;
	}
	return end();
}

bool OtrList::isOn(const XMPP::Jid& _jid)
{
	OtrList::Iterator oi = find(_jid);
	if(oi != end())
		return (*oi).on();
	return false;
}

void OtrList::setOn(const XMPP::Jid& _jid, bool _on)
{
	OtrList::Iterator oi = find(_jid);
	if(oi == end())
	{
		(*this) +=  OtrItem(_jid, _on);
		//append(oii);
	} else
		(*oi).setOn(_on);
}

