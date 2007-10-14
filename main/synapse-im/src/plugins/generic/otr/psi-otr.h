/*
 * psi-otr.h - off-the-record messaging plugin for psi
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


#ifndef PSITESTPLUGIN_H_
#define PSITESTPLUGIN_H_

#include <QObject>
#include <QtGui>

#include "psiplugin.h"

#include "otrconnection.h"
#include "configdlg.h"

class OtrConnection;
class ConfigDlg;

class PsiOtrPlugin : public PsiPlugin {
	
	Q_OBJECT
	Q_INTERFACES(PsiPlugin)
    
	public:
	PsiOtrPlugin();
	QString name() const;
	QString shortName() const;
	QString version() const;
	QString incomingMessage( const QString& fromJid,
			      const QString& toJid, 
			      const QString& message);


	QDomElement incomingMessage( const QString& fromJid,
			      const QString& toJid, 
			      const QDomElement& html);

	QString outgoingMessage(const QString& fromJid, 
				const QString& toJid, 
				const QString& message);
	void sendMessage(const char* fromJid, const char* toJid, const char* message);
	QWidget* options(); 
	QString homeDir();
	void init();
	QVariant getPsiOption(const QString& option);

	private:
	OtrConnection* otrConnection;
	ConfigDlg* configDialog;
	QString removeResourceFromJid(const QString& jid);

	private slots:
	void savePsiOption(QString option, QVariant value );
	
};

#endif /*PSITESTPLUGIN_H_*/
