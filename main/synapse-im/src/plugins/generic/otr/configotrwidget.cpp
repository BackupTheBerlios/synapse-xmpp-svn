/*
 * configotrwidget.cpp - widget for otr configuration.
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

#include "configotrwidget.h"

ConfigOtrWidget::ConfigOtrWidget() {
	QVBoxLayout* mainVLa = new QVBoxLayout(); 

	QLabel* label = new QLabel("OTR Configuration:");
	mainVLa->addWidget(label);
	
	QGroupBox* policyBox = new QGroupBox("OTR-Policy");
	QVBoxLayout* policyBoxLayout = new QVBoxLayout();
	
	pol1 = new QCheckBox("Enable private messaging");
	pol1->setEnabled(true);
	connect(pol1,SIGNAL(stateChanged(int)),SLOT(policyEnabledChanged(int)));
	
	pol2 = new QCheckBox("Automatically start private messaging");
	pol2->setEnabled(false);
	connect(pol2,SIGNAL(stateChanged(int)),SLOT(policyAutoChanged(int)));
	
	pol3 = new QCheckBox("Require private messaging");
	pol3->setEnabled(false);
	connect(pol3,SIGNAL(stateChanged(int)),SLOT(policyRequireChanged(int)));
	
	policyBoxLayout->addWidget(pol1);
	policyBoxLayout->addWidget(pol2);
	policyBoxLayout->addWidget(pol3);
	policyBox->setLayout(policyBoxLayout);

	mainVLa->addWidget(policyBox);
	mainVLa->addStretch();

	setLayout(mainVLa);
}


void ConfigOtrWidget::setOtrPolicy(int p) {
	if (p == 1) {
		pol1->setCheckState(Qt::Checked);
		pol2->setCheckState(Qt::Unchecked);
		pol3->setCheckState(Qt::Unchecked);
		pol1->setEnabled(true);
		pol2->setEnabled(true);
		pol3->setEnabled(false);
	}
	else if (p == 2) {
		pol1->setCheckState(Qt::Checked);
		pol2->setCheckState(Qt::Checked);
		pol3->setCheckState(Qt::Unchecked);
		pol1->setEnabled(true);
		pol2->setEnabled(true);
		pol3->setEnabled(true);
	}
	else if (p == 3) {
		pol1->setCheckState(Qt::Checked);
		pol2->setCheckState(Qt::Checked);
		pol3->setCheckState(Qt::Checked);
		pol1->setEnabled(true);
		pol2->setEnabled(true);
		pol3->setEnabled(true);
	}
	else {
		pol1->setCheckState(Qt::Unchecked);
		pol2->setCheckState(Qt::Unchecked);
		pol3->setCheckState(Qt::Unchecked);
		pol1->setEnabled(true);
		pol2->setEnabled(false);
		pol3->setEnabled(false);
	}
}



/* slots */
void ConfigOtrWidget::policyEnabledChanged(int state) {
	if (state == Qt::Checked) {
		pol2->setEnabled(true);
		pol2->setEnabled(true);
		emit activatePolicy(ENABLED);
	}
	else {
		pol2->setEnabled(false);
		pol3->setEnabled(false);
		emit deactivatePolicy(ENABLED);
	}
}

void ConfigOtrWidget::policyAutoChanged(int state) {
	if (state ==Qt::Checked) {
		pol3->setEnabled(true);
		emit activatePolicy(AUTO);
	}
	else {
		pol3->setEnabled(false);
		emit deactivatePolicy(AUTO);
	}
}

void ConfigOtrWidget::policyRequireChanged(int state) {
	if (state ==Qt::Checked) {
		emit activatePolicy(REQUIRE);
	}
	else {
		emit deactivatePolicy(REQUIRE);
	}
}


