/*
 * configdlg.cpp - dialog for plugin configuration.
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

#include "configdlg.h"

ConfigDlg::ConfigDlg(OtrConnection* aOtrConnection)  {
	otrConnection = aOtrConnection;

	fingerprintWidget = new FingerprintWidget(otrConnection->getFingerprints());
	connect(fingerprintWidget, SIGNAL(forgetFingerprint(unsigned char*)),
		this, SLOT(forgetFingerprint(unsigned char*)));
	connect(fingerprintWidget, SIGNAL(verifyFingerprint(unsigned char*, bool)),
		this, SLOT(verifyFingerprint(unsigned char*, bool)));

	privKeyWidget = new PrivKeyWidget(otrConnection->getPrivateKeysModel());

	configWidget = new ConfigOtrWidget();
	connect( configWidget, SIGNAL(activatePolicy(OtrPolicy)),
		this, SLOT(activatePolicy(OtrPolicy)));
	connect( configWidget, SIGNAL(deactivatePolicy(OtrPolicy)),
		this, SLOT(deactivatePolicy(OtrPolicy)));
	

	tabWidget = new QTabWidget();
	tabWidget->addTab(fingerprintWidget, "Known fingerprints");
	tabWidget->addTab(privKeyWidget, "My private keys");
	tabWidget->addTab(configWidget, "Config");

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	setLayout(mainLayout);
}


void ConfigDlg::setOtrPolicy(int p) {
	if (configWidget != NULL) {
		static_cast<ConfigOtrWidget*>(configWidget)->setOtrPolicy(p);
	}
}



//** slots **
void ConfigDlg::forgetFingerprint(unsigned char* fp) {
	otrConnection->deleteFingerprint(fp);
}

void ConfigDlg::verifyFingerprint(unsigned char* fp, bool verified) {
	qWarning() << "verifyFingrprint";
	otrConnection->verifyFingerprint(fp, verified);
}

void ConfigDlg::activatePolicy(OtrPolicy p) {
	emit savePsiOption(PSI_CONFIG_POLICY, p);
}


void ConfigDlg::deactivatePolicy(OtrPolicy p) {
	emit savePsiOption(PSI_CONFIG_POLICY, p-1);
}



