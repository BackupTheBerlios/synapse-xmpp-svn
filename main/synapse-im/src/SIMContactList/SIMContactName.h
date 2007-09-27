#ifndef SIMCONTACTNAME_H
#define SIMCONTACTNAME_H

#include <QString>
#include <QRect>
#include <QVariant>
#include <QFont>
#include <QColor>
#include <QTextDocument>

class SIMContactListView;

class QPainter;

class SIMContactName {
public:
	enum EditMode { Editable, ReadOnly };

	SIMContactName();
	SIMContactName(const QString &txt, const QColor &color, SIMContactListView *clv);
	
	void setText(const QString &txt, const QColor &color, SIMContactListView *clv);

	void paint(QPainter *painter, const QRect &rect, const QPalette &palette, EditMode mode) const;
	QSize sizeHint( const QRect &rect) const;

private:
	QColor color_;
	QTextDocument *v_rt;
	SIMContactListView *clv_;
};

Q_DECLARE_METATYPE(SIMContactName);

#endif
