/*
 * psitoolbar.h - the Psi toolbar class
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

#ifndef PSITOOLBAR_H
#define PSITOOLBAR_H

#include <QToolBar>

#include "psiactionlist.h"
#include "common.h" // Options + ToolbarPrefs

class PsiCon;
class QContextMenuEvent;

class PsiToolBar : public QToolBar
{
	Q_OBJECT

public:
	static PsiToolBar *fromOptions(const QString &base, QMainWindow* mainWindow, PsiCon* psi, PsiActionList::ActionsType t);
	static void structToOptions(const QString &base, ToolbarPrefs *s);
	
	PsiToolBar(const QString& label, QMainWindow* mainWindow, PsiCon* psi);
	~PsiToolBar();

	PsiActionList::ActionsType type() const;
	void setType( PsiActionList::ActionsType );
	void initialize( QString base, bool createUniqueActions );
  
/*	bool isCustomizeable() const;
	void setCustomizeable( bool );

	bool isMoveable() const;
	void setMoveable( bool );*/
	
signals:
	void registerAction( IconAction * );
	
public slots:
	void optionChanged(const QString& option);
	void update();
	void optionRemoved(const QString& option);

protected:
	void contextMenuEvent(QContextMenuEvent *e);

private:
	class Private;
	Private *d;
};

#endif
