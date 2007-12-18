/*
 * tabbable.cpp
 * Copyright (C) 2007 Kevin Smith
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

#include "tabbablewidget.h"
#include "tabmanager.h"
#include "tabdlg.h"
#include "common.h"
#include "jidutil.h"
#include "groupchatdlg.h"
#include <QTimer>


#ifdef Q_WS_WIN
#include <windows.h>
#endif

//----------------------------------------------------------------------------
// TabbableWidget
//----------------------------------------------------------------------------

TabbableWidget::TabbableWidget(const Jid &jid, PsiAccount *pa, TabManager *tabManager)
	:AdvancedWidget<QWidget>(0), jid_(jid), pa_(pa), tabManager_(tabManager)
{
	hide();
	QTimer::singleShot(0,this,SLOT(ensureTabbedCorrectly()));
}

void TabbableWidget::ensureTabbedCorrectly() {
	if (tabManager_->shouldBeTabbed(this)) {
		if (!isTabbed()) {
			tabManager_->getTabs()->addTab(this);
		}
		if(tabManager_->getTabs()->tabOnTop(this))
			show();
	} else {
		if (isTabbed()) {
			getManagingTabDlg()->closeTab(this, false);
		}
		show();
	}
}

void TabbableWidget::bringToFront() 
{
	if ( isTabbed() )
	{
		getManagingTabDlg()->selectTab(this);
	}
	::bringToFront(this);
}

TabbableWidget::~TabbableWidget()
{
	hide();
}

void TabbableWidget::hideEvent ( QHideEvent * event ) {
	Q_UNUSED(event);
	if (!isVisible()) {
		//you can have a hideEvent and still be visible, check the docs.
		if (isTabbed()) {
			getManagingTabDlg()->removeTabWithNoChecks(this);
		}
	}
}

bool TabbableWidget::isTabbed() {
	return tabManager_->isChatTabbed(this);
}

TabDlg* TabbableWidget::getManagingTabDlg()
{
	return tabManager_->getManagingTabs(this);
}

/**
 * Runs any gumph necessary before hiding a tab.
 * (checking new messages, setting the autodelete, cancelling composing etc)
 * \return TabbableWidget is ready to be hidden.
 */
bool TabbableWidget::readyToHide()
{
	return true;
}

Jid TabbableWidget::jid() const
{
	return jid_;
}

const QString& TabbableWidget::getDisplayName()
{
	return jid_.user();
}

void TabbableWidget::activated()
{
}

bool TabbableWidget::isActiveTab()
{
	if ( isHidden() )
	{
		return false;
	}
	if (!isTabbed()/* && !getManagingTabDlg()*/)
	{
		return isActiveWindow();
	}
	return getManagingTabDlg()->isActiveWindow() &&
	       getManagingTabDlg()->tabOnTop(this);
}