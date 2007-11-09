/*
 * chatdlg.cpp - dialog for handling chats
 * Copyright (C) 2001-2007  Justin Karneges, Michail Pishchagin
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

#include "chatdlg.h"

#include <QLabel>
#include <QCursor>
#include <Q3DragObject>
#include <QLineEdit>
#include <QToolButton>
#include <QLayout>
#include <QSplitter>
#include <QToolBar>
#include <QTimer>
#include <QDateTime>
#include <QPixmap>
#include <QColor>
#include <Qt>
#include <QCloseEvent>
#include <QList>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QDropEvent>
#include <QList>
#include <QMessageBox>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QContextMenuEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QDragEnterEvent>
#include <QTextDocument> // for Qt::escape()

#include "profiles.h"
#include "psiaccount.h"
#include "common.h"
#include "userlist.h"
#include "stretchwidget.h"
#include "psiiconset.h"
#include "iconwidget.h"
#include "textutil.h"
#include "xmpp_message.h"
#include "xmpp_htmlelement.h"
#include "xmpp_xmlcommon.h"
#include "fancylabel.h"
#include "msgmle.h"
#include "iconselect.h"
#include "pgputil.h"
#include "psicon.h"
#include "iconlabel.h"
#include "capsmanager.h"
#include "iconaction.h"
#include "avatars.h"
#include "jidutil.h"
#include "tabdlg.h"
#include "psioptions.h"
#include "psitooltip.h"
#include "shortcutmanager.h"
#include "psicontactlist.h"
#include "accountlabel.h"
#include "serverinfomanager.h"

#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include "psirichtext.h"
#include "hoverlabel.h"

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include "psichatdlg.h"
  
ChatDlg* ChatDlg::create(const Jid& jid, PsiAccount* account, TabManager* tabManager)
{
	ChatDlg* chat = new PsiChatDlg(jid, account, tabManager);
	chat->init();
	chat->ensureTabbedCorrectly();
	return chat;
}
  
ChatDlg::ChatDlg(const Jid& jid, PsiAccount* pa, TabManager* tabManager)
	: TabbableWidget(jid, pa, tabManager)
	, jid_(jid)
	, pa_(pa)
	, highlightersInstalled_(false)
{
	if (option.brushedMetal) {
		setAttribute(Qt::WA_MacMetalStyle);
	}

	pending_ = 0;
	keepOpen_ = false;
	warnSend_ = false;
	selfDestruct_ = 0;
	transid_ = -1;
	key_ = "";
	lastWasEncrypted_ = false;

	status_ = -1;

	// Message events
	contactChatState_ = StateNone;
	lastChatState_ = StateNone;
	sendComposingEvents_ = false;
	isComposing_ = false;
	composingTimer_ = 0;
}
 
void ChatDlg::init()
{
	initUi();
	initActions();
	setShortcuts();
 
	// TODO: this have to be moved to chatEditCreated()
	chatView()->setDialog(this);
	chatEdit()->setDialog(this);

	chatEdit()->installEventFilter(this);
	connect(chatView(), SIGNAL(selectionChanged()), SLOT(logSelectionChanged()));

	// SyntaxHighlighters modify the QTextEdit in a QTimer::singleShot(0, ...) call
	// so we need to install our hooks after it fired for the first time
	QTimer::singleShot(10, this, SLOT(initComposing()));
	connect(this, SIGNAL(composing(bool)), SLOT(updateIsComposing(bool)));

	setAcceptDrops(TRUE);
	updateContact(jid(), true);

	X11WM_CLASS("chat");
	setLooks();

	updatePGP();

	connect(account(), SIGNAL(pgpKeyChanged()), SLOT(updatePGP()));
	connect(account(), SIGNAL(encryptedMessageSent(int, bool, int)), SLOT(encryptedMessageSent(int, bool, int)));
	account()->dialogRegister(this, jid());

	chatView()->setFocusPolicy(Qt::NoFocus);
	chatEdit()->setFocus();
	hover_ = new HoverLabel(chatView(), true);

	// TODO: port to restoreSavedSize() (and adapt it from restoreSavedGeometry())
	resize(PsiOptions::instance()->getOption("options.ui.chat.size").toSize());
}

ChatDlg::~ChatDlg()
{
	account()->dialogUnregister(this);
}

void ChatDlg::initComposing()
{
	highlightersInstalled_ = true;
	chatEditCreated();
}

void ChatDlg::initActions()
{
	act_send_ = new QAction(this);
	addAction(act_send_);
	connect(act_send_, SIGNAL(activated()), SLOT(doSend()));

	act_close_ = new QAction(this);
	addAction(act_close_);
	connect(act_close_, SIGNAL(activated()), SLOT(close()));

	act_scrollup_ = new QAction(this);
	addAction(act_scrollup_);
	connect(act_scrollup_, SIGNAL(activated()), SLOT(scrollUp()));

	act_scrolldown_ = new QAction(this);
	addAction(act_scrolldown_);
	connect(act_scrolldown_, SIGNAL(activated()), SLOT(scrollDown()));

// Text formating XHTML-IM
	act_italic = new QAction(this);
	connect(act_italic, SIGNAL(activated()), SLOT(toggleItalic()));
	addAction(act_italic);

	act_bold = new QAction(this);
	connect(act_bold, SIGNAL(activated()), SLOT(toggleBold()));
	addAction(act_bold);

	act_underline = new QAction(this);
	connect(act_underline, SIGNAL(activated()), SLOT(toggleUnderline()));
	addAction(act_underline);

	act_red = new QAction(this);
	connect(act_red, SIGNAL(activated()), SLOT(toggleRed()));
	addAction(act_red);

	act_green = new QAction(this);
	connect(act_green, SIGNAL(activated()), SLOT(toggleGreen()));
	addAction(act_green);

	act_blue = new QAction(this);
	connect(act_blue, SIGNAL(activated()), SLOT(toggleBlue()));
	addAction(act_blue);

	act_black = new QAction(this);
	connect(act_black, SIGNAL(activated()), SLOT(toggleBlack()));
	addAction(act_black);
//--------------------------
}

void ChatDlg::setShortcuts()
{
	//act_send_->setShortcuts(ShortcutManager::instance()->shortcuts("chat.send"));
	act_scrollup_->setShortcuts(ShortcutManager::instance()->shortcuts("common.scroll-up"));
	act_scrolldown_->setShortcuts(ShortcutManager::instance()->shortcuts("common.scroll-down"));

	act_italic->setShortcuts(ShortcutManager::instance()->shortcuts("chat.italic"));
	act_bold->setShortcuts(ShortcutManager::instance()->shortcuts("chat.bold"));
	act_underline->setShortcuts(ShortcutManager::instance()->shortcuts("chat.underline"));

	act_red->setShortcuts(ShortcutManager::instance()->shortcuts("chat.color.red"));
	act_green->setShortcuts(ShortcutManager::instance()->shortcuts("chat.color.green"));
	act_blue->setShortcuts(ShortcutManager::instance()->shortcuts("chat.color.blue"));
	act_black->setShortcuts(ShortcutManager::instance()->shortcuts("chat.color.black"));

	//if(!option.useTabs) {
	//	act_close_->setShortcuts(ShortcutManager::instance()->shortcuts("common.close"));
	//}
}

void ChatDlg::scrollUp()
{
	chatView()->verticalScrollBar()->setValue(chatView()->verticalScrollBar()->value() - chatView()->verticalScrollBar()->pageStep() / 2);
}

void ChatDlg::scrollDown()
{
	chatView()->verticalScrollBar()->setValue(chatView()->verticalScrollBar()->value() + chatView()->verticalScrollBar()->pageStep() / 2);
}
  
// FIXME: This should be unnecessary, since these keys are all registered as
// actions in the constructor. Somehow, Qt ignores this sometimes (i think
// just for actions that have no modifier).
void ChatDlg::keyPressEvent(QKeyEvent *e)
{
	QKeySequence key = e->key() + (e->modifiers() & ~Qt::KeypadModifier);
	if (!option.useTabs && ShortcutManager::instance()->shortcuts("common.close").contains(key)) {
		close();
	}
	else if (ShortcutManager::instance()->shortcuts("chat.send").contains(key)) {
		doSend();
	}
	else if (ShortcutManager::instance()->shortcuts("common.scroll-up").contains(key)) {
		scrollUp();
	}
	else if (ShortcutManager::instance()->shortcuts("common.scroll-down").contains(key)) {
		scrollDown();
	}
	else {
		e->ignore();
	}
}

void ChatDlg::resizeEvent(QResizeEvent *e)
{
	if (option.keepSizes) {
		PsiOptions::instance()->setOption("options.ui.chat.size", e->size());
	}
}

void ChatDlg::closeEvent(QCloseEvent *e)
{
	if (readyToHide()) {
		e->accept();
	}
	else {
		e->ignore();
	}
}

/**
 * Runs all the gumph necessary before hiding a chat.
 * (checking new messages, setting the autodelete, cancelling composing etc)
 * \return ChatDlg is ready to be hidden.
 */
