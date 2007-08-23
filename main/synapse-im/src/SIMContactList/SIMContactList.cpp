#include "SIMContactList.h"
#include "SIMContactListView.h"
#include "SIMContactListItem.h"
#include "SIMContactListContact.h"
#include "SIMContactListGroup.h"
#include "SIMContactListModel.h"
#include "psiaccount.h"

SIMContactList::SIMContactList(QObject* parent)
: QObject(parent), showOffline_(false), showGroups_(true), showAccounts_(false), showAway_(true), showSelf_(true), showAgents_(true)
{
	root_ = new SIMContactListItem(SIMContactListItem::Root, 0, this);
	//connect(root_, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
	invisibleGroup_ = new SIMContactListItem(SIMContactListItem::Root, 0, this);
	searchGroup_ = new SIMContactListItem(SIMContactListItem::Root, 0, this);
}

SIMContactList::~SIMContactList()
{
	delete root_;
}

SIMContactListItem *SIMContactList::rootItem()
{
	return root_;
}

SIMContactListItem *SIMContactList::invisibleGroup()
{
	return invisibleGroup_;
}

SIMContactListItem *SIMContactList::searchGroup()
{
	return searchGroup_;
}

SIMContactListGroup *SIMContactList::findGroup(const QString &group_name)
{
	SIMContactListGroup *clg = root_->findGroup(group_name);
	if(clg)
		return clg;
	clg = invisibleGroup_->findGroup(group_name);
	if(clg)
		return clg;
	clg = searchGroup_->findGroup(group_name);
	return clg;
}

SIMContactListContact *SIMContactList::findEntry(const QString &j, bool self)
{
	SIMContactListContact *clc = root_->findEntry(j,self);
	if(clc)
		return clc;
	clc = invisibleGroup_->findEntry(j,self);
	if(clc)
		return clc;
	clc = searchGroup_->findEntry(j,self);
	return clc;
}

SIMContactListAccount *SIMContactList::addAccount(PsiAccount *pa)
{
	SIMContactListAccount *account = new SIMContactListAccount(pa, this, root_);
	pa->setContactListAccount(account);
	if(showAccounts())
		root_->appendChild(account);
	updateVisibleParents();
	updateInvisibleParents();

	//emit s_dataChanged();
	return account;
}

const QString &SIMContactList::search()
{
	return search_;
}

void SIMContactList::setSearch(const QString& search)
{
	QString oldsearch = search_;
	search_ = search;

//	if((oldsearch.isEmpty() && !search.isEmpty()) || search.isEmpty())
//		updateParents();

	if(search.isEmpty()) {
		updateVisibleParents();
		updateSearchParents();
	} else if (oldsearch.isEmpty()) {
		updateVisibleParents();
		updateInvisibleParents();
	} else if (search.startsWith(oldsearch))
		updateVisibleParents();
	else if (oldsearch.startsWith(search))
//		updateInvisibleParents();
		updateSearchParents();
	else
		updateParents();
}

bool SIMContactList::isGroupOpen(const QString &s)
{
	printf("group : %s : %s\n", s.ascii(), groupStates[s] ? "open" : "close"); 
	return groupStates[s];
}
void SIMContactList::setGroupOpen(const QString &s, bool b)
{
	groupStates[s] = b;
}

bool SIMContactList::showOffline()
{
	return showOffline_;
}

void SIMContactList::setShowOffline(bool e)
{
	showOffline_ = e;
	updateVisibleParents();
	updateInvisibleParents();
	emit showOffline(e);
}

bool SIMContactList::showAway()
{
	return showAway_;
}

void SIMContactList::setShowAway(bool e)
{
	showAway_ = e;
	updateVisibleParents();
	updateInvisibleParents();
	emit showAway(e);
}

bool SIMContactList::showAgents()
{
	return showAgents_;
}

void SIMContactList::setShowAgents(bool e)
{
	showAgents_ = e;
	updateVisibleParents();
	updateInvisibleParents();
	emit showAgents(e);
}

bool SIMContactList::showSelf()
{
	return showSelf_;
}

void SIMContactList::setShowSelf(bool e)
{
	showSelf_ = e;
	updateVisibleParents();
	updateInvisibleParents();
	emit showSelf(e);
}

bool SIMContactList::showGroups()
{
	return showGroups_;
}

void SIMContactList::setShowGroups(bool e)
{
	showGroups_ = e;
	updateVisibleParents();
	updateInvisibleParents();
	emit showGroups(e);
}

bool SIMContactList::showAccounts()
{
	return showAccounts_;
}



SIMContactListView *SIMContactList::contactListView()
{
	return contactListView_;
}

void SIMContactList::setContactListView(SIMContactListView *clv)
{
	contactListView_ = clv;
}

int SIMContactList::avatarSize()
{
	return contactListView_->iconSize().width() - 4;
}

void SIMContactList::dataChanged()
{
	emit s_dataChanged();
}

void SIMContactList::updateVisibleParents()
{
	root_->updateParents();
	emit s_dataChanged();
}

void SIMContactList::updateInvisibleParents()
{
	invisibleGroup()->updateParents();
	emit s_dataChanged();
}

void SIMContactList::updateSearchParents()
{
	searchGroup()->updateParents();
	emit s_dataChanged();
}

void SIMContactList::updateOptions()
{
	root_->updateOptions();
	searchGroup_->updateOptions();
	invisibleGroup_->updateOptions();
}

void SIMContactList::updateParents()
{
	root_->updateParents();
	searchGroup_->updateParents();
	emit s_dataChanged();
}
