/*
 * psipopup.cpp - the Psi passive popup class
 * Copyright (C) 2003  Michail Pishchagin
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

#include "psipopup.h"
#include "common.h"
#include "fancypopup.h"
#include "fancylabel.h"
#include "userlist.h"
#include "alerticon.h"
#include "psievent.h"
#include "psicon.h"
#include "textutil.h"
#include "psiaccount.h"
#include "psiiconset.h"
#include "iconlabel.h"
#include "psioptions.h"

#include <qapplication.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <Q3PtrList>
#include <QBoxLayout>
#include <QList>
#include <QTextDocument>
#include <QSystemTrayIcon>
#include "applicationinfo.h"
#include "mainwin.h"

#include "avatars.h"
/**
 * Limits number of popups that could be displayed 
 * simultaneously on screen. Old popups momentally
 * disappear when new ones appear.
 */
static int MaxPopups = 5;

//#include "private/qlabel_p.h"
#include "psirichlabel.h"

/**
 * Holds a list of Psi Popups.
 */
static QList<PsiPopup *> *psiPopupList = 0;

//----------------------------------------------------------------------------
// PsiPopup::Private
//----------------------------------------------------------------------------

class PsiPopup::Private : public QObject
{
	Q_OBJECT
public:
	Private(PsiPopup *p);
	~Private();

	void init(const PsiIcon *titleIcon, QString titleText, PsiAccount *_acc, PopupType type);
	QString clipText(QString);
	QPixmap createContactPixmap(const QPixmap *icon,const Jid& j);
	PsiRichLabel *createContactInfo(const PsiIcon *icon, QString text);

private slots:
	void popupDestroyed();
	void popupClicked(int);
	void eventDestroyed();

public:
	PsiCon *psi;
	PsiAccount *account;
	FancyPopup *popup;
	PsiPopup *psiPopup;
	QString id;
	PopupType popupType;
	Jid jid;
	Status status;
	PsiEvent *event;
	PsiIcon *titleIcon;
	QString titleText_;
	QString name_;
	bool display;
};

PsiPopup::Private::Private(PsiPopup *p)
{
	psiPopup = p;
	popup = 0;
	popupType = AlertNone;
	event = 0;
	titleIcon = 0;
}

PsiPopup::Private::~Private()
{
	if ( psiPopupList )
		psiPopupList->removeAll(psiPopup);

	if ( popup )
		delete popup;
	if ( titleIcon )
		delete titleIcon;
	popup = 0;
}

