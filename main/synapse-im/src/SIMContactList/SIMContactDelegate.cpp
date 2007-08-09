#include "SIMContactDelegate.h"
#include "SIMContactName.h"

#include <QModelIndex>
#include <QPainter>

void SIMContactDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(!painter->isActive())
		return;
	if (qVariantCanConvert<SIMContactName>(index.data())) {
		SIMContactName cn = qVariantValue<SIMContactName>(index.data());

		if(option.state & QStyle::State_Selected)
			painter->fillRect(option.rect, option.palette.highlight());

		cn.paint(painter, option.rect, option.palette, SIMContactName::ReadOnly);
	} else {
		QItemDelegate::paint(painter, option, index);
	}
}

QWidget *SIMContactDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QItemDelegate::createEditor(parent, option, index);
}

QSize SIMContactDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (qVariantCanConvert<SIMContactName>(index.data())) {
		SIMContactName cn = qVariantValue<SIMContactName>(index.data());
		return cn.sizeHint(option.rect);
	} else
		return QItemDelegate::sizeHint(option, index);
}
