#include "SIMContactListContact.h"
#include "SIMContactListMeta.h"
#include "SIMContactListGroup.h"
#include "SIMContactList.h"
#include "SIMContactListModel.h"
#include "SIMContactListView.h"
#include "textutil.h"
#include "jidutil.h"
#include "common.h"
#include "iconset.h"
#include "psiiconset.h"
#include "psiaccount.h"
#include "psioptions.h"
#include "capsmanager.h"
#include "pgputil.h"
#include "avatars.h"
#include "desktoputil.h"

#ifdef USE_PEP
#include "serverinfomanager.h"
#endif

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QInputDialog>
#include <QPainter>
#include <QApplication>
#include "qclipboard.h"

SIMContactListContact::SIMContactListContact(const UserListItem &_u, PsiAccount *_pa, SIMContactList *cl, SIMContactListItem *parent)
:SIMContactListItem(SIMContactListItem::Contact, _pa, cl, parent), alertIcon_(NULL)
{
	printf("size(U) : %d\n", sizeof(_u));
	setUserListItem(_u);
}

SIMContactListContact::~SIMContactListContact()
{
}

QString SIMContactListContact::name()
{
	return JIDUtil::nickOrJid(u_.name(), u_.jid().full());
}

XMPP::Jid SIMContactListContact::jid()
{
	return u_.jid();
}

QPixmap SIMContactListContact::state()
{
	if(alertIcon_) {
		return alertIcon_->pixmap();
	}
	return PsiIconset::instance()->statusPtr(&u_)->pixmap();
}

QPixmap SIMContactListContact::pixmap()
{
	bool grey = !u_.isAvailable();
	QPixmap ps = state();
	QPixmap px;
	if(account()->avatarFactory() && (contactList()->avatarSize() > 0)) {
		px = QPixmap(account()->avatarFactory()->getAvatar(u_.jid().bare(), grey, contactList()->avatarSize()));
		if(!px.isNull()) {
			QPainter p(&px);
			QRect r(px.width() - (ps.width()+2), px.height() - ps.height(), ps.height(), ps.height());
			p.drawPixmap(r,ps);
		}
	}
	return (px.isNull()) ? ps : px;
}

QPixmap SIMContactListContact::avatar()
{
	bool grey = !u_.isAvailable();
	if(account()->avatarFactory() && (contactList()->avatarSize() > 0))
		return account()->avatarFactory()->getAvatar(u_.jid().bare(), grey, contactList()->avatarSize());
	return QPixmap();
}

QString SIMContactListContact::description()
{
	return status().status();
}

const SIMContactName &SIMContactListContact::contactName()
{
	return contactName_;
}

const QColor &SIMContactListContact::textColor()
{
	if(status().type() == Status::Away || status().type() == Status::XA) {
		return option.color[cAway];
	}
	else if (status().type() == Status::Offline) {
		return option.color[cOffline];
	}
	else if (status().type() == Status::DND) {
		return option.color[cDND];
	}
	else {
		return option.color[cOnline];
	}

}

bool SIMContactListContact::alerting()
{
	return (alertIcon_ != NULL);
}

void SIMContactListContact::setAlertIcon(PsiIcon *icon)
{
	alertIcon_ = icon;
}

void SIMContactListContact::setUserListItem(const UserListItem &_u)
{
	u_=_u;
	u_.setAvatarFactory(account()->avatarFactory());
	QString s = name();

	if (u_.isSelf())
		s = account()->name();
	if(!status().status().isEmpty()) {
		QFont fnt;
		QFont fnt2;
		fnt2.fromString(option.font[fRoster]);
		fnt.fromString(option.font[fStatus]);
		int size = fnt.pointSize() - fnt2.pointSize();
		s += "<br><font size='"+QString("%1").arg(size)+"' color='" + option.color[cStatus].name() + "'>";		
		if(fnt.bold())
			s += "<b>";
		if(fnt.italic())
			s += "<i>";
		if(PsiOptions::instance()->getOption("options.ui.style.rosterEmoticons").toBool())
			s += TextUtil::emoticonify(TextUtil::plain2rich(description()));
		else
			s += TextUtil::plain2rich(description());
		if(fnt.bold())
			s += "</b>";
		if(fnt.italic())
			s += "</i>";
		s += "</font>";
	}

	contactName_.setText(s, textColor(), contactList()->contactListView()->columnWidth(SIMContactListModel::NameColumn));
}

