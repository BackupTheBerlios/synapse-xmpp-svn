#ifndef MODEL_H
#define MODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

namespace SIMContactList {

class List;
class Item;
class Model : public QAbstractItemModel
{
	Q_OBJECT
public:
	enum {
		ExpandedRole = Qt::UserRole + 0,
		ContextMenuRole = Qt::UserRole + 1,
		ToolTipRole = Qt::UserRole + 2
	};

	enum {
		StateColumn = 0,
		PixmapColumn = 1,
		NameColumn = 2,
		AvatarColumn = 3
	};

	Model(SIMContactList::List *contactList);

	QVariant data(const QModelIndex &index, int role) const;
	virtual bool setData(const QModelIndex&, const QVariant&, int role);
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index( int row, int column, SIMContactList::Item *item) const;
	virtual QModelIndex index( int row, int column, const QModelIndex &parent) const;
	virtual QModelIndex parent(const QModelIndex &index) const;
	virtual int rowCount(const QModelIndex &parent) const;
	virtual int columnCount(const QModelIndex &parent) const;

public slots:
	void contactList_changed();

private:
	SIMContactList::List *contactList_;
};

};

#endif
