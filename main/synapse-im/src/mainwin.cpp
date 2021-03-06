/*
 * mainwin.cpp - the main window.  holds contactlist and buttons.
 * Copyright (C) 2001-2003  Justin Karneges, Michail Pishchagin
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

#include "mainwin.h"

#include <QDesktopServices>
#include <qmessagebox.h>
#include <qicon.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qobject.h>
#include <qpainter.h>
#include <qsignalmapper.h>
#include <qmenubar.h>
#include <QPixmap>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QVBoxLayout>
#include <QMenu>
#include <QMenuItem>
#include <QStyleFactory>

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include "common.h"
#include "showtextdlg.h"
#include "psicon.h"
#include "hoverlabel.h"
#include "psiiconset.h"
#include "serverinfomanager.h"
#include "applicationinfo.h"
#include "psiaccount.h"
#include "psitrayicon.h"
#include "psitoolbar.h"
#include "aboutdlg.h"
#include "psitoolbar.h"
#include "psipopup.h"
#include "psioptions.h"
#include "tipdlg.h"
#include "tunecontroller.h"
#include "mucjoindlg.h"
#include "psicontactlist.h"

#include "Model.h"
#include "View.h"
#include "Delegate.h"

#include "mainwin_p.h"
#include "mooddlg.h"
using namespace XMPP;

// deletes submenus in a popupmenu
/*void qpopupmenuclear(QMenu *p)
{
	while(p->count()) {
		QMenuItem *item = p->findItem(p->idAt(0));
		QMenu *popup = item->menu();
		p->removeItemAt(0);

		if(popup)
			delete popup;
	}
}*/

	
//----------------------------------------------------------------------------
// MainWin::Private
//----------------------------------------------------------------------------

class MainWin::Private
{
public:
	Private(PsiCon *, MainWin *);
	~Private();

	QVBoxLayout *vb_main;
	bool onTop, asTool;
	QMenu *mainMenu, *statusMenu, *statusLastMenu, *optionsMenu, *toolsMenu;
	int sbState;
	QString nickname;
	PsiTrayIcon *tray;
	QMenu *trayMenu;
	QString statusTip;

	QComboBox *cb_search;
	QString searchString,searchString2;
	bool blockString;

	PopupAction *optionsButton, *statusButton;
	IconActionGroup *statusGroup;
	HoverLabel *eventNotifier;
	PsiCon *psi;
	MainWin *mainWin;

	QSignalMapper *statusMapper;
	QSignalMapper *statusLastMapper;

	PsiIcon *nextAnim;
	int nextAmount;

	QMap<QAction *, int> statusActions;
	QAction *statusLastAction[5];

	int lastStatus;

	void registerActions();
	IconAction *getAction( QString name );
	void updateMenu(QStringList actions, QMenu *menu);
};

MainWin::Private::Private(PsiCon *_psi, MainWin *_mainWin)
{
	statusLastMenu = 0;
	psi = _psi;
	mainWin = _mainWin;

	statusGroup   = (IconActionGroup *)getAction("status_all");

	optionsButton = (PopupAction *)getAction("button_options");
	statusButton  = (PopupAction *)getAction("button_status");

	statusMapper = new QSignalMapper(mainWin, "statusMapper");
	mainWin->connect(statusMapper, SIGNAL(mapped(int)), mainWin, SLOT(activatedStatusAction(int)));

	statusLastMapper = new QSignalMapper();
	mainWin->connect(statusLastMapper, SIGNAL(mapped(int)), mainWin, SLOT(activatedStatusLastAction(int)));
}

MainWin::Private::~Private()
{
}

void MainWin::Private::registerActions()
{
	struct {
		const char *name;
		int id;
	} statuslist[] = {
		{ "status_chat",      STATUS_CHAT      },
		{ "status_online",    STATUS_ONLINE    },
		{ "status_away",      STATUS_AWAY      },
		{ "status_xa",        STATUS_XA        },
		{ "status_dnd",       STATUS_DND       },
		{ "status_offline",   STATUS_OFFLINE   },
		{ "", 0 }
	};

	int i;
	QString aName;
	for ( i = 0; !(aName = QString(statuslist[i].name)).isEmpty(); i++ ) {
		IconAction *action = getAction( aName );
		connect (action, SIGNAL(activated()), statusMapper, SLOT(map()));

		statusMapper->setMapping(action, statuslist[i].id);
		statusActions[action] = statuslist[i].id;
	}

	// register all actions
	PsiActionList::ActionsType type = PsiActionList::ActionsType( PsiActionList::Actions_MainWin | PsiActionList::Actions_Common );
	ActionList actions = psi->actionList()->suitableActions( type );
	QStringList names = actions.actions();
	QStringList::Iterator it = names.begin();
	for ( ; it != names.end(); ++it ) {
		IconAction *action = actions.action( *it );
		if ( action )
			mainWin->registerAction( action );
	}
}

IconAction *MainWin::Private::getAction( QString name )
{
	PsiActionList::ActionsType type = PsiActionList::ActionsType( PsiActionList::Actions_MainWin | PsiActionList::Actions_Common );
	ActionList actions = psi->actionList()->suitableActions( type );
	IconAction *action = actions.action( name );

	if ( !action )
		qWarning("MainWin::Private::getAction(): action %s not found!", name.latin1());
	//else
	//	mainWin->registerAction( action );

	return action;
}

