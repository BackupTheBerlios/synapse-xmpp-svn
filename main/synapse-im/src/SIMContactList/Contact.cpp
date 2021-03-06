#include "Contact.h"
//#include "PubSub.h"
#include "Meta.h"
#include "Group.h"
#include "List.h"
#include "Model.h"
#include "View.h"
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
#include "bookmarkmanagedlg.h"
#include "bookmarkmanager.h"

#ifdef USE_PEP
#include "serverinfomanager.h"
#include "geolocation.h"
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

using namespace SIMContactList;

Contact::Contact(const UserListItem &_u, PsiAccount *_pa, List *cl, Item *parent)
:Item(Item::TContact, _pa, cl, parent), alertIcon_(NULL), blocked_(false)
{
	setUserListItem(_u);
}

Contact::~Contact()
{
}

QString Contact::name()
{
	return JIDUtil::nickOrJid(u_.name(), u_.jid().full());
}

XMPP::Jid Contact::jid()
{
	return u_.jid();
}

QPixmap Contact::state()
{
	if(alertIcon_) {
		return alertIcon_->pixmap();
	}

	if(blocked_)
		return IconsetFactory::icon("psi/stop").pixmap();

	return PsiIconset::instance()->statusPtr(&u_)->pixmap();
}

QPixmap Contact::pixmap()
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

QPixmap Contact::avatar()
{
	bool grey = !u_.isAvailable();
	if(account()->avatarFactory() && (contactList()->avatarSize() > 0))
		return account()->avatarFactory()->getAvatar(u_.jid().bare(), grey, contactList()->avatarSize());
	return QPixmap();
}

QString Contact::description()
{
	if(u_.isAvailable())
		return status().status();
	else
		return u_.lastUnavailableStatus().status();
}

const Name &Contact::contactName()
{
	return contactName_;
}

const QColor &Contact::textColor()
{
	if(status().type() == Status::Away || status().type() == Status::XA) {
		return PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.status.away").value<QColor>();
	}
	else if (status().type() == Status::Offline) {
		return PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.status.offline").value<QColor>();
	}
	else if (status().type() == Status::DND) {
		return PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.status.do-not-disturb").value<QColor>();
	}
	else {
		return PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.status.online").value<QColor>();
	}

}

bool Contact::alerting()
{
	return (alertIcon_ != NULL);
}

void Contact::setAlertIcon(PsiIcon *icon)
{
	alertIcon_ = icon;
}

void Contact::setBlocked(bool blocked)
{
	blocked_ = blocked;
	setUserListItem(u_);
}

