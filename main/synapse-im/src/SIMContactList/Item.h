#ifndef ITEM_H
#define ITEM_H

#include <QObject>
#include <QVariant>

class PsiAccount;

namespace SIMContactList {

class List;
class Contact;
class Group;

class Item {
public:
	enum {
		TRoot = 0,
		TAccount = 1,
		TGroup = 2,
		TMeta = 3,
		TContact = 4
	};

	Item(int typ, PsiAccount *_pa, List *cl, Item *parent = 0);
	~Item();

	PsiAccount *account();

	void appendChild(Item *child);
	void removeChild(Item *child);

	Item *findItem(const QString &name, int _type);
	SIMContactList::Contact *findEntry(const QString &j, bool self = false);

	Item *child(int row);
	int type();
	int size() const;
	int columnCount() const;

	const QVariant &data(int column) const;
	int row() const;

	Item *parent();
	List *contactList();
	Item *defaultParent();
	void setDefaultParent(Item* parent);

	virtual void showContextMenu(const QPoint&);

	void setParent(Item *parent);
	void updateParents();
	void updateParent(bool reload = false);
	void updateOptions();

private:
	static int compare_invisible(Item *it1, Item *it2);
	static int compare(Item *it1, Item *it2);

	QList<Item*> childItems;
	int type_;
	Item *parentItem;
	Item *defaultParent_;
	List *contactList_;

	PsiAccount *pa;
};

};

#endif
