#ifndef NAME_H
#define NAME_H

#include <QString>
#include <QRect>
#include <QVariant>
#include <QFont>
#include <QColor>
#include <QTextDocument>

class QPainter;

namespace SIMContactList {

class View;

class Name {
public:
	enum EditMode { Editable, ReadOnly };

	Name();
	Name(const QString &txt, const QColor &color, SIMContactList::View *clv);
	
	void setText(const QString &txt, QColor color, SIMContactList::View *clv);

	void paint(QPainter *painter, const QRect &rect, const QPalette &palette, EditMode mode) const;
	QSize sizeHint( const QRect &rect) const;

private:
	QColor color_;
	QTextDocument *v_rt;
	SIMContactList::View *clv_;
};

};

Q_DECLARE_METATYPE(SIMContactList::Name);

#endif
