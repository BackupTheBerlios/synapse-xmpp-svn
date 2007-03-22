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
#include "im.h"
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
#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
#include "SnarlInterface.h"
#include "applicationinfo.h"
#endif

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
#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
	QString caption;
	QString text;
	QString icon;
#endif
	PsiIcon *titleIcon;
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

	if ( !option.ppIsOn )
		return;

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

#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
	caption = titleText;
	icon = ApplicationInfo::homeDir() + "/iconsets/roster/default/" + icon + ".png";
#endif

	popup = new FancyPopup(0,titleText, titleIcon, 3000, false);
	connect(popup, SIGNAL(clicked(int)), SLOT(popupClicked(int)));
	connect(popup, SIGNAL(destroyed()), SLOT(popupDestroyed()));
	
	// create id
	if ( _titleIcon )
		id += _titleIcon->name();
	id += titleText;
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
			psi->processEvent( event );
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
	if ( option.ppTextClip > 0 ) {
		// richtext will give us trouble here
		if ( ((int)text.length()) > option.ppTextClip ) {
			text = text.left( option.ppTextClip );

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
	font.fromString( option.font[fPopup] );
	textLabel->setFont(font);

	textLabel->setWordWrap(false);
//	textLabel->setText(QString("<qt>%1</qt>").arg(clipText(text)));
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
#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
		d->icon = "online";
#endif
		break;
	case AlertOffline:
		text += PsiPopup::tr("Contact offline");
		icon = (PsiIcon *)IconsetFactory::iconPtr("status/offline");
#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
		d->icon = "offline";
#endif
		break;
	case AlertStatusChange:
		text += PsiPopup::tr("Status change");
		icon = (PsiIcon *)IconsetFactory::iconPtr("status/online");
#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
		d->icon = "online";
#endif
		break;
	case AlertMessage:
		text += PsiPopup::tr("Incoming message");
		icon = (PsiIcon *)IconsetFactory::iconPtr("psi/message");
		doAlertIcon = true;
#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
		d->icon = "message";
#endif
		break;
	case AlertChat:
		text += PsiPopup::tr("Incoming chat message");
		icon= (PsiIcon *)IconsetFactory::iconPtr("psi/chat");
		doAlertIcon = true;
#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
		d->icon = "chat";
#endif
		break;
	case AlertHeadline:
		text += PsiPopup::tr("Headline");
		icon= (PsiIcon *)IconsetFactory::iconPtr("psi/headline");
		doAlertIcon = true;
#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
		d->icon = "headline";
#endif
		break;
	case AlertFile:
		text += PsiPopup::tr("Incoming file");
		icon= (PsiIcon *)IconsetFactory::iconPtr("psi/file");
		doAlertIcon = true;
#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
		d->icon = "file";
#endif
		break;
	default:
		break;
	}

	d->init(icon, text, acc, doAlertIcon ? type : AlertNone);
}

void PsiPopup::setData(const PsiIcon *icon, QString text,const Jid& j) //sets layout in popup
{
	if ( !d->popup ) {
		deleteLater();
		return;
	}

	d->popup->setData(d->createContactPixmap((QPixmap*)&icon->pixmap(),j), d->createContactInfo(icon, text));
	// update id
	if ( icon )
		d->id += icon->name();
	d->id += text;

#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
	d->text = TextUtil::rich2plain(text);
#endif
	show();
}

void PsiPopup::setData(const Jid &j, const Resource &r, const UserListItem *u, const PsiEvent *event)
{
	if ( !d->popup ) {
		deleteLater();
		return;
	}

	d->jid    = j;
	d->status = r.status();
	d->event  = (PsiEvent *)event;

	if ( event )
		connect(event, SIGNAL(destroyed()), d, SLOT(eventDestroyed()));

	PsiIcon *icon = PsiIconset::instance()->statusPtr(j, r.status());
	QString text;

	QString jid = j.full();
	if ( option.ppJidClip > 0 && ((int)jid.length()) > option.ppJidClip )
		jid = jid.left( option.ppJidClip ) + "...";

	QString status;
	if ( option.ppStatusClip != 0 )
		status = r.status().status();
	if ( option.ppStatusClip > 0 )
		if ( ((int)status.length()) > option.ppStatusClip )
			status = status.left ( option.ppStatusClip ) + "...";

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
		if ( !option.ppJidClip )
			name = "<nobr>" + Qt::escape(name) + "</nobr>";
		else
			name = "<nobr>" + Qt::escape(name) + " &lt;" + Qt::escape(jid) + "&gt;" + "</nobr>";
	}
	else 
		name = "<nobr>&lt;" + Qt::escape(jid) + "&gt;</nobr>";

	QString statusString = TextUtil::plain2rich(status);
	if ( option.useEmoticons )
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
	if ( d->popupType != AlertHeadline && (d->popupType != AlertFile || !option.popupFiles) )
		setData(icon, contactText,j);
	else if ( d->popupType == AlertHeadline ) {
		const Message *jmessage = &((MessageEvent *)event)->message();
		QString message;
		if ( !jmessage->subject().isEmpty() )
			message += "<font color=\"red\"><b>" + tr("Subject:") + " " + jmessage->subject() + "</b></font><br>";
		message += TextUtil::plain2rich( jmessage->body() );

/*		QLabel *messageLabel = new QLabel(d->popup);
		QFont font = messageLabel->font();
		font.setPointSize(option.smallFontSize);
		messageLabel->setFont(font);

		messageLabel->setWordWrap(true);
		messageLabel->setTextFormat(Qt::RichText);
		messageLabel->setText( d->clipText(TextUtil::linkify( message )) );
		messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		vbox->addWidget(messageLabel);

		// update id
		if ( icon )
			d->id += icon->name();
		d->id += contactText;
		d->id += message;

		d->popup->addLayout( vbox );*/
		contactText += "<br/>" + message;
		setData((const PsiIcon*)icon, (QString)contactText, j);

		show();
	}
}

void PsiPopup::show()
{
	if ( !d->popup ) {
		deleteLater();
		return;
	}

	if ( !d->id.isEmpty() /*&& option.ppNoDupes*/ ) {
		foreach (PsiPopup *pp, *psiPopupList) {
			if ( d->id == pp->id() && pp->popup() ) {
				pp->popup()->restartHideTimer();

				d->display = false;
				break;
			}
		}
	}

	if ( d->display ) {
#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
		SnarlInterface snarl;
		if(snarl.snShowMessage(d->caption.toStdString(), d->text.toStdString(), 5, d->icon.toStdString(), 0, 0) == 0) {
#endif
		psiPopupList->append( this );
		d->popup->show();
#if defined(Q_WS_WIN) && defined(HAVE_SNARL)
		}
#endif
	}
	else {
		deleteLater();
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
