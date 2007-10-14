/*
 * privkeywidget.cpp - widget for private key configuration.
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


#include<QtGui>

#include "privkeywidget.h"

PrivKeyWidget::PrivKeyWidget(PrivKeyTableModel* aModel) {
	tableModel = aModel;

	QVBoxLayout* mainVLa = new QVBoxLayout();
	QHBoxLayout* buttonHLa = new QHBoxLayout();

	
	QPushButton* delKeyButton = new QPushButton("Delete Key");
	//buttonHLa->addWidget(delKeyButton);
	connect(delKeyButton, SIGNAL(clicked()),
		tableModel, SLOT(deleteKeyClicked()));

	QLabel* l = new QLabel("My private keys:");
	mainVLa->addWidget(l);
	
	QTableView* fpTable = new QTableView();
	fpTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	
	connect(fpTable, SIGNAL(clicked(QModelIndex)),
		tableModel, SLOT(tableClicked(QModelIndex)));
		

	fpTable->setModel(tableModel);
	fpTable->setColumnWidth(0, 250);
	fpTable->setColumnWidth(1, 360);
	mainVLa->addWidget(fpTable);
	mainVLa->addLayout(buttonHLa);

	setLayout(mainVLa);
}