UserListItem *SIMContactListContact::u()
{
	return &u_;
}

XMPP::Status SIMContactListContact::status()
{
	if(!u_.userResourceList().isEmpty()) {
		UserResource &r = *u_.priority();
		return r.status();
	} else if(!u_.lastAvailable().isNull()) {
		return u_.lastUnavailableStatus();
	} else {
		return Status(Status::Offline);
	}
}

void SIMContactListContact::showContextMenu(const QPoint& p)
{
	bool online = account()->loggedIn();
	bool self = u_.isSelf();
	bool inList = u_.inList();
	bool isPrivate = u_.isPrivate();
	bool isAgent = u_.isTransport();
	bool avail = u_.isAvailable();

	QAction *status = NULL;
	QAction *mood = NULL;

	QAction *addToContactList = NULL;
	QAction *serviceDiscovery = NULL;
	QAction *xmlConsole = NULL;
	QAction *sendMessage = NULL;
	QAction *startChat = NULL;
	QAction *RC = NULL;
	QAction *voiceCall = NULL;
	QAction *upload = NULL;
	QAction *history = NULL;
	QAction *info = NULL;
	QAction *rename = NULL;
	QAction *authResend = NULL;
	QAction *authRequest = NULL;
	QAction *authRemove = NULL;
	QAction *remove = NULL;
	QAction *meta = NULL;
	QAction *group = NULL;
	QAction *avatAssign = NULL;
	QAction *avatClear = NULL;
	QAction *pgp = NULL;
	QAction *copyJid = NULL;
	QAction *copyStatusMsg = NULL;
	QAction *goToUrl = NULL;

	QAction *logon = NULL;

	QMenu pm(0);

	if(self) {
		status = pm.addAction(SIMContactList::tr("Set &Status"));
#ifdef USE_PEP
		if(online && account()->serverInfoManager()->hasPEP()) {
			QMenu *set = pm.addMenu(SIMContactList::tr("PEP"));
			mood = set->addAction(IconsetFactory::icon("psi/smile").icon(),SIMContactList::tr("Set Mood"));
			avatAssign = set->addAction(SIMContactList::tr("Set Avatar"));
			avatClear = set->addAction(SIMContactList::tr("Unset Avatar"));
		}
#endif
		addToContactList = pm.addAction(IconsetFactory::icon("psi/addContact").icon(), SIMContactList::tr("&Add a contact"));
		serviceDiscovery = pm.addAction(IconsetFactory::icon("psi/disco").icon(), SIMContactList::tr("Service &Discovery"));
		xmlConsole = pm.addAction(IconsetFactory::icon("psi/xml").icon(), SIMContactList::tr("&XML Console"));
		pm.addSeparator();
	}

	if(!self && !inList && !isPrivate && !option.lockdown.roster && online) {
		addToContactList = pm.addAction(IconsetFactory::icon("psi/addContact").icon(), SIMContactList::tr("Add/Authorize to contact list"));
		
		pm.addSeparator();
	}


	if (PsiOptions::instance()->getOption("options.ui.message.enabled").toBool() && online)
		sendMessage = pm.addAction(IconsetFactory::icon("psi/sendMessage").icon(), SIMContactList::tr("Send &message"));

	if (online)
		startChat = pm.addAction(IconsetFactory::icon("psi/start-chat").icon(), SIMContactList::tr("Open &chat window"));
	
#ifdef WHITEBOARDING
	QAction *openWhiteboard;
	if (online)
		openWhiteboard = pm.addAction(IconsetFactory::icon("psi/whiteboard").icon(), SIMContactList::tr("Open a &whiteboard"));
#endif

	if(!isPrivate && option.useRC) {
		RC = pm.addAction(SIMContactList::tr("E&xecute command"));
	}

	if(account()->voiceCaller() && !isAgent && online) {
		bool hasVoice = false;
		const UserResourceList &rl = u_.userResourceList();
		for (UserResourceList::ConstIterator it = rl.begin(); it != rl.end() && !hasVoice; ++it) {
			hasVoice = account()->capsManager()->features(u_.jid().withResource((*it).name())).canVoice();
		}
		if( hasVoice && account()->capsManager()->isEnabled() )
			voiceCall = pm.addAction(IconsetFactory::icon("psi/voice").icon(), SIMContactList::tr("Voice call"));
	}

	if(!isAgent && online) {
		pm.addSeparator();
		upload = pm.addAction(IconsetFactory::icon("psi/upload").icon(), SIMContactList::tr("Send &file"));
	}

	if(!self) {
		history = pm.addAction(IconsetFactory::icon("psi/history").icon(), SIMContactList::tr("&History"));
		info = pm.addAction(IconsetFactory::icon("psi/vCard").icon(), SIMContactList::tr("User &Info"));
	}

	if(!self && online && !option.lockdown.roster) {
		QMenu *manage = pm.addMenu(SIMContactList::tr("Manage"));

		rename = manage->addAction(IconsetFactory::icon("psi/edit/clear").icon(), SIMContactList::tr("Re&name"));

		QMenu *authorize = manage->addMenu(IconsetFactory::icon("psi/register").icon(), SIMContactList::tr("Authorization"));
		authResend = authorize->addAction(SIMContactList::tr("Resend authorization to"));
		authRequest = authorize->addAction(SIMContactList::tr("Rerequest authorization from"));
		authRemove = authorize->addAction(SIMContactList::tr("Remove authorization from"));

		remove = manage->addAction(IconsetFactory::icon("psi/remove").icon(), SIMContactList::tr("Rem&ove"));

		meta = manage->addAction(SIMContactList::tr("Add meta.."));
		group = manage->addAction(SIMContactList::tr("Add group.."));

		if (PsiOptions::instance()->getOption("options.ui.menu.contact.custom-picture").toBool()) {
			QMenu *avpm = manage->addMenu(SIMContactList::tr("&Picture"));

			avatAssign = avpm->addAction(SIMContactList::tr("&Assign Custom Picture"));
			if(account()->avatarFactory()->hasManualAvatar(u_.jid()))
				avatClear = avpm->addAction(SIMContactList::tr("&Clear Custom Picture"));
		}

		if(PGPUtil::instance().pgpAvailable() && PsiOptions::instance()->getOption("options.ui.menu.contact.custom-pgp-key").toBool()) {
			if(u_.publicKeyID().isEmpty())
				pgp = manage->addAction(IconsetFactory::icon("psi/gpg-yes").icon(), SIMContactList::tr("Assign Open&PGP key"));
			else
				pgp = manage->addAction(IconsetFactory::icon("psi/gpg-no").icon(), SIMContactList::tr("Unassign Open&PGP key"));
		}
	}

	if(!self && online && isAgent)
		if(!u_.isAvailable())
			logon = pm.addAction(PsiIconset::instance()->status(jid(), STATUS_ONLINE).icon(), SIMContactList::tr("&Log on"));
		else
			logon = pm.addAction(PsiIconset::instance()->status(jid(), STATUS_OFFLINE).icon(), SIMContactList::tr("&Log off"));

	copyJid = pm.addAction(SIMContactList::tr("Copy JID"));
	if(!description().isEmpty()) {
		copyStatusMsg = pm.addAction(SIMContactList::tr("Copy status message"));
		if(TextUtil::linkify(description()).compare(description()) != 0)
			goToUrl = pm.addAction(SIMContactList::tr("Go to URL.."));
	}

	QAction *ret = pm.exec(p);
	if(ret == NULL)
		return;

	if (ret == status) {
		account()->changeStatus(0);
#ifdef USE_PEP
	} else if (ret == mood) {
		account()->actionSetMood();
#endif
	} else if (ret == serviceDiscovery) {
		account()->actionDisco(Jid(account()->jid().host()),"");
	} else if (ret == xmlConsole) {
		account()->showXmlConsole();
	} else if (ret == addToContactList) {
		if(self)
			account()->openAddUserDlg();
		else {
			account()->actionAdd(u_.jid());
			account()->actionAuth(u_.jid());
		}
//		QMessageBox::information(, tr("Add"), tr("Added/Authorized <b>%1</b> to the contact list.").arg(name()));
	} else if (ret == sendMessage) {
		account()->actionSendMessage(u_.jid());
	} else if (ret == startChat) {
		account()->actionOpenChat(u_.jid());
#ifdef WHITEBOARDING
	} else if (ret == openWhiteboard) {
		account()->actionOpenWhiteboard(u_.jid());
#endif
	} else if (ret == RC) {
		const UserResourceList &rl = u_.userResourceList(); 
		QStringList rnl;
		for(UserResourceList::ConstIterator it = rl.begin(); it != rl.end(); ++it) {
			const UserResource &r = *it;
			rnl << r.name();
		}
		account()->actionExecuteCommandSpecific(Jid(jid().bare() + "/" + QInputDialog::getItem(contactList()->contactListView(),
			SIMContactList::tr("Choose resource.."), SIMContactList::tr("Choose resource.."),
			rnl)), "");
	} else if (ret == voiceCall) {
		account()->actionVoice(u_.jid());
	} else if (ret == upload) {
		account()->actionSendFile(u_.jid());
	} else if (ret == history) {
		account()->actionHistory(u_.jid());
	} else if (ret == info) {
		account()->actionInfo(u_.jid());
	} else if (ret == rename) {
//		account()->actionRename(u.jid());
		QModelIndex m = ((SIMContactListModel*)contactList()->contactListView()->model())->index(row(), SIMContactListModel::NameColumn, this);
		contactList()->contactListView()->edit(m);
	} else if (ret == group) {
		while(1) {
			bool ok = false;
			QString newgroup = QInputDialog::getText(SIMContactList::tr("Create New Group"), SIMContactList::tr("Enter the new Group name:"), QLineEdit::Normal, QString::null, &ok, contactList()->contactListView());
			if(!ok)
				break;
			if(newgroup.isEmpty())
				continue;

			// make sure we don't have it already
			bool found = false;
			const QStringList &groups = u_.groups();
			for(QStringList::ConstIterator it = groups.begin(); it != groups.end(); ++it) {
				if(*it == newgroup) {
					found = true;
					break;
				}
			}

			if(!found) {
				SIMContactListGroup *group = dynamic_cast<SIMContactListGroup*>(parent());
				if(group) {
					account()->actionGroupRemove(u_.jid(), group->name());
					account()->actionGroupAdd(u_.jid(), newgroup);
					break;
				}
			}
		}
	} else if (ret == meta) {
		while(1) {
			bool ok = false;
			QString newmeta = QInputDialog::getText(SIMContactList::tr("Create New Metacontact"), SIMContactList::tr("Enter the new Metacontact name:"), QLineEdit::Normal, QString::null, &ok, contactList()->contactListView());
			int newmeta_priority = QInputDialog::getInteger(SIMContactList::tr("Set priority for contact"), SIMContactList::tr("Enter the priority:"), 1, 1,100,1, &ok, contactList()->contactListView());
			if(!ok)
				break;
			if(newmeta.isEmpty())
				continue;

			// make sure we don't have it already
			bool found = false;
			const QStringList &metas = u_.metas();
			for(QStringList::ConstIterator it = metas.begin(); it != metas.end(); ++it) {
				if(*it == newmeta) {
					found = true;
					break;
				}
			}

			if(!found) {
				account()->actionMetaAdd(u_.jid(), newmeta, newmeta_priority);
				break;
/*				SIMContactListMeta *meta = dynamic_cast<SIMContactListMeta*>(parent());
				if(meta) {
					account()->actionMetaRemove(u_.jid(), group->name());
					account()->actionMetaAdd(u_.jid(), newgroup, newmeta_priority);
					break;
				}*/
			}
		}
	} else if (ret == authResend) {
		account()->actionAuth(u_.jid());
//		QMessageBox::information(this, tr("Authorize"), tr("Resend authorization to <b>%1</b>.").arg(name()));
	} else if (ret == authRequest) {
		account()->actionAuthRequest(u_.jid());
//		QMessageBox::information(this, tr("Authorize"), tr("Rerequested authorization from <b>%1</b>.").arg(name()));
	} else if (ret == authRemove) {
		account()->actionAuthRemove(u_.jid());
//		QMessageBox::information(this, tr("Authorize"), tr("Removed authorization from <b>%1</b>.").arg(name()));
	} else if (ret == remove) {
		account()->actionRemove(u_.jid());
	} else if (ret == avatAssign) {
#ifdef USE_PEP
		if(self)
			account()->actionSetAvatar();
		else {
#endif
  			QString file = QFileDialog::getOpenFileName(0, SIMContactList::tr("Choose an image"), "", SIMContactList::tr("All files (*.png *.jpg *.gif)"));
 			if(!file.isNull()) {
 				account()->avatarFactory()->importManualAvatar(u_.jid(), file);
 			}
#ifdef USE_PEP
		}
#endif
	} else if (ret == avatClear) {
#ifdef USE_PEP
		if(self)
			account()->actionUnsetAvatar();
		else
#endif
			account()->avatarFactory()->removeManualAvatar(u_.jid());
	} else if (ret == pgp) {
		if(u_.publicKeyID().isEmpty())
			account()->actionAssignKey(u_.jid());
		else 
			account()->actionUnassignKey(u_.jid());
	} else if (ret == logon) {
		if(!u_.isAvailable()) {
			Status s=makeStatus(STATUS_ONLINE,"");
			account()->actionAgentSetStatus(jid(), s);
		} else {
			Status s=makeStatus(STATUS_OFFLINE,"");
			account()->actionAgentSetStatus(jid(), s);
		}
	} else if (ret == copyJid) {
		QClipboard *clipboard = QApplication::clipboard();
		QString cliptext = jid().bare();
			
		clipboard->setText(cliptext, QClipboard::Clipboard);
		clipboard->setText(cliptext, QClipboard::Selection);	
	} else if (ret == copyStatusMsg) {
		QClipboard *clipboard = QApplication::clipboard();
		QString cliptext = description();
			
		clipboard->setText(cliptext, QClipboard::Clipboard);
		clipboard->setText(cliptext, QClipboard::Selection);	
	} else if (ret == goToUrl) {
		QString url = TextUtil::linkify(description());
		int i = url.find("href=\"");
		int j = url.find("\">",i);
		if ( j == -1 )
			j = url.find("\" >", i);
		i = i+6;
		url = url.mid(i, j - i);
		DesktopUtil::openUrl(url);
	}
}

