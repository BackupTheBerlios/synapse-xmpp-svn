#include "Item.h"
#include "Account.h"

#ifndef LIST_H
#define LIST_H

namespace SIMContactList {

class View;

class List : public QObject {
	Q_OBJECT
	friend class Model;
public:
	List(QObject *);
	~List();

	Item *rootItem();
	Item *invisibleGroup();
	Item *searchGroup();

	Item *findItem(const QString &name, int _type);
	Contact *findEntry(const QString &j, bool self = false);

	const QString &search();
	void setSearch(const QString& search);

	bool isGroupOpen(const QString&);
	void setGroupOpen(const QString&, bool);

	bool showOffline();
	bool showAway();
	bool showAgents();
	bool showSelf();
	bool showGroups();
	bool showAccounts();

	void updateVisibleParents();
	void updateInvisibleParents();
	void updateSearchParents();
	void updateParents();

	Account *addAccount(PsiAccount *pa);

	int avatarSize();


	View *contactListView();
	void setContactListView(View *clv);

	QMap<QString,bool> groupStates;

public slots:
	void setShowOffline(bool);
	void setShowAway(bool);
	void setShowAgents(bool);
	void setShowSelf(bool);
	void setShowGroups(bool);

	void updateOptions();
	void dataChanged();
	void contactBlocked(const QString &jid, bool blocked);
signals:
	void showOffline(bool);
	void showAway(bool);
	void showAgents(bool);
	void showSelf(bool);
	void showGroups(bool);

	void s_dataChanged();

private:
	Item *root_;
	Item *invisibleGroup_;
	Item *searchGroup_;
	View *contactListView_;

	QString search_;
	bool showOffline_;
	bool showAway_;
	bool showAgents_;
	bool showSelf_;
	bool showGroups_;
	bool showAccounts_;
};

};

#endif