void MainWin::Private::updateMenu(QStringList actions, QMenu *menu)
{
	menu->clear();

	IconAction *action;
	foreach (QString name, actions) {
		// workind around Qt/X11 bug, which displays
		// actions's text and the separator bar in Qt 4.1.1
		if ( name == "separator" ) {
			menu->insertSeparator();
			continue;
		}

		if ( name == "diagnostics" ) {
			QMenu *diagMenu = new QMenu(mainWin);
			menu->insertItem(tr("Diagnostics"), diagMenu);
			getAction("help_diag_qcaplugin")->addTo(diagMenu);
			getAction("help_diag_qcakeystore")->addTo(diagMenu);
			continue;
		}
		
		if ( (action = getAction(name)) )
			action->addTo(menu);
	}
}

//----------------------------------------------------------------------------
// MainWin
//----------------------------------------------------------------------------

//#ifdef Q_WS_X11
//#define TOOLW_FLAGS WStyle_Customize
//#else
//#define TOOLW_FLAGS ((Qt::WFlags) 0)
//#endif

#ifdef Q_WS_WIN
#define TOOLW_FLAGS (Qt::WindowMinimizeButtonHint)
#else
#define TOOLW_FLAGS ((Qt::WFlags) 0)
#endif

MainWin::MainWin(bool _onTop, bool _asTool, PsiCon *psi, const char *name)
:AdvancedWidget<QMainWindow>(0, (_onTop ? Qt::WStyle_StaysOnTop : Qt::Widget) | (_asTool ? (Qt::WStyle_Tool |TOOLW_FLAGS) : Qt::Widget))
//: Q3MainWindow(0,name,(_onTop ? Qt::WStyle_StaysOnTop : Qt::Widget) | (_asTool ? (Qt::WStyle_Tool |TOOLW_FLAGS) : Qt::Widget))
{
	setObjectName(name);
	setAttribute(Qt::WA_AlwaysShowToolTips);
  	if ( PsiOptions::instance()->getOption("options.ui.mac.use-brushed-metal-windows").toBool() ) {
		setAttribute(Qt::WA_MacMetalStyle);
	}
	d = new Private(psi, this);

#ifdef HAVE_DBUS
	dbus = new DBus(this);
// 	QDBus::sessionBus().registerObject("/Synapse-IM", app);
#endif

	setWindowIcon(PsiIconset::instance()->status(STATUS_OFFLINE).impix());

	d->onTop = _onTop;
	d->asTool = _asTool;

	// sbState:
	//   -1 : connect
	// >= 0 : STATUS_*
	d->sbState = STATUS_OFFLINE;
	d->lastStatus = -2;

	d->nextAmount = 0;
	d->nextAnim = 0;
	d->tray = 0;
	d->trayMenu = 0;
	d->statusTip = "";
	d->nickname = "";

	QString styleName = PsiOptions::instance()->getOption("options.ui.style.name").toString();
	if(!styleName.isEmpty())
		QApplication::setStyle(QStyleFactory::create(PsiOptions::instance()->getOption("options.ui.style.name").toString()));
	if(PsiOptions::instance()->getOption("options.ui.style.use-palette").toBool())
		QApplication::setPalette(QApplication::style()->standardPalette());

	QWidget *center = new QWidget (this, "Central widget");
	setCentralWidget ( center );

	d->vb_main = new QVBoxLayout(center);

	d->cb_search = new QComboBox(center);
	d->cb_search->setEditable(true);

	cvlist = new SIMContactList::View(center,d->cb_search);

	d->eventNotifier = new HoverLabel(cvlist, HoverLabel::BottomLeft);
 	connect(cvlist, SIGNAL(resizeEventNotifier(QWidget*)), d->eventNotifier,  SLOT(resizeEvent(QWidget*)));
	connect(d->eventNotifier, SIGNAL(clicked()), SLOT(doRecvNextEvent()));

	cvlist->setItemDelegate(new SIMContactList::Delegate);
	((SIMContactList::List *)d->psi->contactList())->setContactListView(cvlist);
	SIMContactList::Model *model = new SIMContactList::Model(d->psi->contactList());
	cvlist->setModel(model);
	QPalette pal = cvlist->palette();
	pal.setColor(QPalette::Base, PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.background").value<QColor>());
	cvlist->setPalette(pal);

	int layoutMargin = 0;
#ifdef Q_WS_MAC
	layoutMargin = 0;
	cvlist->setFrameShape(QFrame::NoFrame);
#endif
	d->vb_main->setMargin(layoutMargin);
	d->vb_main->setSpacing(layoutMargin);

	d->blockString = false;
	d->vb_main->addWidget(d->cb_search);
	connect(d->cb_search, SIGNAL(editTextChanged(const QString&)), this, SLOT(searchRoster(const QString&)));
	d->vb_main->addWidget(cvlist);

#ifdef Q_WS_MAC
	// Disable the empty vertical scrollbar:
	// it's here because of weird code in q3scrollview.cpp
	// Q3ScrollView::updateScrollBars() around line 877
	d->vb_main->addSpacing(4);
#endif

	d->statusMenu = new QMenu(this);
	d->optionsMenu = new QMenu(this);
#ifdef Q_WS_MAC
	d->trayMenu = d->statusMenu;
#else
	d->trayMenu = new QMenu(this);
	buildTrayMenu();
	connect(d->trayMenu, SIGNAL(aboutToShow()), SLOT(buildTrayMenu()));
#endif


	buildStatusMenu();
	buildOptionsMenu();
	connect(d->optionsMenu, SIGNAL(aboutToShow()), SLOT(buildOptionsMenu()));


	X11WM_CLASS("main");

	connect(d->psi, SIGNAL(accountCountChanged()), SLOT(numAccountsChanged()));
	numAccountsChanged();

	updateCaption();

	d->registerActions();
	
	connect(d->psi->contactList(), SIGNAL(accountFeaturesChanged()), SLOT(accountFeaturesChanged()));
	accountFeaturesChanged();

	decorateButton(STATUS_OFFLINE);

	// Mac-only menus
#ifdef Q_WS_MAC
	QMenu *mainMenu = new QMenu(this);
	mainMenuBar()->insertItem(tr("Menu"), mainMenu);
	d->getAction("menu_options")->addTo(mainMenu);
	d->getAction("menu_quit")->addTo(mainMenu);
	d->getAction("help_about")->addTo(mainMenu);
	d->getAction("help_about_qt")->addTo(mainMenu);

	d->mainMenu = new QMenu(this);
	mainMenuBar()->insertItem(tr("General"), d->mainMenu);
	connect(d->mainMenu, SIGNAL(aboutToShow()), SLOT(buildMainMenu()));
#else
	mainMenuBar()->insertItem(tr("General"), d->optionsMenu);
#endif

	mainMenuBar()->insertItem(tr("Status"), d->statusMenu);

	QMenu *viewMenu = new QMenu(this);
	mainMenuBar()->insertItem(tr("View"), viewMenu);
	d->getAction("show_offline")->addTo(viewMenu);
	d->getAction("show_no_offline_status_msg")->addTo(viewMenu);
	if (PsiOptions::instance()->getOption("options.ui.menu.view.show-away").toBool())
		d->getAction("show_away")->addTo(viewMenu);
	d->getAction("show_hidden")->addTo(viewMenu);
	d->getAction("show_agents")->addTo(viewMenu);
	d->getAction("show_self")->addTo(viewMenu);
	viewMenu->insertSeparator();
	d->getAction("show_statusmsg")->addTo(viewMenu);

	// Mac-only menus
#ifdef Q_WS_MAC
	d->toolsMenu = new QMenu(this);
	mainMenuBar()->insertItem(tr("Tools"), d->toolsMenu);
	connect(d->toolsMenu, SIGNAL(aboutToShow()), SLOT(buildToolsMenu()));

	QMenu *helpMenu = new QMenu(this);
	mainMenuBar()->insertItem(tr("Help"), helpMenu);
	d->getAction("help_readme")->addTo (helpMenu);
	d->getAction("help_tip")->addTo (helpMenu);
	helpMenu->insertSeparator();
	d->getAction("help_online_help")->addTo (helpMenu);
	d->getAction("help_online_wiki")->addTo (helpMenu);
	d->getAction("help_online_home")->addTo (helpMenu);
	d->getAction("help_psi_muc")->addTo (helpMenu);
	d->getAction("help_report_bug")->addTo (helpMenu);
	QMenu *diagMenu = new QMenu(this);
	helpMenu->insertItem(tr("Diagnostics"), diagMenu);
	d->getAction("help_diag_qcaplugin")->addTo (diagMenu);
	d->getAction("help_diag_qcakeystore")->addTo (diagMenu);
#else
	if (!PsiOptions::instance()->getOption("options.ui.contactlist.show-menubar").toBool())  {
		mainMenuBar()->hide();
	}
	//else 
	//	mainMenuBar()->show();
#endif
	d->optionsButton->setMenu( d->optionsMenu );
	d->statusButton->setMenu( d->statusMenu );
	
	buildinitialToolbars();

	connect(qApp, SIGNAL(dockActivated()), SLOT(dockActivated()));

	connect(psi, SIGNAL(emitOptionsUpdate()), SLOT(optionsUpdate()));
	optionsUpdate();
}

MainWin::~MainWin()
{
	PsiPopup::deleteAll();

	if(d->tray) {
		delete d->tray;
		d->tray = 0;
	}

	//saveToolbarsPositions();
	// need to find some workaround to case, when you're logging off. in that case
	// toobars are all disabled, and when you start psi again you need to enable
	// your toolbars

	delete d;
}

void MainWin::registerAction( IconAction *action )
{
	char activated[] = SIGNAL( activated() );
	char toggled[]   = SIGNAL( toggled(bool) );
	char setChecked[]     = SLOT( setChecked(bool) );

	struct {
		const char *name;
		const char *signal;
		QObject *receiver;
		const char *slot;
	
	} actionlist[] = {
		{ "show_offline", toggled, ((SIMContactList::List*)d->psi->contactList()), SLOT( setShowOffline(bool) ) },
		{ "show_away",    toggled, ((SIMContactList::List*)d->psi->contactList()), SLOT( setShowAway(bool) ) },
		{ "show_groups", toggled, ((SIMContactList::List*)d->psi->contactList()), SLOT( setShowGroups(bool) ) },
		{ "show_agents",  toggled, ((SIMContactList::List*)d->psi->contactList()), SLOT( setShowAgents(bool) ) },
		{ "show_self",    toggled, ((SIMContactList::List*)d->psi->contactList()), SLOT( setShowSelf(bool) ) },

		{ "button_options", activated, this, SIGNAL( doOptions() ) },

		{ "menu_disco",       SIGNAL( activated(PsiAccount *, int) ), this, SLOT( activatedAccOption(PsiAccount*, int) ) },
		{ "menu_add_contact", SIGNAL( activated(PsiAccount *, int) ), this, SLOT( activatedAccOption(PsiAccount*, int) ) },
		{ "menu_xml_console", SIGNAL( activated(PsiAccount *, int) ), this, SLOT( activatedAccOption(PsiAccount*, int) ) },

		{ "menu_new_message",    activated, this, SIGNAL( blankMessage() ) },
		{ "menu_join_groupchat", activated, this, SIGNAL( doGroupChat() ) },
		{ "menu_account_setup",  activated, this, SIGNAL( doManageAccounts() ) },
		{ "menu_options",        activated, this, SIGNAL( doOptions() ) },
		{ "menu_file_transfer",  activated, this, SIGNAL( doFileTransDlg() ) },
		{ "menu_toolbars",       activated, this, SIGNAL( doToolbars() ) },
		{ "menu_change_profile", activated, this, SIGNAL( changeProfile() ) },
		{ "menu_quit",           activated, this, SLOT( try2tryCloseProgram() ) },
		{ "menu_play_sounds",    toggled,   this, SLOT( actPlaySoundsActivated(bool) ) },
		{ "publish_tune",        toggled,   this, SLOT( actPublishTuneActivated(bool) ) },
		{ "publish_mood",        activated,   this, SLOT( actPublishMood() ) },

		{ "help_readme",      activated, this, SLOT( actReadmeActivated() ) },
		{ "help_tip",         activated, this, SLOT( actTipActivated() ) },
		{ "help_online_help", activated, this, SLOT( actOnlineHelpActivated() ) },
		{ "help_online_wiki", activated, this, SLOT( actOnlineWikiActivated() ) },
		{ "help_online_home", activated, this, SLOT( actOnlineHomeActivated() ) },
		{ "help_psi_muc",     activated, this, SLOT( actJoinPsiMUCActivated() ) },
		{ "help_report_bug",  activated, this, SLOT( actBugReportActivated() ) },
		{ "help_about",       activated, this, SLOT( actAboutActivated() ) },
		{ "help_about_qt",    activated, this, SLOT( actAboutQtActivated() ) },
		{ "help_diag_qcaplugin",   activated, this, SLOT( actDiagQCAPluginActivated() ) },
		{ "help_diag_qcakeystore", activated, this, SLOT( actDiagQCAKeyStoreActivated() ) },

		{ "", 0, 0, 0 }
	};

	int i;
	QString aName;
	for ( i = 0; !(aName = QString(actionlist[i].name)).isEmpty(); i++ ) {
		if ( aName == action->name() ) {
			// Check before connecting, otherwise we get a loop
			if ( aName == "publish_tune")
				action->setChecked( PsiOptions::instance()->getOption("options.extended-presence.tune.publish").toBool() );

			disconnect( action, actionlist[i].signal, actionlist[i].receiver, actionlist[i].slot ); // for safety
			connect( action, actionlist[i].signal, actionlist[i].receiver, actionlist[i].slot );

			// special cases
			if ( aName == "menu_play_sounds" )
				action->setChecked(PsiOptions::instance()->getOption("options.ui.notifications.sounds.enable").toBool());
			//else if ( aName == "foobar" )
			//	;
		}
	}

	struct {
		const char *name;
		QObject *sender;
		const char *signal;
		const char *slot;
		bool checked;
	} reverseactionlist[] = {
		{ "show_away",    ((SIMContactList::List *)d->psi->contactList()), SIGNAL( showAway(bool) ), setChecked, ((SIMContactList::List *)d->psi->contactList())->showAway()},
//		{ "show_hidden",  ((SIMContactList::List *)d->psi->contactList()), SIGNAL( showHidden(bool) ), setChecked, cvlist->showHidden()},
		{ "show_offline", ((SIMContactList::List *)d->psi->contactList()), SIGNAL( showOffline(bool) ), setChecked, ((SIMContactList::List *)d->psi->contactList())->showOffline()},
		{ "show_self",    ((SIMContactList::List *)d->psi->contactList()), SIGNAL( showSelf(bool) ), setChecked, ((SIMContactList::List *)d->psi->contactList())->showSelf()},
		{ "show_agents",  ((SIMContactList::List *)d->psi->contactList()), SIGNAL( showAgents(bool) ), setChecked, ((SIMContactList::List *)d->psi->contactList())->showAgents()},
		{ "show_groups", ((SIMContactList::List *)d->psi->contactList()), SIGNAL( showGroups(bool) ), setChecked, ((SIMContactList::List *)d->psi->contactList())->showGroups()},
		{ "", 0, 0, 0, false }
	};

	for ( i = 0; !(aName = QString(reverseactionlist[i].name)).isEmpty(); i++ ) {
		if ( aName == action->name() ) {
			disconnect( reverseactionlist[i].sender, reverseactionlist[i].signal, action, reverseactionlist[i].slot ); // for safety
			connect( reverseactionlist[i].sender, reverseactionlist[i].signal, action, reverseactionlist[i].slot );

			if (aName == "show_statusmsg") {
				action->setChecked( PsiOptions::instance()->getOption("options.ui.contactlist.status-messages.show").toBool() );
			}
			else
				action->setChecked( reverseactionlist[i].checked );
		}
	}
}

PsiCon *MainWin::psiCon() const
{
	return d->psi;
}

void MainWin::showMessage(const QString &title, const QString &msg, QSystemTrayIcon::MessageIcon mi, int msecs)
{
	if(d->tray)
		d->tray->showMessage(title, msg, mi, msecs);
}

void MainWin::setWindowOpts(bool _onTop, bool _asTool)
{
	if(_onTop == d->onTop && _asTool == d->asTool)
		return;

	d->onTop = _onTop;
	d->asTool = _asTool;

	Qt::WFlags flags = 0;
	if(d->onTop)
		flags |= Qt::WStyle_StaysOnTop;
	if(d->asTool)
		flags |= Qt::WStyle_Tool | TOOLW_FLAGS;

	QPoint p = pos();
	reparent(parentWidget(), flags, p, FALSE);
	move(p);
	show();
}

void MainWin::setUseDock(bool use)
{
	if (use == (d->tray != 0)) {
		return;
	}

	if (d->tray) {
		delete d->tray;
		d->tray = 0;
	}

	Q_ASSERT(!d->tray);
	if (use) {
		d->tray = new PsiTrayIcon("Synapse-IM", d->trayMenu, NULL);
/*		if (d->old_trayicon) {
			connect(d->tray, SIGNAL(closed()), SLOT(dockActivated()));
			connect(qApp, SIGNAL(trayOwnerDied()), SLOT(dockActivated()));
		}*/
		connect(d->tray, SIGNAL(clicked(const QPoint &, int)), SLOT(trayClicked(const QPoint &, int)));
		connect(d->tray, SIGNAL(doubleClicked(const QPoint &)), SLOT(trayDoubleClicked()));
		d->tray->setIcon(PsiIconset::instance()->statusPtr(STATUS_OFFLINE));
		d->tray->setToolTip(ApplicationInfo::name());

		updateReadNext(d->nextAnim, d->nextAmount);

		d->tray->show();
	}
}

void MainWin::searchRoster(const QString &text)
{
	d->psi->contactList()->setSearch(text);
}

void MainWin::updateStatusLastMenu()
{
	LastStatus *ls = NULL;
	for(int i=0; i<5; i++)
	{
		ls = d->psi->getLastStatus(i);
		if(ls == NULL)
			return;
		QString t;
		switch(ls->type) {
			case STATUS_OFFLINE:
				t="status/offline";
				break;
			case STATUS_AWAY:
				t="status/away";
				break;
			case STATUS_XA:
				t="status/xa";
				break;
			case STATUS_DND:
				t="status/dnd";
				break;
			case STATUS_CHAT:
				t="status/chat";
				break;
			case STATUS_ONLINE:	
			default:
				t="status/online";
				break;
		}
		d->statusLastAction[i]->setIcon(IconsetFactory::iconPtr(t)->icon());
		d->statusLastAction[i]->setText(ls->status.left(15));
	}
}

void MainWin::buildStatusLastMenu()
{
	if(d->statusLastMenu)
	{
		delete d->statusLastMenu;
		d->statusLastMenu = 0;
	}

	d->statusLastMenu = d->statusMenu->addMenu(IconsetFactory::iconPtr("status/ask")->icon(),tr("Lastly used.."));

	for(int i=0; i<5; i++)
	{
		d->statusLastAction[i] = d->statusLastMenu->addAction("Last 1");
		d->statusLastMapper->setMapping(d->statusLastAction[i], i);
		connect (d->statusLastAction[i], SIGNAL(activated()), d->statusLastMapper, SLOT(map()));
	}
}

void MainWin::buildStatusMenu()
{
	d->statusMenu->clear();
	d->getAction("status_online")->addTo(d->statusMenu);
	if (PsiOptions::instance()->getOption("options.ui.menu.status.chat").toBool())
		d->getAction("status_chat")->addTo(d->statusMenu);
	d->statusMenu->insertSeparator();
	d->getAction("status_away")->addTo(d->statusMenu);
	if (PsiOptions::instance()->getOption("options.ui.menu.status.xa").toBool())
		d->getAction("status_xa")->addTo(d->statusMenu);
	d->getAction("status_dnd")->addTo(d->statusMenu);
	d->statusMenu->insertSeparator();

	buildStatusLastMenu();
//	updateStatusLastMenu();

	d->statusMenu->insertSeparator();
	d->getAction("status_offline")->addTo(d->statusMenu);
	d->statusMenu->insertSeparator();
	d->getAction("publish_mood")->addTo(d->statusMenu);
	d->getAction("publish_tune")->addTo(d->statusMenu);
}

void MainWin::activatedStatusAction(int id)
{
	QObjectList l = d->statusGroup->queryList( "IconAction" );
	for (QObjectList::Iterator it = l.begin() ; it != l.end(); ++it) {
		IconAction *action = (IconAction *)(*it);
		action->setChecked ( d->statusActions[action] == id );
	}

	statusChanged(id);
}

void MainWin::activatedStatusLastAction(int id)
{
	QObjectList l = d->statusGroup->queryList( "IconAction" );
	for (QObjectList::Iterator it = l.begin() ; it != l.end(); ++it) {
		IconAction *action = (IconAction *)(*it);
		action->setChecked ( false );
	}

	printf("clicked %d\n",id);
//	statusChanged(id);
	statusLastChanged(id);
}

QMenuBar* MainWin::mainMenuBar() const
{
#ifdef Q_WS_MAC
	return psiCon()->defaultMenuBar();
#else
	return menuBar();
#endif
}

void MainWin::addToolbar(const QString &base)
{
	PsiToolBar *tb = PsiToolBar::fromOptions(base, this, d->psi, PsiActionList::Actions_MainWin);
	
	//connect( tb, SIGNAL( registerAction( IconAction * ) ), SLOT( registerAction( IconAction * ) ) );
	
	toolbars << tb;
}

void MainWin::buildinitialToolbars()
{
	QStringList bases = PsiOptions::instance()->getChildOptionNames("options.ui.contactlist.toolbars", true, true);
	foreach(QString base, bases) {
		addToolbar(base);
	}
}

void MainWin::saveToolbarsPositions()
{
/*
	LEGOPTFIXME
	for (int i = 0; i < toolbars.count(); i++) {
		Options::ToolbarPrefs &tbPref = LEGOPTS.toolbars["mainWin"][i];
		getLocation ( toolbars.at(i), tbPref.dock, tbPref.index, tbPref.nl, tbPref.extraOffset );
		tbPref.on = toolbars.at(i)->isVisible();
	}
*/
}

bool MainWin::showDockMenu(const QPoint &)
{
	return false;
}

void MainWin::buildOptionsMenu()
{
	// help menu
	QMenu *helpMenu = new QMenu(d->optionsMenu);

	QStringList actions;
	actions << "help_readme"
	        << "help_tip"
	        << "separator"
	        << "help_online_help"
	        << "help_online_wiki"
	        << "help_online_home"
		<< "help_psi_muc"
	        << "help_report_bug"
		<< "diagnostics"
	        << "separator"
	        << "help_about"
	        << "help_about_qt";

	d->updateMenu(actions, helpMenu);

	buildGeneralMenu( d->optionsMenu );

	d->optionsMenu->insertSeparator();
	d->optionsMenu->insertItem(IconsetFactory::icon("psi/help").icon(), tr("&Help"), helpMenu);
	d->getAction("menu_quit")->addTo( d->optionsMenu );

}

void MainWin::buildMainMenu()
{
	// main menu
	QStringList actions;
	actions << "menu_add_contact";
	if (PsiOptions::instance()->getOption("options.ui.message.enabled").toBool())
		actions << "menu_new_message";
	actions << "menu_disco"
	        << "menu_join_groupchat"
	        << "separator"
	        << "menu_account_setup";
	if (PsiOptions::instance()->getOption("options.ui.menu.main.change-profile").toBool())
	        actions << "menu_change_profile";
	actions << "menu_play_sounds";

	d->updateMenu(actions, d->mainMenu);
}

void MainWin::buildToolsMenu()
{
	QStringList actions;
	actions << "menu_file_transfer"
	        << "separator"
	        << "menu_xml_console";
	
	d->updateMenu(actions, d->toolsMenu);
}
	
void MainWin::buildGeneralMenu(QMenu *menu)
{
	// options menu
	QStringList actions;
	actions << "menu_add_contact";
	if (PsiOptions::instance()->getOption("options.ui.message.enabled").toBool())
		actions << "menu_new_message";
	actions << "menu_disco"
	        << "menu_join_groupchat"
	        << "menu_account_setup"
	        << "menu_options"
	        << "menu_file_transfer";
	if (PsiOptions::instance()->getOption("options.ui.menu.main.change-profile").toBool())
	        actions << "menu_change_profile";
	actions << "menu_play_sounds";

	d->updateMenu(actions, menu);
}

void MainWin::actReadmeActivated ()
{
	ShowTextDlg *w = new ShowTextDlg(":/README");
	w->setWindowTitle(CAP(tr("ReadMe")));
	w->show();
}

void MainWin::actOnlineHelpActivated ()
{
	QDesktopServices::openUrl(QUrl("http://psi-im.org/wiki/User_Guide"));
}

void MainWin::actOnlineWikiActivated ()
{
	QDesktopServices::openUrl(QUrl("http://psi-im.org/wiki"));
}

void MainWin::actOnlineHomeActivated ()
{
	QDesktopServices::openUrl(QUrl("http://synapse.hi-low.eu"));
}

void MainWin::actJoinPsiMUCActivated()
{
	PsiAccount *account = d->psi->contactList()->defaultAccount();
	if(!account)
		return;

	account->actionJoin("psi@conference.psi-im.org");
}

void MainWin::actBugReportActivated ()
{
	QDesktopServices::openUrl(QUrl("http://synapse.hi-low.eu"));
}

void MainWin::actAboutActivated ()
{
	AboutDlg *about = new AboutDlg();
	about->show();
}

void MainWin::actTipActivated ()
{
	TipDlg::show(d->psi);
}

void MainWin::actAboutQtActivated ()
{
	QMessageBox::aboutQt(this);
}

void MainWin::actDiagQCAPluginActivated()
{
	QString dtext = QCA::pluginDiagnosticText();
	ShowTextDlg *w = new ShowTextDlg(dtext, true, false, this);
	w->setWindowTitle(CAP(tr("Security Plugins Diagnostic Text")));
	w->resize(560, 240);
	w->show();
}

void MainWin::actDiagQCAKeyStoreActivated()
{
	QString dtext = QCA::KeyStoreManager::diagnosticText();
	ShowTextDlg *w = new ShowTextDlg(dtext, true, false, this);
	w->setWindowTitle(CAP(tr("Key Storage Diagnostic Text")));
	w->resize(560, 240);
	w->show();
}

void MainWin::actPlaySoundsActivated (bool state)
{
	PsiOptions::instance()->setOption("options.ui.notifications.sounds.enable", state);
}

void MainWin::actPublishMood ()
{
	QList<PsiAccount *> pal = d->psi->contactList()->enabledAccounts();
	QList<PsiAccount *> pal_pep;
	printf("list\n");
	foreach( PsiAccount *pa, pal) {
		if( pa->serverInfoManager()->hasPEP() )
			pal_pep.append(pa);
	}
	printf("done\n");
	if (!pal_pep.isEmpty()) {
		printf("mood\n");
		MoodDlg *w = new MoodDlg(pal_pep);
		w->show();
	}
}

void MainWin::actPublishTuneActivated (bool state)
{
	if(!state)
		d->psi->tuneController()->stop();
	PsiOptions::instance()->setOption("options.extended-presence.tune.publish",state);
}

void MainWin::activatedAccOption(PsiAccount *pa, int x)
{
	if(x == 0)
		pa->openAddUserDlg();
	else if(x == 2)
		pa->showXmlConsole();
	else if(x == 3)
		pa->doDisco();
}

void MainWin::buildTrayMenu()
{
#ifndef Q_WS_MAC
	d->trayMenu->clear();

	if(d->nextAmount > 0) {
		d->trayMenu->insertItem(tr("Receive next event"), this, SLOT(doRecvNextEvent()));
		d->trayMenu->insertSeparator();
	}

	if(isHidden())
		d->trayMenu->insertItem(tr("Un&hide"), this, SLOT(trayShow()));
	else
		d->trayMenu->insertItem(tr("&Hide"), this, SLOT(trayHide()));
	d->optionsButton->addTo(d->trayMenu);
	d->trayMenu->insertItem(tr("Status"), d->statusMenu);
	
	d->trayMenu->insertSeparator();
	// TODO!
	d->getAction("menu_quit")->addTo(d->trayMenu);
#endif
}

void MainWin::setTrayToolTip(int status)
{
	if (!d->tray)
		return;
	d->tray->setToolTip(QString("Synapse-IM - " + status2txt(status)));
}

void MainWin::decorateButton(int status)
{
	// update the 'change status' buttons
	QObjectList l = d->statusGroup->queryList( "IconAction" );
	for (QObjectList::Iterator it = l.begin() ; it != l.end(); ++it) {
		IconAction *action = (IconAction *)(*it);
		action->setChecked ( d->statusActions[action] == status );
	}

	if(d->lastStatus == status)
		return;
	d->lastStatus = status;

	if(status == -1) {
		d->statusButton->setText(tr("Connecting"));
		if (PsiOptions::instance()->getOption("options.ui.notifications.alert-style").toString() != "no") {
			d->statusButton->setAlert(IconsetFactory::iconPtr("psi/connect"));
			d->statusGroup->setPsiIcon(IconsetFactory::iconPtr("psi/connect"));
		}
		else {
			d->statusButton->setIcon(PsiIconset::instance()->statusPtr(STATUS_OFFLINE));
			d->statusGroup->setPsiIcon(PsiIconset::instance()->statusPtr(STATUS_OFFLINE));
		}

		setWindowIcon(PsiIconset::instance()->status(STATUS_OFFLINE).impix());
	}
	else {
		d->statusButton->setText(status2txt(status));
		d->statusButton->setIcon(PsiIconset::instance()->statusPtr(status));
		d->statusGroup->setPsiIcon(PsiIconset::instance()->statusPtr(status));

		setWindowIcon(PsiIconset::instance()->status(status).impix());
	}

	updateTray();
}

bool MainWin::askQuit()
{
	return TRUE;
}

void MainWin::try2tryCloseProgram()
{
	QTimer::singleShot(0, this, SLOT(tryCloseProgram()));
}

void MainWin::tryCloseProgram()
{
	if(askQuit())
		closeProgram();
}

void MainWin::closeEvent(QCloseEvent *e)
{
#ifdef Q_WS_MAC
	trayHide();
	e->accept();
#else
	if(d->tray) {
		trayHide();
		e->accept();
		return;
	}

	if(!askQuit())
		return;

        closeProgram();
	e->accept();
#endif
}

void MainWin::keyPressEvent(QKeyEvent *e)
{
#ifdef Q_WS_MAC
	bool allowed = true;
#else
	bool allowed = d->tray ? true: false;
#endif

	bool closekey = false;
	if(e->key() == Qt::Key_Escape)
		closekey = true;
#ifdef Q_WS_MAC
	else if(e->key() == Qt::Key_W && e->modifiers() & Qt::ControlModifier)
		closekey = true;
#endif

	if(allowed && closekey) {
		close();
		e->accept();
		return;
	}

	QWidget::keyPressEvent(e);
}

#ifdef Q_WS_WIN
#include <windows.h>
bool MainWin::winEvent(MSG *msg, long *result)
{
	if (d->asTool && msg->message == WM_SYSCOMMAND && msg->wParam == SC_MINIMIZE) {
		hide();	// minimized toolwindows look bad on Windows, so let's just hide it instead
			// plus we cannot do this in changeEvent(), because it's called too late
		*result = 0;
		return true;	// don't let Qt process this event
	}
	return false;
}
#endif

void MainWin::updateCaption()
{
	QString str = "";

	if(d->nextAmount > 0)
		str += "* ";

	if(d->nickname.isEmpty())
		str += ApplicationInfo::name();
	else
		str += d->nickname;

	if(str == caption())
		return;

	setWindowTitle(str);
}

void MainWin::optionsUpdate()
{
	int status = d->lastStatus;
	d->lastStatus = -2;
	decorateButton(status);

#ifndef Q_WS_MAC
	if (!PsiOptions::instance()->getOption("options.ui.contactlist.show-menubar").toBool()) {
		mainMenuBar()->hide();
	} else {
		mainMenuBar()->show();
	}
#endif
	QPalette pal = cvlist->palette();
	pal.setColor(QPalette::Base, PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.background").value<QColor>());
	cvlist->setPalette(pal);

	setWindowOpacity(double(qMax(MINIMUM_OPACITY,PsiOptions::instance()->getOption("options.ui.contactlist.opacity").toInt()))/100);

	buildStatusMenu();
	
	updateTray();
}

void MainWin::toggleVisible()
{
	if(!isHidden())
		trayHide();
	else
		trayShow();
}

void MainWin::setTrayToolTip(const Status &status, bool)
{
	if (!d->tray)
		return;
	QString s = "Synapse-IM";

 	QString show = status.show();
	if(!show.isEmpty()) {
		show[0] = show[0].upper();
		s += " - "+show;
	}

	QString text = status.status();
	if(!text.isEmpty())
		s += ": "+text;

	d->tray->setToolTip(s);
}

void MainWin::trayClicked(const QPoint &, int button)
{
	if(PsiOptions::instance()->getOption("options.ui.systemtray.use-double-click").toBool())
		return;

	if(button == Qt::MidButton || d->nextAmount > 0) {
		doRecvNextEvent();
		return;
	}

	if(!isHidden())
		trayHide();
	else
		trayShow();
}

void MainWin::trayDoubleClicked()
{
	if(!PsiOptions::instance()->getOption("options.ui.systemtray.use-double-click").toBool())
		return;

	if(d->nextAmount > 0) {
		doRecvNextEvent();
		return;
	}


	if(!isHidden())
		trayHide();
	else
		trayShow();
}

void MainWin::trayShow()
{
	bringToFront(this);
}

void MainWin::trayHide()
{
	emit geomChanged(saveableGeometry());
	hide();
}

void MainWin::updateReadNext(PsiIcon *anim, int amount)
{
	d->nextAnim = anim;
	if(anim == 0)
		d->nextAmount = 0;
	else
		d->nextAmount = amount;

	if(d->nextAmount <= 0) {
		d->eventNotifier->hide();
		d->eventNotifier->setText("");
	}
	else {
		d->eventNotifier->setText(numEventsString(d->nextAmount));
		d->eventNotifier->show();
		// make sure it shows
		//qApp->processEvents();
	}

	updateTray();
	updateCaption();
}

QString MainWin::numEventsString(int x) const
{
	QString s;
	if(x <= 0)
		s = "";
	else if(x == 1)
		s = tr("1 event received");
	else
		s = tr("%1 events received").arg(x);

	return s;
}

void MainWin::updateTray()
{
	if(!d->tray)
		return;

	if ( d->nextAmount > 0 )
		d->tray->setAlert(d->nextAnim);
	else if ( d->lastStatus == -1 )
		d->tray->setAlert(IconsetFactory::iconPtr("psi/connect"));
	else
		d->tray->setIcon(PsiIconset::instance()->statusPtr(d->lastStatus));
	
	buildTrayMenu();
	d->tray->setContextMenu(d->trayMenu);
}

void MainWin::doRecvNextEvent()
{
	recvNextEvent();
}

void MainWin::statusClicked(int x)
{
	if(x == Qt::MidButton)
		recvNextEvent();
}

void MainWin::numAccountsChanged()
{
	d->statusButton->setEnabled(d->psi->contactList()->haveEnabledAccounts());
}

void MainWin::accountFeaturesChanged()
{
	bool have_pep = false;
	foreach(PsiAccount* account, d->psi->contactList()->enabledAccounts()) {
		if (account->serverInfoManager()->hasPEP()) {
			have_pep = true;
			break;
		}
	}

//	d->getAction("publish_tune")->setEnabled(have_pep);
	d->getAction("publish_mood")->setEnabled(have_pep);
	d->getAction("publish_tune")->setEnabled(true);
}

void MainWin::dockActivated()
{
	if(isHidden())
		show();
}


#ifdef Q_WS_MAC
void MainWin::setWindowIcon(const QPixmap&)
{
}
#else
void MainWin::setWindowIcon(const QPixmap& p)
{
	QMainWindow::setWindowIcon(p);
}
#endif

void MainWin::showNoFocus()
{
	bringToFront(this);
}

void MainWin::moveEvent(QMoveEvent* e)
{
	AdvancedWidget<QMainWindow>::moveEvent(e);
	QTimer::singleShot(0, this, SLOT(geometryChanged()));
}

void MainWin::resizeEvent(QResizeEvent* e)
{
	AdvancedWidget<QMainWindow>::resizeEvent(e);
	QTimer::singleShot(0, this, SLOT(geometryChanged()));
}

void MainWin::geometryChanged()
{
	emit geomChanged(saveableGeometry());
}


