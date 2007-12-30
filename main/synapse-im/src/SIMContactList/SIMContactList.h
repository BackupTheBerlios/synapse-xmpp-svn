#include "SIMContactListItem.h"
#include "SIMContactListAccount.h"

#ifndef SIMCONTACTLIST_H
#define SIMCONTACTLIST_H

class SIMContactListView;

class SIMContactList : public QObject {
	Q_OBJECT
	friend class SIMContactListModel;
public:
	SIMContactList(QObject *);
	~SIMContactList();

	SIMContactListItem *rootItem();
	SIMContactListItem *invisibleGroup();
	SIMContactListItem *searchGroup();

	SIMContactListItem *findItem(const QString &name, int _type);
	SIMContactListContact *findEntry(const QString &j, bool self = false);

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

	SIMContactListAccount *addAccount(PsiAccount *pa);

	int avatarSize();


	SIMContactListView *contactListView();
	void setContactListView(SIMContactListView *clv);

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
	SIMContactListItem *root_;
	SIMContactListItem *invisibleGroup_;
	SIMContactListItem *searchGroup_;
	SIMContactListView *contactListView_;

	QString search_;
	bool showOffline_;
	bool showAway_;
	bool showAgents_;
	bool showSelf_;
	bool showGroups_;
	bool showAccounts_;
};

#endif

