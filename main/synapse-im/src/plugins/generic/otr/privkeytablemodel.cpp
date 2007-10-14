/*
 * privkeytablemodel.cpp - table model for private keys
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

#include "privkeytablemodel.h"


PrivKeyTableModel::PrivKeyTableModel(QObject *parent) : QAbstractTableModel(parent) {
	tableHeader = new QList< QString>();
	tableHeader->append("Account");
	tableHeader->append("Fingerprint");
	selectedAccount = NULL;
}

PrivKeyTableModel::PrivKeyTableModel(OtrConnection* aOtrConnection) : QAbstractTableModel(0) {
	otrConnection = aOtrConnection;
	otrConnection->getPrivateKeys();

	tableData = new QHash<QString, QString>();
	tableData = otrConnection->getPrivateKeys();

	tableHeader = new QList< QString>();
	tableHeader->append("Account");
	tableHeader->append("Fingerprint");
	selectedAccount =  NULL;
}


QVariant PrivKeyTableModel::data( const QModelIndex & index, int role ) const {
	if (!index.isValid()) {
		return QVariant();
	}
	if (role != Qt::DisplayRole) {
        	return QVariant();
	}
	if (index.column() == 0) {
		return tableData->keys().at( index.row() );
	}
	else if (index.column() == 1) {
		return tableData->values().at( index.row() );
	}
	return QVariant();   
}


int PrivKeyTableModel::rowCount ( const QModelIndex & parent ) const {
	Q_UNUSED(parent);
	return tableData->size();
}


int PrivKeyTableModel::columnCount ( const QModelIndex & parent ) const {
	Q_UNUSED(parent);
	return 2;
}



QVariant PrivKeyTableModel::headerData ( int section, Qt::Orientation orientation, int role ) const {
	if (role ==Qt::DisplayRole ) {
		if (orientation == Qt::Vertical) {
			return QString::number(section+1, 10);
		}
		else {
			return tableHeader->at(section);
		}
	}
	else {
		return QVariant();
	}
}

void PrivKeyTableModel::tableClicked(const QModelIndex & index ) {
	selectedAccount =  new QString(tableData->keys().at( index.row()));
}

void PrivKeyTableModel::deleteKeyClicked() {
	if (selectedAccount != NULL) {
		return;
	}
	QString msgText = "Do you really want to delete the private key for account "
				+ *selectedAccount + " ?";

	QMessageBox* d = new QMessageBox(QMessageBox::Information,
					 "psi-otr",
					 msgText,
					 QMessageBox::Yes | QMessageBox::No,
					 NULL,
					 Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	int ret = d->exec();
	if (ret == QMessageBox::Yes) {
		otrConnection->deletePrivateKey(*selectedAccount);
		tableData->take(*selectedAccount);
		emit layoutChanged();
		selectedAccount = NULL;
	}
}
