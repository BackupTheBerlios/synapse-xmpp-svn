#include "Delegate.h"
#include "Name.h"

#include <QModelIndex>
#include <QPainter>

using namespace SIMContactList;

void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(!painter->isActive())
		return;
	if (qVariantCanConvert<Name>(index.data())) {
		Name cn = qVariantValue<Name>(index.data());

		if(option.state & QStyle::State_Selected)
			painter->fillRect(option.rect, option.palette.highlight());

		cn.paint(painter, option.rect, option.palette, Name::ReadOnly);
	} else {
		QItemDelegate::paint(painter, option, index);
	}
}

QWidget *Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QItemDelegate::createEditor(parent, option, index);
}

QSize Delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (qVariantCanConvert<Name>(index.data())) {
		Name cn = qVariantValue<Name>(index.data());
		return cn.sizeHint(option.rect);
	} else
		return QItemDelegate::sizeHint(option, index);
}
