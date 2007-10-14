/*
 * configotrwidget.h - widget for otr configuration.
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

#ifndef  CONFIGOTRWIDGET_INC
#define  CONFIGOTRWIDGET_INC

#include<QtGui>

class QWidget;

enum OtrPolicy {
	OFF,
	ENABLED,
	AUTO,
	REQUIRE,
};

class ConfigOtrWidget : public QWidget {
	Q_OBJECT

	public:
	ConfigOtrWidget();
	void setOtrPolicy(int p);
	int getOtrPolicy();

	private:
	QCheckBox* pol1;
	QCheckBox* pol2;
	QCheckBox* pol3;


	private slots:
	void policyEnabledChanged(int state);
	void policyAutoChanged(int state);
	void policyRequireChanged(int state);

	signals:
	void activatePolicy(OtrPolicy p);
	void deactivatePolicy(OtrPolicy p);



};




#endif   /* ----- #ifndef CONFIGOTRWIDGET_INC  ----- */
