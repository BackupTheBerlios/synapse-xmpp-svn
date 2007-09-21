#ifndef SIMCONTACTLISTITEM_H
#define SIMCONTACTLISTITEM_H

#include <QObject>
#include <QVariant>

class PsiAccount;
class SIMContactList;
class SIMContactListContact;
class SIMContactListGroup;

class SIMContactListItem {
public:
	enum {
		Root = 0,
		Account = 1,
		Group = 2,
		Meta = 3,
		Contact = 4
	};

	SIMContactListItem(int typ, PsiAccount *_pa, SIMContactList *cl, SIMContactListItem *parent = 0);
	~SIMContactListItem();

	PsiAccount *account();

	void appendChild(SIMContactListItem *child);
	void removeChild(SIMContactListItem *child);

	SIMContactListItem *findItem(const QString &name, int _type);
	SIMContactListContact *findEntry(const QString &j, bool self = false);

	SIMContactListItem *child(int row);
	int type();
	int size() const;
	int columnCount() const;

	const QVariant &data(int column) const;
	int row() const;

	SIMContactListItem *parent();
	SIMContactList *contactList();
	SIMContactListItem *defaultParent();
	void setDefaultParent(SIMContactListItem* parent);

	virtual void showContextMenu(const QPoint&);

	void setParent(SIMContactListItem *parent);
	void updateParents();
	void updateParent();
	void updateOptions();

private:
	static int compare_invisible(SIMContactListItem *it1, SIMContactListItem *it2);
	static int compare(SIMContactListItem *it1, SIMContactListItem *it2);

	QList<SIMContactListItem*> childItems;
	int type_;
	SIMContactListItem *parentItem;
	SIMContactListItem *defaultParent_;
	SIMContactList *contactList_;

	PsiAccount *pa;
};

#endif
