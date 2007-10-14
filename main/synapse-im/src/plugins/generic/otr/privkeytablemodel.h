/*
 * privkeytablemodel.h - table model for private keys.
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

#ifndef  PRIVKEYTABLEMODEL_INC
#define  PRIVKEYTABLEMODEL_INC

#include<QtCore>
#include<QtGui>

#include "otrconnection.h"

class QVariant;
class OtrConnection;
class QModelIndex;

class PrivKeyTableModel : public QAbstractTableModel {
	Q_OBJECT

	public:
	PrivKeyTableModel(QObject *parent = 0);
	PrivKeyTableModel(OtrConnection* aOtrConnection);


	QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

	private:
	QList< QString >* tableHeader;
	QHash<QString, QString>* tableData;
	OtrConnection* otrConnection;
	QString* selectedAccount;

	public slots:
	void tableClicked(const QModelIndex & index );
	void deleteKeyClicked();

};


#endif   /* ----- #ifndef PRIVKEYTABLEMODEL_INC  ----- */
