#include "hoverlabel.h"
#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>

HoverLabel::HoverLabel(QWidget *parent,bool _top)
: QWidget(parent), animating_(false), percent_(0), top_(_top)
{
	timer_.setInterval(1000/30);
	hideTimer_.setInterval(500);
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

QSize HoverLabel::sizeForFont() const
{
	QFont f = font();
	QFontMetrics fm(f);
	return QSize(fm.width(text_) + 10, fm.height() + 6);
}

QSize HoverLabel::sizeHint() const
{
	if (!animating_)
		return sizeForFont();
	else
		return (newSize_.width() > oldSize_.width()) ? newSize_ : oldSize_;
}

void HoverLabel::updateSize()
{
	QRect r = geometry();
	QSize newSize = sizeHint();
	r = QRect(r.x(), r.y(), newSize.width(), newSize.height());
	setGeometry(r);
}

void HoverLabel::resetAnimation()
{
	animating_ = true;
	percent_ = 0;
	if (!timer_.isActive())
		timer_.start();
}

bool HoverLabel::top()
{
	return top_;
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
		if(top_) {
			QRect r(-1, -2, size.width(), size.height());
			path.moveTo(r.left(), r.bottom());
			path.lineTo(r.right()-roundness, r.bottom());
			path.cubicTo(r.right(), r.bottom(), r.right(), r.bottom(), r.right(), r.bottom()-roundness);
			path.lineTo(r.topRight());
			path.lineTo(r.topLeft());
		} else {
			QRect r(-1, 0, size.width(), size.height()+2);
			path.moveTo(r.left(), r.top());
			path.lineTo(r.right()-roundness, r.top());
			path.cubicTo(r.right(), r.top(), r.right(), r.top(), r.right(), r.top()+roundness);
			path.lineTo(r.bottomRight());
			path.lineTo(r.bottomLeft());
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

	QString txt;
	QFontMetrics fm(fontMetrics());
	txt = fm.elidedText(text_, Qt::ElideRight, size.width()-5);
	p.drawText(5, height()-6, txt);
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
	setGeometry(0, w->height() - hs.height(), 300, hs.height());
}

#include "hoverlabel.moc"
