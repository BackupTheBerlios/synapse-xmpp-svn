/*
 * fingerprintwidget.h - widget for fingerprint configuration.
 *
 * Copyright (C) 2007  Timo Engel (timo-e@freenet.de)
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


#ifndef FINGERPRINTWIDGET_H_
#define FINGERPRINTWIDGET_H_

#include <QWidget>

#include "fprint.h"

class QLabel;
class QPushButton;
class QGroupBox;
class QCheckBox;
class QTabWidget;
class QStandardItem;
class QStandardItemModel;
class QTableView;
class QModelIndex;

class OtrConnection;

class FingerprintWidget : public QWidget {
	Q_OBJECT
	
	public:
		FingerprintWidget(QList< Fprint > aFpList);	
	
	private:
		QList< Fprint > fpTableContent;
		QTableView* fpTable;
		QStandardItemModel* model;
		int tableIndex;
	
	private slots:
		void forgetFingerprint();
		void verifyFingerprint();
		void tableClicked(const QModelIndex & index );

	signals:
		void forgetFingerprint(unsigned char* fp);
		void verifyFingerprint(unsigned char* fp, bool verified);

	
};


#endif /*CONFIGDLG_H_*/
