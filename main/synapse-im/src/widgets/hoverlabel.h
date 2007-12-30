#include <QObject>
#include <QWidget>
#include <QTimer>

#ifndef HOVERLABEL_H
#define HOVERLABEL_H

class HoverLabel : public QWidget {
	Q_OBJECT
public:
	enum Position {
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};

	HoverLabel(QWidget *parent=0, int _pos = TopLeft, int timeout = 500);

signals:
	void clicked();

public slots:
	void setText(const QString &text);
	QSize sizeForFont() const;
	QSize sizeHint() const;
	void updateSize();
	void resetAnimation();

	int position();
	void resizeEvent(QWidget *w);

protected:
	void mousePressEvent(QMouseEvent *e);
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
	int	pos_;
};

#endif