bool ChatDlg::readyToHide()
{
	// really lame way of checking if we are encrypting
	if (!chatEdit()->isEnabled()) {
		return false;
	}

	if (keepOpen_) {
		int n = QMessageBox::information(this, tr("Warning"), tr("A new chat message was just received.\nDo you still want to close the window?"), tr("&Yes"), tr("&No"));
		if (n != 0) {
			return false;
		}
	}

	// destroy the dialog if delChats is dcClose
	if (option.delChats == dcClose) {
		setAttribute(Qt::WA_DeleteOnClose);
	}
	else {
		if (option.delChats == dcHour) {
			setSelfDestruct(60);
		}
		else if (option.delChats == dcDay) {
			setSelfDestruct(60 * 24);
		}
	}

	// Reset 'contact is composing' & cancel own composing event
	resetComposing();
	setChatState(StateGone);
	if (contactChatState_ == StateComposing || contactChatState_ == StateInactive) {
		setContactChatState(StatePaused);
	}

	if (pending_ > 0) {
		pending_ = 0;
		messagesRead(jid());
		updateCaption();
	}
	doFlash(false);

	chatEdit()->setFocus();
	return true;
}

void ChatDlg::capsChanged(const Jid& j)
{
	if (jid().compare(j, false)) {
		capsChanged();
	}
}

