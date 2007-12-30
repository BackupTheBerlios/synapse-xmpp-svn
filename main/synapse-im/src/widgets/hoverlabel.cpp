#include "hoverlabel.h"
#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>

HoverLabel::HoverLabel(QWidget *parent,int _pos, int timeout)
: QWidget(parent), animating_(false), percent_(0), pos_(_pos)
{
	timer_.setInterval(1000/30);
	hideTimer_.setInterval(timeout);
	hideTimer_.setSingleShot(true);
	connect(&timer_, SIGNAL(timeout()), this, SLOT(update()));
	connect(&hideTimer_, SIGNAL(timeout()), this, SLOT(hide()));
}

void HoverLabel::setText(const QString &text)
{
	text_ = text;
	if (text_.isEmpty()) {
		hideTimer_.start();
	} else {
		hideTimer_.stop();
		oldSize_ = newSize_;
		newSize_ = sizeForFont();
		resetAnimation();
		updateSize();
		show();
		repaint();
	}
}

/*void HoverLabel::setSize(const QSize &s)
{
	staticSize_ = s;
	oldSize_ = s;
	newSize_ = s;
	updateSize();
	show();
	repaint();
	hideTimer_.start();
}*/

QSize HoverLabel::sizeForFont() const
{
	QFont f = font();
	QFontMetrics fm(f);
	return QSize(fm.width(text_) + 10, fm.height() + 6);
}

QSize HoverLabel::sizeHint() const
{
//	if (!staticSize_.isNull())
//		return staticSize_;
	if (!animating_)
		return sizeForFont();
	else
		return (newSize_.width() > oldSize_.width()) ? newSize_ : oldSize_;
}

void HoverLabel::updateSize()
{
/*	QRect r = geometry();
	QSize newSize = sizeHint();
	r = QRect(r.x(), r.y(), newSize.width(), newSize.height());
	setGeometry(r);*/
	resizeEvent(QWidget::parentWidget());
}

void HoverLabel::resetAnimation()
{
	animating_ = true;
	percent_ = 0;
	if (!timer_.isActive())
		timer_.start();
}

int HoverLabel::position()
{
	return pos_;
}

void HoverLabel::mousePressEvent(QMouseEvent *e)
{
	emit clicked();
}

void HoverLabel::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	p.setClipRect(e->rect());
	p.setPen(QPen(Qt::black, 1));
	QLinearGradient gradient(rect().topLeft(), rect().bottomLeft());
	gradient.setColorAt(0, QColor(255, 255, 255, 220));
	gradient.setColorAt(1, QColor(193, 193, 193, 220));
	p.setBrush(QBrush(gradient));
	QSize size;
	{
	//draw a nicely rounded corner rectangle. to avoid unwanted
	// borders we move the coordinates outsize the our clip region
		size = interpolate(oldSize_, newSize_, percent_);
		const int roundness = 20;
		QPainterPath path;
		if(pos_ == TopLeft) {
			QRect r(-1, -2, size.width(), size.height());
			path.moveTo(r.left(), r.bottom());
			path.lineTo(r.right()-roundness, r.bottom());
			path.cubicTo(r.right(), r.bottom(), r.right(), r.bottom(), r.right(), r.bottom()-roundness);
			path.lineTo(r.topRight());
			path.lineTo(r.topLeft());
		} 
		else if(pos_ == TopRight) {
			QRect r(0, -2, size.width()+1, size.height());
			path.moveTo(r.right(), r.bottom());
			path.lineTo(r.left()+roundness, r.bottom());
			path.cubicTo(r.left(), r.bottom(), r.left(), r.bottom(), r.left(), r.bottom()-roundness);
			path.lineTo(r.topLeft());
			path.lineTo(r.topRight());
		}
		else if(pos_ == BottomLeft) {
			QRect r(-1, 0, size.width(), size.height()+2);
			path.moveTo(r.left(), r.top());
			path.lineTo(r.right()-roundness, r.top());
			path.cubicTo(r.right(), r.top(), r.right(), r.top(), r.right(), r.top()+roundness);
			path.lineTo(r.bottomRight());
			path.lineTo(r.bottomLeft());
		}
		else if(pos_ == BottomRight) {
			QRect r(0, 0, size.width()+1, size.height()+2);
			path.moveTo(r.right(), r.top());
			path.lineTo(r.left()+roundness, r.top());
			path.cubicTo(r.left(), r.top(), r.left(), r.top(), r.left(), r.top()+roundness);
			path.lineTo(r.bottomLeft());
			path.lineTo(r.bottomRight());
		}
		path.closeSubpath();
		p.setRenderHint(QPainter::Antialiasing);
		p.drawPath(path);
	}
	if (animating_) {
		if (qFuzzyCompare(percent_, 1)) {
		animating_ = false;
		percent_ = 0;
		timer_.stop();
		} else {
			percent_ += 0.1;
			if (percent_ >= 0.99) {
				percent_ = 1;
			}
		}
	}

	if(!text_.isEmpty()) {
		QString txt;
		QFontMetrics fm(fontMetrics());
		txt = fm.elidedText(text_, Qt::ElideRight, size.width()-5);
		p.drawText(5, height()-6, txt);
	}
}

QSize HoverLabel::interpolate(const QSize &src, const QSize &dst, qreal percent)
{
	int widthDiff  = int((dst.width() - src.width())  * percent);
	int heightDiff = int((dst.height() - src.height()) * percent);
	return QSize(src.width()  + widthDiff, src.height() + heightDiff);
}

void HoverLabel::resizeEvent(QWidget *w)
{
	QSize hs = sizeHint();
	if(pos_ == TopLeft)
		setGeometry(0, 0, hs.width(), hs.height());
	else if(pos_ == TopRight)
		setGeometry(w->width() - hs.width(), 0, hs.width(), hs.height());
	else if(pos_ == BottomLeft)
		setGeometry(0, w->height() - hs.height(), hs.width(), hs.height());
	else if(pos_ == BottomRight)
		setGeometry(w->width() - hs.width(), w->height() - hs.height(), hs.width(), hs.height());
}

#include "hoverlabel.moc"
