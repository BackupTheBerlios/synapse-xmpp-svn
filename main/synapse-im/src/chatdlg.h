/*
 * chatdlg.h - dialog for handling chats
 * Copyright (C) 2001, 2002  Justin Karneges
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

#ifndef CHATDLG_H
#define CHATDLG_H

#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QDropEvent>
#include <QCloseEvent>

#include "advwidget.h"
#include "xmpp_chatstate.h" 

namespace XMPP {
	class Jid;
	class Message;
}
using namespace XMPP;

class PsiAccount;
class UserListItem;
class QDropEvent;
class QDragEnterEvent;

#include "ui_chatdlg.h"

class RichStatus : public QWidget
{
	Q_OBJECT
public:
	RichStatus(QWidget *parent);
	~RichStatus();

	void setStatusString(QString *txt, int width);
	bool setPEP(QString *pep,int width);
	void paintEvent(QPaintEvent *pe);
private:
	QTextDocument *v_rs;
	QString txt_;
	QString pep_;
};

class ChatDlg : public AdvancedWidget<QWidget>
{
	Q_OBJECT
public:
	ChatDlg(const Jid &, PsiAccount *);
	~ChatDlg();

	const Jid & jid() const;
	void setJid(const Jid &);
	const QString & getDisplayNick();

	void updateOtr();
	void updateSave(bool on);

	static QSize defaultSize();
	bool readyToHide();

signals:
	void aInfo(const Jid &);
	void aHistory(const Jid &);
	void aVoice(const Jid &);
	void aOtr(const XMPP::Jid &);
	void messagesRead(const Jid &);
	void aSend(const Message &);
	void aFile(const Jid &);
	void captionChanged(ChatDlg*);
	void contactStateChanged( XMPP::ChatState );
	void unreadMessageUpdate(ChatDlg*, int);

protected:
	void setShortcuts();

	// reimplemented
	void keyPressEvent(QKeyEvent *);
	void closeEvent(QCloseEvent *);
	void resizeEvent(QResizeEvent *);
	void hideEvent(QHideEvent *);
	void showEvent(QShowEvent *);
	void windowActivationChange(bool);
	void dropEvent(QDropEvent* event);
	void dragEnterEvent(QDragEnterEvent* event);
	
	bool eventFilter(QObject *obj, QEvent *event);

public slots:
	void optionsUpdate();
	void updateContact(const Jid &, bool);
	void incomingMessage(const Message &);
	void activated();
	void updateAvatar();
	void updateAvatar(const Jid&);
	void updatePEP();
	void resizeToolBox(QSize size);

private slots:
	void scrollUp();
	void scrollDown();
	void doInfo();
	void doHistory();
	void doClear();
	void doClearButton();
//--- XHTML-IM Formating
	void toggleItalic();
	void toggleBold();
	void toggleUnderline();

	void toggleRed();
	void toggleGreen();
	void toggleBlue();
	void toggleBlack();
//----------------------
	void doSend();
	void doVoice();
	void doOtr();
	void doFile();
	void setKeepOpenFalse();
	void setWarnSendFalse();
	void updatePGP();
	void encryptedMessageSent(int, bool, int);
	void slotScroll();
	void setChatState(XMPP::ChatState s);
	void updateIsComposing(bool);
	void setContactChatState(ChatState s);
	void toggleSmallChat();
	void toggleEncryption();
	void buildMenu();
	void logSelectionChanged();
	void capsChanged(const Jid&);
	void updateIdentityVisibility();
	void chatEditCreated();
	void initComposing();

	void showToolBox();

public:
	class Private;
private:
	Private *d;
	Ui::ChatDlg ui_;
	bool highlightersInstalled_;

	void contextMenuEvent(QContextMenuEvent *);

	void doneSend();
	void setLooks();
	void setSelfDestruct(int);
	void updateCaption();
	void deferredScroll();

	void appendMessage(const Message &, bool local=false);
	void appendSysMsg(const QString &);
};

#endif