void ChatDlg::capsChanged()
{
}

void ChatDlg::hideEvent(QHideEvent *)
{
	if (isMinimized()) {
		resetComposing();
		setChatState(StateInactive);
	}
}

void ChatDlg::showEvent(QShowEvent *)
{
	setSelfDestruct(0);
}

void ChatDlg::windowActivationChange(bool oldstate)
{
	QWidget::windowActivationChange(oldstate);

	// if we're bringing it to the front, get rid of the '*' if necessary
	if (isActiveTab()) {
		activated();
	}
}

void ChatDlg::logSelectionChanged()
{
#ifdef Q_WS_MAC
	// A hack to only give the message log focus when text is selected
	if (chatView()->hasSelectedText()) {
		chatView()->setFocus();
	}
	else {
		chatEdit()->setFocus();
	}
#endif
}

void ChatDlg::activated()
{
	if (pending_ > 0) {
		pending_ = 0;
		messagesRead(jid());
		updateCaption();
	}
	doFlash(false);

	chatEdit()->setFocus();
}

void ChatDlg::dropEvent(QDropEvent* event)
{
	QStringList l;
	if (account()->loggedIn() && Q3UriDrag::decodeLocalFiles(event, l) && !l.isEmpty()) {
		account()->actionSendFiles(jid(), l);
	}
}

void ChatDlg::dragEnterEvent(QDragEnterEvent* event)
{
	QStringList l;
	event->accept(account()->loggedIn() && Q3UriDrag::canDecode(event) && Q3UriDrag::decodeLocalFiles(event, l) && !l.isEmpty());
}


Jid ChatDlg::jid() const
{
	return jid_;
}

void ChatDlg::setJid(const Jid &j)
{
	if (!j.compare(jid())) {
		account()->dialogUnregister(this);
		jid_ = j;
		account()->dialogRegister(this, jid());
		updateContact(jid(), false);
	}
}

const QString& ChatDlg::getDisplayName()
{
	return dispNick_;
}

QSize ChatDlg::defaultSize()
{
	return QSize(320, 280);
}

struct UserStatus {
	UserStatus()
			: userListItem(0)
			, statusType(XMPP::Status::Offline) {}
	UserListItem* userListItem;
	XMPP::Status::Type statusType;
	QString status;
	QString publicKeyID;
};
  
