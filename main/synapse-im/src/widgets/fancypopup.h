/*
 * fancypopup.h - the FancyPopup passive popup widget
 * Copyright (C) 2003-2005  Michail Pishchagin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef FANCYPOPUP_H
#define FANCYPOPUP_H

#include <QWidget>
#include "psirichlabel.h"

class PsiIcon;
class QTimer;

class FancyPopup : public QWidget
{
	Q_OBJECT
public:
	FancyPopup(QWidget *parent, QString title, const PsiIcon *icon = 0, int timeout = 3000, bool copyIcon = true);
	~FancyPopup();

	void setData(QPixmap pix, PsiRichLabel *prl);

	void setup();
	void show();
	void restartHideTimer();

signals:
	void clicked(int);

protected:
	void hideEvent(QHideEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void paintEvent(QPaintEvent *);

	static const int MARGIN = 15;

public:
	class Private;
private:
	QSize dimensions();	

	QPixmap m_logo;
	QRect logo_rect;
	QTimer *m_timer;
	PsiRichLabel *m_prl;
	QString m_title;
	QRect m_titleSize;
	QSize size;
	int m_timeout;
	uint xround;
	uint yround;

};

#endif