void Contact::setUserListItem(const UserListItem &_u)
{
	u_=_u;
	u_.setAvatarFactory(account()->avatarFactory());
	QString s = name();

	if (u_.isSelf())
		s = account()->name();
	if(!description().isEmpty()) {
		QFont fnt;
		QFont fnt2;
		fnt2.fromString(PsiOptions::instance()->getOption("options.ui.look.font.contactlist").toString());
		fnt.fromString(PsiOptions::instance()->getOption("options.ui.look.font.status").toString());
		int size = fnt.pointSize() - fnt2.pointSize();
		s += "<br><font size='"+QString("%1").arg(size)+"' color='" + PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.status-messages").value<QColor>().name() + "'>";		
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

	contactName_.setText(s, textColor(),
	contactList()->contactListView());
}

UserListItem *Contact::u()
{
	return &u_;
}

XMPP::Status Contact::status()
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

void Contact::showContextMenu(const QPoint& p)
{
	bool online = account()->loggedIn();
	bool self = u_.isSelf();
	bool inList = u_.inList();
	bool isPrivate = u_.isPrivate();
	bool isAgent = u_.isTransport();
	bool avail = u_.isAvailable();

	QAction *status = NULL;
	QAction *mood = NULL;

	QAction *geo = NULL;
	QAction *bookmarksManage = NULL;
	QMap<QAction*,int> bookmarks;

	QAction *addToContactList = NULL;
	QAction *serviceDiscovery = NULL;
	QAction *xmlConsole = NULL;
	QAction *sendMessage = NULL;
	QAction *startChat = NULL;
#ifdef WHITEBOARDING
	QAction *openWhiteboard = NULL;
#endif
	QAction *RC = NULL;
	QAction *voiceCall = NULL;
	QAction *upload = NULL;
#ifdef XEP-0136
	QAction *archive = NULL;
#endif
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
	QAction *block = NULL;
	QAction *unblock = NULL;
	QAction *copyJid = NULL;
	QAction *copyStatusMsg = NULL;
	QAction *goToUrl = NULL;

	QAction *logon = NULL;

	QMenu pm(0);

	if(self) {
		status = pm.addAction(List::tr("Set &Status"));
#ifdef USE_PEP
		if(online && account()->serverInfoManager()->hasPEP()) {
			QMenu *set = pm.addMenu(List::tr("PEP"));
			mood = set->addAction(IconsetFactory::icon("psi/smile").icon(),List::tr("Set Mood"));
			geo = set->addAction(List::tr("Geolocation"));
			avatAssign = set->addAction(List::tr("Set Avatar"));
			avatClear = set->addAction(List::tr("Unset Avatar"));
		}
#endif

		QMenu *bm = pm.addMenu(List::tr("Bookmarks"));
		bookmarksManage = bm->addAction(List::tr("Manage..."));
		if (account()->bookmarkManager()->isAvailable()) {
			int idx = 0;
			bm->insertSeparator();
			foreach(ConferenceBookmark c, account()->bookmarkManager()->conferences()) {
				bookmarks[bm->addAction(QString(List::tr("Join %1")).arg(c.name()))] = idx;
				idx++;
			}
		}
		else {
			bm->setEnabled(false);
		}

		addToContactList = pm.addAction(IconsetFactory::icon("psi/addContact").icon(), List::tr("&Add a contact"));
		serviceDiscovery = pm.addAction(IconsetFactory::icon("psi/disco").icon(), List::tr("Service &Discovery"));
		xmlConsole = pm.addAction(IconsetFactory::icon("psi/xml").icon(), List::tr("&XML Console"));
		pm.addSeparator();
	} else {

		if(!inList && !isPrivate && !PsiOptions::instance()->getOption("options.ui.contactlist.lockdown-roster").toBool() && online) {
			addToContactList = pm.addAction(IconsetFactory::icon("psi/addContact").icon(), List::tr("Add/Authorize to contact list"));
		
			pm.addSeparator();
		}

		if (PsiOptions::instance()->getOption("options.ui.message.enabled").toBool() && online)
			sendMessage = pm.addAction(IconsetFactory::icon("psi/sendMessage").icon(), List::tr("Send &message"));

		if (online)
			startChat = pm.addAction(IconsetFactory::icon("psi/start-chat").icon(), List::tr("Open &chat window"));
	
#ifdef WHITEBOARDING
		if (online)
			openWhiteboard = pm.addAction(IconsetFactory::icon("psi/whiteboard").icon(), List::tr("Open a &whiteboard"));
#endif
	}

	if(!isPrivate && PsiOptions::instance()->getOption("options.external-control.adhoc-remote-control.enable").toBool()) {
		RC = pm.addAction(List::tr("E&xecute command"));
	}

	if(!self) {
		if(account()->voiceCaller() && !isAgent && online) {
			bool hasVoice = false;
			const UserResourceList &rl = u_.userResourceList();
			for (UserResourceList::ConstIterator it = rl.begin(); it != rl.end() && !hasVoice; ++it) {
			hasVoice = account()->capsManager()->features(u_.jid().withResource((*it).name())).canVoice();
		}
		if( hasVoice && account()->capsManager()->isEnabled() )
			voiceCall = pm.addAction(IconsetFactory::icon("psi/voice").icon(), List::tr("Voice call"));
		}

		if(!isAgent && online) {
			pm.addSeparator();
			upload = pm.addAction(IconsetFactory::icon("psi/upload").icon(), List::tr("Send &file"));
		}

#ifdef XEP-0136
		ServerInfoManager *sim = account()->serverInfoManager();
//		if(sim && sim->hasMessageArchiving()) {
			archive = pm.addAction(IconsetFactory::icon("psi/history").icon(), List::tr("&Archive"));
//		}
#endif

		history = pm.addAction(IconsetFactory::icon("psi/history").icon(), List::tr("&History"));
	}

	info = pm.addAction(IconsetFactory::icon("psi/vCard").icon(), List::tr("User &Info"));

	if(!self && online && !PsiOptions::instance()->getOption("options.ui.contactlist.lockdown-roster").toBool()) {
#ifdef USE_PEP
		if(!u_.geoLocation().isNull())
			geo = pm.addAction(List::tr("Geolocation"));
#endif
		QMenu *manage = pm.addMenu(List::tr("Manage"));

		rename = manage->addAction(IconsetFactory::icon("psi/edit/clear").icon(), List::tr("Re&name"));

		QMenu *authorize = manage->addMenu(IconsetFactory::icon("psi/register").icon(), List::tr("Authorization"));
		authResend = authorize->addAction(List::tr("Resend authorization to"));
		authRequest = authorize->addAction(List::tr("Rerequest authorization from"));
		authRemove = authorize->addAction(List::tr("Remove authorization from"));

		remove = manage->addAction(IconsetFactory::icon("psi/remove").icon(), List::tr("Rem&ove"));

		meta = manage->addAction(List::tr("Add meta.."));
		group = manage->addAction(List::tr("Add group.."));

		if (PsiOptions::instance()->getOption("options.ui.menu.contact.custom-picture").toBool()) {
			QMenu *avpm = manage->addMenu(List::tr("&Picture"));

			avatAssign = avpm->addAction(List::tr("&Assign Custom Picture"));
			if(account()->avatarFactory()->hasManualAvatar(u_.jid()))
				avatClear = avpm->addAction(List::tr("&Clear Custom Picture"));
		}

		if(PGPUtil::instance().pgpAvailable() && PsiOptions::instance()->getOption("options.ui.menu.contact.custom-pgp-key").toBool()) {
			if(u_.publicKeyID().isEmpty())
				pgp = manage->addAction(IconsetFactory::icon("psi/gpg-yes").icon(), List::tr("Assign Open&PGP key"));
			else
				pgp = manage->addAction(IconsetFactory::icon("psi/gpg-no").icon(), List::tr("Unassign Open&PGP key"));
		}
	}

	if(!self && online) {
		if(isAgent) {
			if(!u_.isAvailable())
				logon = pm.addAction(PsiIconset::instance()->status(jid(), STATUS_ONLINE).icon(), List::tr("&Log on"));
			else
				logon = pm.addAction(PsiIconset::instance()->status(jid(), STATUS_OFFLINE).icon(), List::tr("&Log off"));
		}
		if(blocked_)
			unblock = pm.addAction(IconsetFactory::icon("psi/stop").icon(), List::tr("Unblock contact"));
		else
			block = pm.addAction(IconsetFactory::icon("psi/stop").icon(), List::tr("Block contact"));

		copyJid = pm.addAction(List::tr("Copy JID"));
		if(!description().isEmpty()) {
			copyStatusMsg = pm.addAction(List::tr("Copy status message"));
			if(TextUtil::linkify(description()).compare(description()) != 0)
				goToUrl = pm.addAction(List::tr("Go to URL.."));
		}

	}

	QAction *ret = pm.exec(p);
	if(ret == NULL)
		return;

	if (ret == status) {
		account()->changeStatus(0);
#ifdef USE_PEP
	} else if (ret == mood) {
		account()->actionSetMood();
	} else if (ret == geo) {
		if(u_.isSelf())
			account()->actionSetGeolocation();
		else
			account()->actionShowGeolocation(u_.jid());
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
			List::tr("Choose resource.."), List::tr("Choose resource.."),
			rnl)), "");
	} else if (ret == voiceCall) {
		account()->actionVoice(u_.jid());
	} else if (ret == upload) {
		account()->actionSendFile(u_.jid());
#ifdef XEP-0136
	} else if (ret == archive) {
		account()->actionArchive(u_.jid());
#endif
	} else if (ret == history) {
		account()->actionHistory(u_.jid());
	} else if (ret == info) {
		if(!self)
			account()->actionInfo(u_.jid());
		else
			account()->changeVCard();
	} else if (ret == rename) {
//		account()->actionRename(u.jid());
		QModelIndex m = ((Model*)contactList()->contactListView()->model())->index(row(), Model::NameColumn, this);
		contactList()->contactListView()->edit(m);
	} else if (ret == group) {
		while(1) {
			bool ok = false;
			QString newgroup = QInputDialog::getText(List::tr("Create New Group"), List::tr("Enter the new Group name:"), QLineEdit::Normal, QString::null, &ok, contactList()->contactListView());
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
				Group *group = dynamic_cast<Group*>(parent());
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
			QString newmeta = QInputDialog::getText(List::tr("Create New Metacontact"), List::tr("Enter the new Metacontact name:"), QLineEdit::Normal, QString::null, &ok, contactList()->contactListView());
			int newmeta_priority = QInputDialog::getInteger(List::tr("Set priority for contact"), List::tr("Enter the priority:"), 1, 1,100,1, &ok, contactList()->contactListView());
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
  			QString file = QFileDialog::getOpenFileName(0, List::tr("Choose an image"), "", List::tr("All files (*.png *.jpg *.gif)"));
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
	} else if (ret == unblock) {
		account()->unblockContact(jid().bare());
	} else if (ret == block) {
		account()->blockContact(jid().bare());
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
	} else if (ret == bookmarksManage) {
		BookmarkManageDlg *dlg = account()->findDialog<BookmarkManageDlg*>();
		if(dlg) {
			bringToFront(dlg);
		} else {
			dlg = new BookmarkManageDlg(account());
			dlg->show();
		}
	} else {
		ConferenceBookmark c = account()->bookmarkManager()->conferences()[bookmarks[ret]];
		account()->actionJoin(c, true);
	}
}

QString Contact::toolTip()
{
	return u_.makeBareTip(false,true);
}

Item *Contact::updateParent(Contact *item, List *contactList)
{
	Item *newParent = item->parent();

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

void Contact::updateOptions()
{
	UserListItem uloc = *u();
	setUserListItem(uloc);
}

