#include <QObject>
#include <QWidget>
#include <QTimer>

#ifndef HOVERLABEL_H
#define HOVERLABEL_H

class HoverLabel : public QWidget {
	Q_OBJECT
public:
	HoverLabel(QWidget *parent=0, bool _top=true);

public slots:
	void setText(const QString &text);
	QSize sizeForFont() const;
	QSize sizeHint() const;
	void updateSize();
	void resetAnimation();

	bool top();

protected:
	void paintEvent(QPaintEvent *e);

private:
	QSize interpolate(const QSize &src, const QSize &dst, qreal percent);

	QString text_;
	bool    animating_;
	QTimer  timer_;
	QTimer  hideTimer_;
	QSize   oldSize_;
	QSize   newSize_;
	qreal   percent_;
	bool	top_;
};

#endif
