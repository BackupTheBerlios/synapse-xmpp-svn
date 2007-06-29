#include "ahc_plugin.h"
#include "ahcommanddlg.h"
#include "ahcservermanager.h"
#include "psiaccount.h"
#include <QtGui>

AHCBox::~AHCBox()
{
}

void AHCBox::setup(PsiAccount *pa, const XMPP::Jid& receiver, bool enable)
{
	pa_ = pa;
}

void AHCBox::init() {
 	rcSetStatusServer = 0;
	rcForwardServer = 0;
	rcSetOptionsServer = 0;
	ahcManager = new AHCServerManager(pa_);
}

//Not used

void AHCBox::reset()
{
}

void AHCBox::setJid(const XMPP::Jid& receiver)
{
}

bool AHCBox::isEnabled()
{
	return false;
}

void AHCBox::setEnabled(bool enable)
{
 	if (!rcSetStatusServer && enable) {
 		rcSetStatusServer = new RCSetStatusServer(ahcManager);
 		rcForwardServer = new RCForwardServer(ahcManager);
 		rcSetOptionsServer = new RCSetOptionsServer(ahcManager, psi_);
 	} else if (!enable) {
 		delete rcSetStatusServer;
 		rcSetStatusServer = 0;
 		delete rcForwardServer;
 		rcForwardServer = 0;
 		delete rcSetOptionsServer;
 		rcSetOptionsServer = 0;
	}

}

bool AHCBox::isEvent(const XMPP::Jid& _jid)
{
	return false;
}

int AHCBox::countEvents()
{
	return 0;
}

void AHCBox::changed()
{
}

ContactViewItem *AHCBox::cvi()
{
	return NULL;
}

void AHCBox::setCvi(ContactViewItem *i)
{
}

//--------

void AHCBox::setPsiCon(PsiCon *psi)
{
	psi_ = psi;
}

void AHCBox::process(const XMPP::Jid& j, const QString& s)
{
	if(s.isEmpty()) {
		AHCommandDlg *w = new AHCommandDlg(pa_, j);
		w->show();
	} else {
 		AHCommandDlg::executeCommand(pa_->client(), j, s);
	}
}

QString AHCBox::name()
{
	return QString("Ad-hoc Commands");
}

QString AHCBox::version()
{
	return QString("20070628");
}

Q_EXPORT_PLUGIN2(ahc, AHCBox);
