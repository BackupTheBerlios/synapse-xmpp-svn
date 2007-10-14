/*
 * fingerprintwidget.cpp - widget for fingerprint configuration.
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

#include<QtGui>

#include "fingerprintwidget.h"

FingerprintWidget::FingerprintWidget(QList< Fprint > aFpTableContent)  {
	fpTableContent = aFpTableContent;
	QVBoxLayout* mainVLa = new QVBoxLayout();
	QHBoxLayout* buttonHLa = new QHBoxLayout();

	
	QPushButton* forgetButton = new QPushButton("forget fingerprint");
	QPushButton* verifyButton = new QPushButton("verify fingerprint");
	buttonHLa->addWidget(forgetButton);
	buttonHLa->addWidget(verifyButton);
	connect(forgetButton,SIGNAL(clicked()),SLOT(forgetFingerprint()));
	connect(verifyButton,SIGNAL(clicked()),SLOT(verifyFingerprint()));


	QLabel* l = new QLabel("Fingerprints");
	mainVLa->addWidget(l);

	fpTable = new QTableView();
	fpTable->setShowGrid(true);
	fpTable->setEditTriggers(0);
	fpTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	connect(fpTable, 
		SIGNAL(clicked(QModelIndex)),
		SLOT(tableClicked(QModelIndex)) );

	model =	new QStandardItemModel();
	model->setColumnCount(5);
	
	QStringList header;
	header << "account" << "buddy" << "fingerprint" << "verified" << "status";

	model->setHorizontalHeaderLabels(header);
	
	QListIterator< Fprint > fpIterator(fpTableContent);
	while(fpIterator.hasNext()) {
		model->appendRow(fpIterator.next().row());
	}
	tableIndex = -1;
	fpTable->setModel(model);
	fpTable->setColumnWidth(0, 250);
	fpTable->setColumnWidth(1, 250);
	fpTable->setColumnWidth(2, 360);
	fpTable->setColumnWidth(3, 60);
	fpTable->setColumnWidth(4, 80);

	mainVLa->addWidget(fpTable);
	mainVLa->addLayout(buttonHLa);

	setLayout(mainVLa);
}



//** slots **

void FingerprintWidget::forgetFingerprint() {
	if (tableIndex == -1) {
		return;
	}
	QString msgText = "Are you sure you want to delete the fingerprint:\naccount: "
		+ model->item(tableIndex,0 )->text() + "\n" +
		"buddy: " + model->item(tableIndex, 1)->text() + "\n" +
		"fingerprint: " + model->item(tableIndex, 2)->text();
	
	QMessageBox* d = new QMessageBox(QMessageBox::Information,
					 "psi-otr",
					 msgText,
					 QMessageBox::Yes | QMessageBox::No,
					 NULL,
					 Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	int ret = d->exec();
	if (ret == QMessageBox::Yes) {
		emit forgetFingerprint((unsigned char*) fpTableContent.at(tableIndex).getFingerprint());
		model->removeRow(tableIndex);
		fpTableContent.removeAt(tableIndex);
		tableIndex = -1;
 	}
	else if (ret  == QMessageBox::No) {
 	}
}

void FingerprintWidget::verifyFingerprint() {
	if (tableIndex == -1) {
		return;
	}
	QString msgText = "User: " + model->item(tableIndex, 1)->text() + "\n" +
		"Fingerprint: " + model->item(tableIndex, 2)->text() + "\n\n" +
		"Have you verified that this is in fact the correct fingerprint?";


	QMessageBox* d = new QMessageBox(QMessageBox::Information,
					 "Verify fingerprint",
					 msgText,
					 QMessageBox::Yes | QMessageBox::No,
					 NULL,
					 Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	int ret = d->exec();
	if (ret == QMessageBox::Yes) {
		emit verifyFingerprint( (unsigned char*) fpTableContent.at(tableIndex).getFingerprint(), 
				true);
		model->setItem(tableIndex, 3, new QStandardItem("verified"));
	}
	else if (ret == QMessageBox::No) {
		emit verifyFingerprint( (unsigned char*) fpTableContent.at(tableIndex).getFingerprint(), 
				false);
		model->setItem(tableIndex, 3, new QStandardItem(""));
	}
}

void FingerprintWidget::tableClicked(const QModelIndex & index ) {
	tableIndex = index.row();
}

