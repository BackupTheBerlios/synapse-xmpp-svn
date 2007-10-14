/*
 * privkeywidget.h - widget for private key configuration.
 *
 * Copyright (C) Timo Engel (timo-e@freenet.de), Berlin 2007.
 * This program was written as part of a diplom thesis advised by 
 * Prof. Dr. Ruediger Weis (PST Labor)
 * at the Technical University of Applied Sciences Berlin.
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

#ifndef PRIVKEYWIDGET_H_
#define PRIVKEYWIDGET_H_

#include <QWidget>
#include <QtCore>

#include "privkeytablemodel.h"

class QLabel;
class QPushButton;
class QGroupBox;
class QCheckBox;
class QTabWidget;
class AbstractItemModel;
class PrivKeyTableModel;


class PrivKeyWidget : public QWidget {
	Q_OBJECT
	
	public:
		PrivKeyWidget(PrivKeyTableModel* model);

	private:
		PrivKeyTableModel* tableModel;
	

	
};


#endif /*CONFIGDLG_H_*/
