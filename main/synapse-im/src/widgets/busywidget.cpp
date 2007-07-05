/*
 * busywidget.cpp - cool animating widget
 * Copyright (C) 2007	Andrzej Wójcik
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

#include "busywidget.h"

/////////////////////////////////////////////////////////////////////////////
// code
//
BusyWidget::BusyWidget(QWidget *parent)
:QProgressBar(parent)
{
	setRange(0,100);
	setValue(100);
}

BusyWidget::~BusyWidget()
{
}

QSize BusyWidget::minimumSizeHint() const
{
	return QSize( 82, 19 );
}

QSize BusyWidget::sizeHint() const
{
	return minimumSizeHint();
}

bool BusyWidget::isActive() const
{
	return (value()!=100);
}

void BusyWidget::setActive(bool a)
{
	if ( a )
		start();
	else
		stop();
}

void BusyWidget::start()
{
	setTextVisible(false);
	setRange(0,0);
	setValue(1);
}

void BusyWidget::stop()
{
	setRange(0,100);
	setTextVisible(true);
	setValue(100);
}