void PsiPopup::Private::init(const PsiIcon *_titleIcon, QString titleText, PsiAccount *acc, PopupType type)
{
	psi = acc->psi();
	account = acc;
	display = true;
	if ( !PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.enabled").toBool() )
		return;
	if(PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.type").toString() == "Full") {

	if ( !psiPopupList )
		psiPopupList = new QList<PsiPopup *>();

	if ( psiPopupList->count() >= MaxPopups && MaxPopups > 0 )
		delete psiPopupList->first();

	FancyPopup *lastPopup = 0;
	if ( psiPopupList->count() && psiPopupList->last() )
		lastPopup = psiPopupList->last()->popup();

	if ( type != AlertNone )
		titleIcon = new AlertIcon(_titleIcon);
	else
		titleIcon = new PsiIcon(*_titleIcon);

	popup = new FancyPopup();
	popup->setData(titleText, PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.place").toInt(), PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.timeout").toInt());
	connect(popup, SIGNAL(clicked(int)), SLOT(popupClicked(int)));
	connect(popup, SIGNAL(destroyed()), SLOT(popupDestroyed()));
	
	// create id
	if ( _titleIcon )
		id += _titleIcon->name();
	id += titleText;
	} else if (PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.type").toString() == "Basic")
	{
		printf("basic\n");
		titleText_ = titleText;
	}
}

void PsiPopup::Private::popupDestroyed()
{
	popup = 0;
	psiPopup->deleteLater();
}

void PsiPopup::Private::popupClicked(int button)
{
	if ( button == (int)Qt::LeftButton ) {
		if ( event )
			psi->processEvent(event, UserAction);
		else if ( account ) {
			// FIXME: it should work in most cases, but
			// maybe it's better to fix UserList::find()?
			Jid j( jid.userHost() );
			account->actionDefault( j );
		}
	}
}

void PsiPopup::Private::eventDestroyed()
{
	popup->deleteLater();
	event = 0;
}

QString PsiPopup::Private::clipText(QString text)
{
	if ( PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.maximum-text-length").toInt() > 0 ) {
		// richtext will give us trouble here
		if ( ((int)text.length()) > PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.maximum-text-length").toInt() ) {
			text = text.left( PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.maximum-text-length").toInt() );

			// delete last unclosed tag
			/*if ( text.find("</") > text.find(">") ) {

				text = text.left( text.find("</") );
			}*/

			text += "...";
		}
	}

	return text;
}

QPixmap PsiPopup::Private::createContactPixmap(const QPixmap *icon,const Jid& j)
{
	QPixmap px = account->avatarFactory()->getAvatar(j);
	int avatar_x = px.width();
	int avatar_y = px.height();
	if((avatar_x != 0 )&&( avatar_y !=0))
	{
		int avatar_size = PsiOptions::instance()->getOption("options.ui.contactlist.avatar.size").toInt();
		int x = (avatar_x > avatar_y)? avatar_size : ((avatar_x/avatar_y)*avatar_size);
		int y = (avatar_x > avatar_y)? ((avatar_y/avatar_x)*avatar_size) : avatar_size;
		px = px.scaled(x,y);
		avatar_x = px.width();
		avatar_y = px.height();
		QPainter p(&px);
		p.translate(avatar_x-icon->width(),avatar_y-icon->height());
		QRect ri(0,0,icon->width(),icon->height());
		p.drawPixmap(ri,*icon);
		return px;
	} else {
		return *icon;
	}
}

PsiRichLabel *PsiPopup::Private::createContactInfo(const PsiIcon *icon, QString text)
{
	PsiRichLabel *textLabel = new PsiRichLabel(QString("<qt>%1</qt>").arg(clipText(text)),0);
	QFont font;
	font.fromString( PsiOptions::instance()->getOption("options.ui.look.font.passive-popup").toString() );
	textLabel->setFont(font);

	textLabel->setWordWrap(false);
	textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	return textLabel;
}

//----------------------------------------------------------------------------
// PsiPopup
//----------------------------------------------------------------------------

PsiPopup::PsiPopup(const PsiIcon *titleIcon, QString titleText, PsiAccount *acc)
{
	d = new Private(this);
	d->init(titleIcon, titleText, acc, AlertNone);
}

PsiPopup::~PsiPopup()
{
	delete d;
}

PsiPopup::PsiPopup(PopupType type, PsiAccount *acc)
{
	d = new Private(this);

	d->popupType = type;
	PsiIcon *icon = 0;
	QString text = "Synapse-IM: ";
	bool doAlertIcon = false;

	switch(type) {
	case AlertOnline:
		text += PsiPopup::tr("Contact online");
		icon = (PsiIcon *)IconsetFactory::iconPtr("status/online");
		break;
	case AlertOffline:
		text += PsiPopup::tr("Contact offline");
		icon = (PsiIcon *)IconsetFactory::iconPtr("status/offline");
		break;
	case AlertStatusChange:
		text += PsiPopup::tr("Status change");
		icon = (PsiIcon *)IconsetFactory::iconPtr("status/online");
		break;
	case AlertMessage:
		text += PsiPopup::tr("Incoming message");
		icon = (PsiIcon *)IconsetFactory::iconPtr("psi/message");
		doAlertIcon = true;
		break;
	case AlertChat:
		text += PsiPopup::tr("Incoming chat message");
		icon= (PsiIcon *)IconsetFactory::iconPtr("psi/chat");
		doAlertIcon = true;
		break;
	case AlertHeadline:
		text += PsiPopup::tr("Headline");
		icon= (PsiIcon *)IconsetFactory::iconPtr("psi/headline");
		doAlertIcon = true;
		break;
	case AlertFile:
		text += PsiPopup::tr("Incoming file");
		icon= (PsiIcon *)IconsetFactory::iconPtr("psi/file");
		doAlertIcon = true;
		break;
	default:
		break;
	}

	d->init(icon, text, acc, doAlertIcon ? type : AlertNone);
}

void PsiPopup::setData(const PsiIcon *icon, QString text,const Jid& j) //sets layout in popup
{
	d->jid = j;
	if(PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.type").toString() == "Full") {
	if ( !d->popup ) {
		deleteLater();
		return;
	}

	// update id
	if ( icon )
		d->id += icon->name();
	d->id += text;
	d->popup->setData(d->createContactPixmap((QPixmap*)&icon->pixmap(),j), d->createContactInfo(icon, text));

//	show();
	} else if (PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.type").toString() == "Basic") {
		show();
	}
}

void PsiPopup::setData(const Jid &j, const Resource &r, const UserListItem *u, const PsiEvent *event)
{
	d->jid    = j;
	d->name_ = (u && !u->name().isEmpty()) ? u->name() : j.bare();
	if(PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.type").toString() == "Full") {

	if ( !d->popup ) {
		deleteLater();
		return;
	}

	d->status = r.status();
	d->event  = (PsiEvent *)event;

	if ( event )
		connect(event, SIGNAL(destroyed()), d, SLOT(eventDestroyed()));

	PsiIcon *icon = PsiIconset::instance()->statusPtr(j, r.status());
	QString text;

	QString jid = j.full();
	if ( PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.maximum-jid-length").toInt() > 0 && ((int)jid.length()) > PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.maximum-jid-length").toInt() )
		jid = jid.left( PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.maximum-jid-length").toInt() ) + "...";

	QString status;
	if ( PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.maximum-status-length").toInt() != 0 )
		status = r.status().status();
	if ( PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.maximum-status-length").toInt() > 0 )
		if ( ((int)status.length()) > PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.maximum-status-length").toInt() )
			status = status.left ( PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.maximum-status-length").toInt() ) + "...";

	QString name;
	if ( u && !u->name().isEmpty() ) {
		name = u->name();
	}
	else if (event && event->type() == PsiEvent::Auth) {
		name = ((AuthEvent*) event)->nick();
	}
	else if (event && event->type() == PsiEvent::Message) {
		name = ((MessageEvent*) event)->nick();
	}
		
	if (!name.isEmpty()) {
		if ( !PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.maximum-jid-length").toInt() )
			name = "<nobr>" + Qt::escape(name) + "</nobr>";
		else
			name = "<nobr>" + Qt::escape(name) + " &lt;" + Qt::escape(jid) + "&gt;" + "</nobr>";
	}
	else 
		name = "<nobr>&lt;" + Qt::escape(jid) + "&gt;</nobr>";

	QString statusString = TextUtil::plain2rich(status);
	if ( PsiOptions::instance()->getOption("options.ui.emoticons.use-emoticons").toBool() )
		statusString = TextUtil::emoticonify(statusString);
	if( PsiOptions::instance()->getOption("options.ui.chat.legacy-formatting").toBool() )
		statusString = TextUtil::legacyFormat(statusString);

	if ( !statusString.isEmpty() )
		statusString = "<br>" + statusString;

	QString contactText = "<nobr><font size=\"+1\">" + name + "</font></nobr>" + statusString;

	// hack for duplicate "Contact Online"/"Status Change" popups
	foreach (PsiPopup *pp, *psiPopupList) {
		if ( d->jid.full() == pp->d->jid.full() && d->status.show() == pp->d->status.show() && d->status.status() == d->status.status() ) {
			if ( d->popupType == AlertStatusChange && pp->d->popupType == AlertOnline ) {
				d->display = false;
				deleteLater();
				break;
			}
		}
	}

	// show popup
	if ( d->popupType != AlertHeadline && (d->popupType != AlertFile || !PsiOptions::instance()->getOption("options.ui.file-transfer.auto-popup").toBool()) )
		setData(icon, contactText,j);
	else if ( d->popupType == AlertHeadline ) {
		const Message *jmessage = &((MessageEvent *)event)->message();
		QString message;
		if ( !jmessage->subject().isEmpty() )
			message += "<font color=\"red\"><b>" + tr("Subject:") + " " + jmessage->subject() + "</b></font><br>";
		message += TextUtil::plain2rich( jmessage->body() );

		contactText += "<br/>" + message;
		setData((const PsiIcon*)icon, (QString)contactText, j);

		show();
	}
	} else if (PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.type").toString() == "Basic") {
		show();
	}
}

void PsiPopup::show()
{
	if(PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.type").toString() == "Full") {
	if ( !d->popup ) {
		deleteLater();
		return;
	}

	if ( !d->id.isEmpty() /*&& LEGOPTS.ppNoDupes*/ ) {
		foreach (PsiPopup *pp, *psiPopupList) {
			if ( d->id == pp->id() && pp->popup() ) {
//				pp->popup()->restartHideTimer();

				d->display = false;
				break;
			}
		}
	}
	if ( d->display ) {
		psiPopupList->append( this );
		d->popup->show();
	}
	else {
		deleteLater();
	}

	} else if (PsiOptions::instance()->getOption("options.ui.notifications.passive-popups.type").toString() == "Basic") {
		printf("basic\n");
		d->psi->mainWin()->showMessage(d->titleText_, d->name_/*jid.bare()*/,QSystemTrayIcon::MessageIcon(0), 5000);
	}
}

QString PsiPopup::id() const
{
	return d->id;
}

FancyPopup *PsiPopup::popup()
{
	return d->popup;
}

void PsiPopup::deleteAll()
{
	if ( !psiPopupList )
		return;

	psiPopupList->clear();
	delete psiPopupList;
	psiPopupList = 0;
}

#include "psipopup.moc"
