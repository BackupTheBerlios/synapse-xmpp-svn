/*
 * fancypopup.cpp - the FancyPopup passive popup widget
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

#include "fancypopup.h"

#include <QPixmap>
#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QTimer>
#include <QPainter>
#include <QBitmap>
#include <QList>
#include <QToolButton>
#include <QStyle>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QList>

#include "iconset.h"
#include "fancylabel.h"
#include "iconlabel.h"
#include "psitooltip.h"

#define BUTTON_WIDTH	10
#define BUTTON_HEIGHT	10

//----------------------------------------------------------------------------
// FancyPopup
//----------------------------------------------------------------------------

FancyPopup::FancyPopup(QWidget *parent, QString title, const PsiIcon *icon, int timeout, bool copyIcon)
 : QWidget( parent, "Synapse-IM OSD", Qt::WType_TopLevel | Qt::WNoAutoErase | Qt::WStyle_Customize | Qt::WX11BypassWM | Qt::WStyle_StaysOnTop | Qt::WStyle_Tool ), m_timer(new QTimer()), m_timeout(timeout), m_prl(NULL), m_title(title)
{
	QWidget::setAttribute(Qt::WA_DeleteOnClose);
	setFocusPolicy(Qt::NoFocus);
	setBackgroundRole(QPalette::Midlight);
	setAutoFillBackground(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(hide()));

	resize(500,100);
}

FancyPopup::~FancyPopup()
{
	if(m_timer != NULL) delete m_timer;
	if(m_prl != NULL) delete m_prl;
}

void FancyPopup::setData(QPixmap pix, PsiRichLabel *prl)
{
	m_logo = pix;
	m_prl = prl;
	resize(dimensions());
}

void FancyPopup::show()
{
	setup();
	restartHideTimer();
	QWidget::show();
printf("done\n");
}

QSize FancyPopup::dimensions()
{
	m_titleSize = fontMetrics().boundingRect( 0, 0,
            500, fontMetrics().height(), Qt::AlignCenter | Qt::WordBreak, m_title );
	int width = m_logo.width() + ((m_titleSize.width()>m_prl->sizeHint().width()) ? m_titleSize.width() : m_prl->sizeHint().width());
	width += MARGIN;
	int text_height = m_prl->sizeHint().height() + m_titleSize.height() + 4;
	int height = MARGIN + ((m_logo.height()>text_height) ? m_logo.height() : text_height);
	return QSize(width, height);
}

void FancyPopup::paintEvent(QPaintEvent *pe)
{
	QPainter p((QWidget*)this);
	QRect rect(pe->rect());
	p.setClipRect(rect);
	p.fillRect( rect, backgroundColor() );

	p.save();
	p.setBrush(Qt::white);
	p.drawRoundRect(rect, xround, yround);
	p.restore();

	rect.adjust(2,2,-2,-2);
	p.setBrush(backgroundColor());
	p.drawRoundRect(rect, xround, yround);

	p.setPen( backgroundColor().dark() );
	p.drawRoundRect( rect, xround, yround );

	const uint METRIC = fontMetrics().width('X');
	rect.adjust(METRIC, METRIC, -METRIC, -METRIC);
/// Paint logo
	p.translate(rect.topLeft());
	p.drawPixmap(logo_rect, m_logo);
	rect.adjust(m_logo.width() + METRIC, 0,0,0);

/// Paint reason
	p.translate(m_logo.width() + METRIC, 0);
	p.setPen( foregroundColor() );
	p.setFont( font() );
	p.drawText( QRect(0,0,m_titleSize.width(), m_titleSize.height()), Qt::AlignTop| Qt::WordBreak, m_title );

/// Paint contact info
	p.translate(0, m_titleSize.height());
	m_prl->paintThere(&p,rect);
}

void FancyPopup::setup()
{
	QSize size = dimensions();

	QPoint point( MARGIN, MARGIN );
	const QRect screen = QApplication::desktop()->screenGeometry( 0 );
	point.rx() = screen.width() - MARGIN - size.width();
	point.ry() = screen.height() - MARGIN - size.height();
	point += screen.topLeft();

	QRect rect(point, size);
	const uint METRIC = fontMetrics().width('X');
	xround = (METRIC * 200) / size.width();
	yround = (METRIC * 200) / size.height();

	logo_rect = QRect(0,0,m_logo.width(),m_logo.height());
	setGeometry(rect);
}

void FancyPopup::hideEvent(QHideEvent *e)
{
	m_timer->stop();
	deleteLater();

	QWidget::hide();
}

void FancyPopup::mouseReleaseEvent(QMouseEvent *e)
{
	if (!isVisible())
		return;

	emit clicked((int)e->button());
	hide();
}

void FancyPopup::restartHideTimer()
{
	m_timer->start(m_timeout);
}

#include "fancypopup.moc"