UserStatus userStatusFor(const Jid& jid, QList<UserListItem*> ul, bool forceEmptyResource)
{
	if (ul.isEmpty())
		return UserStatus();

	UserStatus u;

	u.userListItem = ul.first();
	if (jid.resource().isEmpty() || forceEmptyResource) {
		// use priority
		if (u.userListItem->isAvailable()) {
			const UserResource &r = *u.userListItem->userResourceList().priority();
			u.statusType = r.status().type();
			u.status = r.status().status();
			u.publicKeyID = r.publicKeyID();
		}
	}
	else {
		// use specific
		UserResourceList::ConstIterator rit = u.userListItem->userResourceList().find(jid.resource());
		if (rit != u.userListItem->userResourceList().end()) {
			u.statusType = (*rit).status().type();
			u.status = (*rit).status().status();
			u.publicKeyID = (*rit).publicKeyID();
		}
	}

	if (u.statusType == XMPP::Status::Offline)
		u.status = u.userListItem->lastUnavailableStatus().status();

	return u;
}

void ChatDlg::updateContact(const Jid &j, bool fromPresence)
{
	// if groupchat, only update if the resource matches
	if (account()->findGCContact(j) && !jid().compare(j)) {
		return;
	}

	if (jid().compare(j, false)) {
		QList<UserListItem*> ul = account()->findRelevant(j);
		UserStatus userStatus = userStatusFor(jid(), ul, false);
		if (userStatus.statusType == XMPP::Status::Offline)
			contactChatState_ = StateNone;

		bool statusChanged = false;
		if (status_ != userStatus.statusType || statusString_ != userStatus.status) {
			statusChanged = true;
			status_ = userStatus.statusType;
			statusString_ = userStatus.status;
		}

		contactUpdated(userStatus.userListItem, userStatus.statusType, userStatus.status);

		if (userStatus.userListItem) {
			dispNick_ = JIDUtil::nickOrJid(userStatus.userListItem->name(), userStatus.userListItem->jid().full());
			nicksChanged();
			updateCaption();

			key_ = userStatus.publicKeyID;
			updatePGP();

			if (fromPresence && statusChanged) {
				QString msg = tr("%1 is %2").arg(Qt::escape(dispNick_)).arg(status2txt(status_));
				if (!statusString_.isEmpty()) {
					QString ss = TextUtil::linkify(TextUtil::plain2rich(statusString_));
					if (option.useEmoticons) {
						ss = TextUtil::emoticonify(ss);
					}
					if (PsiOptions::instance()->getOption("options.ui.chat.legacy-formatting").toBool()) {
						ss = TextUtil::legacyFormat(ss);
					}
					msg += QString(" [%1]").arg(ss);
				}
				appendSysMsg(msg);
			}
		}

		// Update capabilities
		capsChanged(jid());

		// Reset 'is composing' event if the status changed
		if (statusChanged && contactChatState_ != StateNone) {
 			if (contactChatState_ == StateComposing || contactChatState_ == StateInactive) {
				setContactChatState(StatePaused);
			}
		}
	}
}

void ChatDlg::contactUpdated(UserListItem* u, int status, const QString& statusString)
{
	Q_UNUSED(u);
	Q_UNUSED(status);
	Q_UNUSED(statusString);
}

void ChatDlg::doVoice()
{
	aVoice(jid());
}

void ChatDlg::updateAvatar(const Jid& j)
{
	if (j.compare(jid(), false))
		updateAvatar();
}

void ChatDlg::setLooks()
{
	// update the font
	QFont f;
	f.fromString(option.font[fChat]);
	chatView()->setFont(f);
	chatEdit()->setFont(f);

	// update contact info
	status_ = -2; // sick way of making it redraw the status
	updateContact(jid(), false);

	// update the widget icon
#ifndef Q_WS_MAC
	setWindowIcon(IconsetFactory::icon("psi/start-chat").icon());
#endif

	/*QBrush brush;
	brush.setPixmap( QPixmap( option.chatBgImage ) );
	chatView()->setPaper(brush);
	chatView()->setStaticBackground(true);*/
 
	setWindowOpacity(double(qMax(MINIMUM_OPACITY, PsiOptions::instance()->getOption("options.ui.chat.opacity").toInt())) / 100);
}