QString SIMContactListContact::toolTip()
{
	return u_.makeBareTip(false,true);
}

SIMContactListItem *SIMContactListContact::updateParent(SIMContactListContact *item, SIMContactList *contactList)
{
	SIMContactListItem *newParent = item->parent();

	if (!contactList->search().isEmpty()) {
		QString search = contactList->search();
		if (item->name().contains(search) || item->jid().bare().contains(search)) {
			newParent = item->defaultParent();;
		} else {
			newParent = contactList->searchGroup();
		}
	}
 	else if (!contactList->showOffline() && item->status().type() == Status::Offline && !item->alerting() && !item->u()->isSelf()) {
		newParent = contactList->invisibleGroup();
	}
	else if (!contactList->showAway() && (item->status().type() == Status::Away || item->status().type() == Status::XA)) {
		newParent = contactList->invisibleGroup();
	}
 	else if (!contactList->showAgents() &&  item->u()->isTransport()) {
		 return newParent = contactList->invisibleGroup();
	}
 	else if (!contactList->showSelf() &&  item->u()->isSelf()) {
		newParent = contactList->invisibleGroup();
	}
	else {
		newParent = item->defaultParent();
	}

	if(contactList->showSelf() && item->u()->isSelf() && contactList->search().isEmpty()) {
		newParent = item->defaultParent();
	}

	if ((!contactList->showGroups() || !contactList->search().isEmpty()) && newParent != contactList->searchGroup()) {
		newParent = contactList->rootItem();
	}

	return newParent;
}

void SIMContactListContact::updateOptions()
{
	UserListItem uloc = *u();
	setUserListItem(uloc);
}

