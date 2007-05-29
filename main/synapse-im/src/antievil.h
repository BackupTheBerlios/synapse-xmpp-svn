/*
 * antievil.h - anti evil scanner task
 * Copyright (C) 2007-04-01  Maciej Niedzielski
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef ANTIEVIL_H
#define ANTIEVIL_H

#include "xmpp_task.h"
#include <QDateTime>

class AntiEvil : public XMPP::Task
{
public:
	AntiEvil(Task *);
	~AntiEvil();
	bool take(const QDomElement &);

	class Stats;
	static const Stats* stats() { return &s; }

private:
	static Stats s;

};

class AntiEvil::Stats : public QObject
{
	Q_OBJECT
public:
	int scanned;
	int blocked;

	QString lastBlockedFrom;
	QDateTime lastBlockedTime;

signals:
	void scannedNext(int);
	void blockedNext(int, const QString&, const QDateTime&);

	friend class AntiEvil;
};

#endif