void ChatDlg::optionsUpdate()
{
	setLooks();
	setShortcuts();

	if (isHidden()) {
		if (option.delChats == dcClose) {
			deleteLater();
			return;
		}
		else {
			if (option.delChats == dcHour) {
				setSelfDestruct(60);
			}
			else if (option.delChats == dcDay) {
				setSelfDestruct(60 * 24);
			}
			else {
				setSelfDestruct(0);
			}
		}
	}
}
  
void ChatDlg::updatePGP()
{
}

void ChatDlg::doInfo()
{
	aInfo(jid());
}

void ChatDlg::doHistory()
{
	aHistory(jid());
}

void ChatDlg::doFile()
{
	aFile(jid());
}
  
void ChatDlg::doClear()
{
	chatView()->clear();
}

void ChatDlg::setKeepOpenFalse()
{
	keepOpen_ = false;
}

void ChatDlg::setWarnSendFalse()
{
	warnSend_ = false;
}

void ChatDlg::setSelfDestruct(int minutes)
{
	if (minutes <= 0) {
		if (selfDestruct_) {
			delete selfDestruct_;
			selfDestruct_ = 0;
		}
		return;
	}

	if (!selfDestruct_) {
		selfDestruct_ = new QTimer(this);
		connect(selfDestruct_, SIGNAL(timeout()), SLOT(deleteLater()));
	}
 
	selfDestruct_->start(minutes * 60000);
}

void ChatDlg::updateCaption()
{
	QString cap = "";

	if (pending_ > 0) {
		cap += "* ";
		if (pending_ > 1) {
			cap += QString("[%1] ").arg(pending_);
		}
	}
	cap += dispNick_;

	if (contactChatState_ == StateComposing) {
		cap = tr("%1 (Composing ...)").arg(cap);
	}
	else if (contactChatState_ == StateInactive) {
		cap = tr("%1 (Inactive)").arg(cap);
	}

	setWindowTitle(cap);

	emit captionChanged(cap);
	emit unreadEventUpdate(pending_);
}

void ChatDlg::toggleItalic()
{
	if(!chatEdit()->isEnabled())
		return;
	
	chatEdit()->setFontItalic(!chatEdit()->fontItalic());
}

void ChatDlg::toggleBold()
{
	if(!chatEdit()->isEnabled())
		return;
	chatEdit()->setFontWeight((chatEdit()->fontWeight() == QFont::Normal) ? QFont::Bold : QFont::Normal);
}

void ChatDlg::toggleUnderline()
{
	if(!chatEdit()->isEnabled())
		return;
	chatEdit()->setFontUnderline(!chatEdit()->fontUnderline());
}

void ChatDlg::toggleRed()
{
	if(!chatEdit()->isEnabled())
		return;
	chatEdit()->setTextColor(QColor(255,0,0));
}

void ChatDlg::toggleGreen()
{
	if(!chatEdit()->isEnabled())
		return;
	chatEdit()->setTextColor(QColor(0,255,0));
}

void ChatDlg::toggleBlue()
{
	if(!chatEdit()->isEnabled())
		return;
	chatEdit()->setTextColor(QColor(0,0,255));
}

void ChatDlg::toggleBlack()
{
	if(!chatEdit()->isEnabled())
		return;
	chatEdit()->setTextColor(QColor(255,255,255));
}

bool ChatDlg::isEncryptionEnabled() const
{
	return false;
}
  
