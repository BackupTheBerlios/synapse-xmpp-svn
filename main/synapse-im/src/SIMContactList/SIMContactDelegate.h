#ifndef SIMCONTACTDELEGATE_H
#define SIMCONTACTDELEGATE_H

#include <QItemDelegate>
class QAbstractModelItem;

class SIMContactDelegate : public QItemDelegate {
	Q_OBJECT
public:
	SIMContactDelegate(QWidget *parent = 0) : QItemDelegate(parent) {}

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setEditorData(QWidget *editor, const QModelIndex &index) const {};
	void setModelData(QWidget *editor, QAbstractModelItem *model, const QModelIndex &index) const {};

};

#endif