void ChatDlg::doSend()
{
	if (!chatEdit()->isEnabled()) {
		return;
	}

	if (chatEdit()->text().isEmpty()) {
		return;
	}

	if (chatEdit()->text() == "/clear") {
		chatEdit()->clear();
		doClear();
		return;
	}

	if (!account()->loggedIn()) {
		return;
	}

	if (warnSend_) {
		warnSend_ = false;
		int n = QMessageBox::information(this, tr("Warning"), tr(
		                                     "<p>Encryption was recently disabled by the remote contact.  "
		                                     "Are you sure you want to send this message without encryption?</p>"
		                                 ), tr("&Yes"), tr("&No"));
		if (n != 0) {
			return;
		}
	}

	Message m(jid());
	m.setType("chat");
	m.setBody(chatEdit()->text());
	m.setTimeStamp(QDateTime::currentDateTime());
	if (isEncryptionEnabled()) {
		m.setWasEncrypted(true);
	}
	QString tmp = chatEdit()->toHtml();
	int start = tmp.indexOf("<body");
	QString body = tmp.mid(start, (tmp.indexOf("</body>") - start) +7);
	QDomDocument doc;
	QString l = TextUtil::linkify(chatEdit()->text());
	int start1 = 0;
	while(1) {
		start1 = l.indexOf("<a",start1);
		if(start1 == -1)
			break;
		int start2 = l.indexOf("href=", start1 + 3) + 6;
		int end2 = l.indexOf("\"",start2);
		int end1 = l.indexOf("</a>",end2)+4;
		body = body.replace(l.mid(start2,end2-start2), l.mid(start1, end1-start1));
		start1 += end1 - start1;
	}
	doc.setContent(body);
	QDomElement e = doc.documentElement();
	HTMLElement html(e);
	m.setHTML(html);
	m.setHTMLString(body);

	m_ = m;

	// Request events
#ifdef USE_XEP0022
	if (option.messageEvents) {

		// Only request more events when really necessary
		if (sendComposingEvents_) {
			m.addEvent(ComposingEvent);
		}
		m.setChatState(StateActive);
	}
#endif

	// Update current state
	setChatState(StateActive);

	if (isEncryptionEnabled()) {
		chatEdit()->setEnabled(false);
		transid_ = account()->sendMessageEncrypted(m);
		if (transid_ == -1) {
			chatEdit()->setEnabled(true);
			chatEdit()->setFocus();
			return;
		}
	}
	else {
		aSend(m);
		doneSend();
	}
	chatEdit()->setFocus();
}

void ChatDlg::doneSend()
{
	appendMessage(m_, true);
	disconnect(chatEdit(), SIGNAL(textChanged()), this, SLOT(setComposing()));
	chatEdit()->clear();

	// Reset composing timer
	connect(chatEdit(), SIGNAL(textChanged()), this, SLOT(setComposing()));
	// Reset composing timer
	resetComposing();
}

void ChatDlg::encryptedMessageSent(int x, bool b, int e)
{
	if (transid_ == -1 || transid_ != x) {
		return;
	}
	transid_ = -1;
	if (b) {
		doneSend();
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("There was an error trying to send the message encrypted.\nReason: %1.").arg(PGPUtil::instance().messageErrorString((QCA::SecureMessage::Error) e)));
	}
	chatEdit()->setEnabled(true);
	chatEdit()->setFocus();
}
  
void ChatDlg::incomingMessage(const Message &m)
{
	if (m.body().isEmpty()) {
		// Event message
#ifdef USE_XEP0022
		if (m.containsEvent(CancelEvent)) {
			setContactChatState(StatePaused);
		}
		else if (m.containsEvent(ComposingEvent)) {
			setContactChatState(StateComposing);
		}
#endif

		if (m.chatState() != StateNone) {
  			setContactChatState(m.chatState());
  		}
	}
	else {
		// Normal message
		// Check if user requests event messages
#ifdef USE_XEP0022
		sendComposingEvents_ = m.containsEvent(ComposingEvent);
		if (!m.eventId().isEmpty()) {
			eventId_ = m.eventId();
		}
		if (m.containsEvents() || m.chatState() != StateNone) {
#else
		if (m.chatState() != StateNone) {
#endif
			setContactChatState(StateActive);
		}
		else {
			setContactChatState(StateNone);
		}
		if(m.ampRules()->isEmpty())
			appendMessage(m);
		else
			appendSysMsg(m.body());
	}
}

void ChatDlg::setPGPEnabled(bool enabled)
{
	Q_UNUSED(enabled);
}

QString ChatDlg::whoNick(bool local) const
{
	QString result;

	if (local) {
		result = account()->nick();
	}
	else {
		result = dispNick_;
	}

	return Qt::escape(result);
}

void ChatDlg::appendMessage(const Message &m, bool local)
{
	// figure out the encryption state
	bool encChanged = false;
	bool encEnabled = false;
	if (lastWasEncrypted_ != m.wasEncrypted()) {
		encChanged = true;
	}
	lastWasEncrypted_ = m.wasEncrypted();
	encEnabled = lastWasEncrypted_;

	if (encChanged) {
		if (encEnabled) {
			appendSysMsg(QString("<icon name=\"psi/cryptoYes\"> ") + tr("Encryption Enabled"));
			if (!local) {
				setPGPEnabled(true);
			}
		}
		else {
			appendSysMsg(QString("<icon name=\"psi/cryptoNo\"> ") + tr("Encryption Disabled"));
			if (!local) {
				setPGPEnabled(false);

				// enable warning
				warnSend_ = true;
				QTimer::singleShot(3000, this, SLOT(setWarnSendFalse()));
			}
		}
	}
	
	QString txt = messageText(m);

	ChatDlg::SpooledType spooledType = m.spooled() ?
	                                   ChatDlg::Spooled_OfflineStorage :
	                                   ChatDlg::Spooled_None;
	if (isEmoteMessage(m))
		appendEmoteMessage(spooledType, m.timeStamp(), local, txt);
	else
		appendNormalMessage(spooledType, m.timeStamp(), local, txt);

	appendMessageFields(m);

	if (local) {
		deferredScroll();
	}

	// if we're not active, notify the user by changing the title
	if (!isActiveTab()) {
		++pending_;
		updateCaption();
		if (PsiOptions::instance()->getOption("options.ui.flash-windows").toBool()) {
			doFlash(true);
		}
		if (option.raiseChatWindow) {
			if (option.useTabs) {
 				TabDlg* tabSet = getManagingTabDlg();
				tabSet->selectTab(this);
				::bringToFront(tabSet, false);
			}
			else {
				::bringToFront(this, false);
			}
		}
	}
	//else {
	//	messagesRead(jid());
	//}

	if (!local) {
		keepOpen_ = true;
		QTimer::singleShot(1000, this, SLOT(setKeepOpenFalse()));
	}
}

void ChatDlg::deferredScroll()
{
	QTimer::singleShot(250, this, SLOT(slotScroll()));
}

void ChatDlg::slotScroll()
{
	chatView()->scrollToBottom();
}

void ChatDlg::updateIsComposing(bool b)
{
	setChatState(b ? StateComposing : StatePaused);
}

void ChatDlg::setChatState(ChatState state)
{
#ifdef USE_XEP0022
	if (option.messageEvents && (sendComposingEvents_ || (contactChatState_ != StateNone))) {
#else
	if (option.messageEvents && (contactChatState_ != StateNone)) {
#endif
		// Don't send to offline resource
		QList<UserListItem*> ul = account()->findRelevant(jid());
		if (ul.isEmpty()) {
#ifdef USE_XEP0022
			sendComposingEvents_ = false;
#endif
			lastChatState_ = StateNone;
			return;
		}

		UserListItem *u = ul.first();
		if (!u->isAvailable()) {
#ifdef USE_XEP0022
			sendComposingEvents_ = false;
#endif
			lastChatState_ = StateNone;
			return;
		}

		// Transform to more privacy-enabled chat states if necessary
		if (!option.inactiveEvents && (state == StateGone || state == StateInactive)) {
			state = StatePaused;
		}

		if (lastChatState_ == StateNone && (state != StateActive && state != StateComposing && state != StateGone)) {
			//this isn't a valid transition, so don't send it, and don't update laststate
			return;
		}
			
		// Check if we should send a message
		if (state == lastChatState_ || state == StateActive || (lastChatState_ == StateActive && state == StatePaused)) {
			lastChatState_ = state;
			return;
		}

		// Build event message
		Message m(jid());
#ifdef USE_XEP0022
		if (sendComposingEvents_) {
			m.setEventId(eventId_);
			if (state == StateComposing) {
				m.addEvent(ComposingEvent);
			}
			else if (lastChatState_ == StateComposing) {
				m.addEvent(CancelEvent);
			}
		}
#endif
		if (contactChatState_ != StateNone) {
			if (lastChatState_ != StateGone) {
				if ((state == StateInactive && lastChatState_ == StateComposing) || (state == StateComposing && lastChatState_ == StateInactive)) {
					// First go to the paused state
					Message tm(jid());
					m.setType("chat");
					m.setChatState(StatePaused);
					account()->dj_sendMessage(m, false);
				}
				m.setChatState(state);
			}
		}
		
		// Send event message
#ifdef USE_XEP0022
		if (m.containsEvents() || m.chatState() != StateNone) {
#else
		if (m.chatState() != StateNone) {
#endif
			m.setType("chat");
			account()->dj_sendMessage(m, false);
		}

		// Save last state
		if (lastChatState_ != StateGone || state == StateActive) {
			lastChatState_ = state;
		}
	}
}

void ChatDlg::setContactChatState(ChatState state)
{
	contactChatState_ = state;
	if (state == StateGone) {
		appendSysMsg(tr("%1 ended the conversation").arg(Qt::escape(dispNick_)));
		hover_->setText(tr("%1 ended the conversation").arg(Qt::escape(dispNick_)));
	}
	else {
		// Activate ourselves
		if (lastChatState_ == StateGone) {
			setChatState(StateActive);
		}
	}
	if (contactChatState_ == StateComposing)
		hover_->setText(tr("%1 (Composing ...)").arg(Qt::escape(dispNick_)));
//		tr("%1 (Composing ...)").arg(cap);
	else if (contactChatState_ == StateInactive)
		hover_->setText(tr("%1 (Inactive)").arg(Qt::escape(dispNick_)));
	else hover_->setText("");
//		cap = tr("%1 (Inactive)").arg(cap);

	emit contactStateChanged(state);
	updateCaption();
}

bool ChatDlg::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		keyPressEvent(static_cast<QKeyEvent*>(event));
		if (event->isAccepted())
			return true;
	}

	if (chatView()->handleCopyEvent(obj, event, chatEdit()))
		return true;

	return QWidget::eventFilter(obj, event);
}

void ChatDlg::addEmoticon(QString text)
{
	if (!isActiveTab())
		return;

	chatEdit()->insert(text + " ");
}
  
/**
 * Records that the user is composing
 */
void ChatDlg::setComposing()
{
	if (!composingTimer_) {
		/* User (re)starts composing */
		composingTimer_ = new QTimer(this);
		connect(composingTimer_, SIGNAL(timeout()), SLOT(checkComposing()));
		composingTimer_->start(2000); // FIXME: magic number
		emit composing(true);
	}
 	isComposing_ = true;
}
  
/**
 * Checks if the user is still composing
 */
void ChatDlg::checkComposing()
{
	if (!isComposing_) {
		// User stopped composing
		delete composingTimer_;
		composingTimer_ = 0;
		emit composing(false);
  	}
	isComposing_ = false; // Reset composing
}

void ChatDlg::resetComposing()
{
	if (composingTimer_) {
		delete composingTimer_;
		composingTimer_ = 0;
		isComposing_ = false;
	}
}

PsiAccount* ChatDlg::account() const
{
	return pa_;
}

void ChatDlg::nicksChanged()
{
	// this function is intended to be reimplemented in subclasses
}

static const QString me_cmd = "/me ";

bool ChatDlg::isEmoteMessage(const XMPP::Message& m)
{
	if (m.body().startsWith(me_cmd) || m.html().text().trimmed().startsWith(me_cmd))
		return true;

	return false;
}

QString ChatDlg::messageText(const XMPP::Message& m)
{
	bool emote = isEmoteMessage(m);
	QString txt;

	if (m.containsHTML() && PsiOptions::instance()->getOption("options.html.chat.render").toBool() && (!m.html().text().isEmpty() || !m.htmlString().isEmpty())) {
		if(!m.html().text().isEmpty())
			txt = m.html().toString("span");
		else
			txt = m.htmlString();

		if (emote) {
			int cmd = txt.indexOf(me_cmd);
			txt = txt.remove(cmd, me_cmd.length());
		}
		// qWarning("html body:\n%s\n",qPrintable(txt));
	}
	else {
		txt = m.body();

		if (emote)
			txt = txt.mid(me_cmd.length());

		txt = TextUtil::plain2rich(txt);
		txt = TextUtil::linkify(txt);
		// qWarning("regular body:\n%s\n",qPrintable(txt));
	}

	if (option.useEmoticons)
		txt = TextUtil::emoticonify(txt);
	if (PsiOptions::instance()->getOption("options.ui.chat.legacy-formatting").toBool())
		txt = TextUtil::legacyFormat(txt);
 
	return txt;
}

void ChatDlg::chatEditCreated()
{
	chatView()->setDialog(this);
	chatEdit()->setDialog(this);

	if (highlightersInstalled_) {
		connect(chatEdit(), SIGNAL(textChanged()), this, SLOT(setComposing()));
	}
}

#include "chatdlg.moc"
