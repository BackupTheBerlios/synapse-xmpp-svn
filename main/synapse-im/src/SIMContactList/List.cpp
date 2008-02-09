#include "List.h"
#include "View.h"
#include "Item.h"
#include "Contact.h"
#include "Group.h"
#include "Model.h"
#include "psiaccount.h"
#include "profiles.h"

using namespace SIMContactList;

List::List(QObject* parent)
: QObject(parent), showOffline_(false), showGroups_(true), showAccounts_(false), showAway_(true), showSelf_(true), showAgents_(true)
{
	root_ = new Item(Item::TRoot, 0, this);
	invisibleGroup_ = new Item(Item::TRoot, 0, this);
	searchGroup_ = new Item(Item::TRoot, 0, this);
}

List::~List()
{
	delete root_;
}

Item *List::rootItem()
{
	return root_;
}

Item *List::invisibleGroup()
{
	return invisibleGroup_;
}

Item *List::searchGroup()
{
	return searchGroup_;
}

Item *List::findItem(const QString &name, int _type)
{
	Item *cli = root_->findItem(name, _type);
	if(cli)
		return cli;
	cli = invisibleGroup_->findItem(name, _type);
	if(cli)
		return cli;
	cli = searchGroup_->findItem(name, _type);
	return cli;
}

Contact *List::findEntry(const QString &j, bool self)
{
	Contact *clc = root_->findEntry(j,self);
	if(clc)
		return clc;
	clc = invisibleGroup_->findEntry(j,self);
	if(clc)
		return clc;
	clc = searchGroup_->findEntry(j,self);
	return clc;
}

Account *List::addAccount(PsiAccount *pa)
{
	Account *account = new Account(pa, this, root_);
	pa->setContactListAccount(account);
	if(showAccounts())
		root_->appendChild(account);
	updateVisibleParents();
	updateInvisibleParents();

	//emit s_dataChanged();
	return account;
}

const QString &List::search()
{
	return search_;
}

void List::setSearch(const QString& search)
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
		updateSearchParents();
	else
		updateParents();
}

bool List::isGroupOpen(const QString &s)
{
	UserProfile::instance()->isGroupOpen(s);
}
void List::setGroupOpen(const QString &s, bool b)
{
	UserProfile::instance()->setGroupOpen(s,b);
}

bool List::showOffline()
{
	return showOffline_;
}

void List::setShowOffline(bool e)
{
	showOffline_ = e;
	updateVisibleParents();
	updateInvisibleParents();
	emit showOffline(e);
}

bool List::showAway()
{
	return showAway_;
}

void List::setShowAway(bool e)
{
	showAway_ = e;
	updateVisibleParents();
	updateInvisibleParents();
	emit showAway(e);
}

bool List::showAgents()
{
	return showAgents_;
}

void List::setShowAgents(bool e)
{
	showAgents_ = e;
	updateVisibleParents();
	updateInvisibleParents();
	emit showAgents(e);
}

bool List::showSelf()
{
	return showSelf_;
}

void List::setShowSelf(bool e)
{
	showSelf_ = e;
	updateVisibleParents();
	updateInvisibleParents();
	emit showSelf(e);
}

bool List::showGroups()
{
	return showGroups_;
}

void List::setShowGroups(bool e)
{
	showGroups_ = e;
	updateVisibleParents();
	updateInvisibleParents();
	emit showGroups(e);
}

bool List::showAccounts()
{
	return showAccounts_;
}



View *List::contactListView()
{
	return contactListView_;
}

void List::setContactListView(View *clv)
{
	contactListView_ = clv;
}

int List::avatarSize()
{
	return contactListView_->iconSize().width() - 4;
}

void List::dataChanged()
{
	emit s_dataChanged();
}

void List::contactBlocked(const QString &jid, bool blocked)
{
	Contact *clc = findEntry(jid);
	if(clc)
		clc->setBlocked(blocked);
}

void List::updateVisibleParents()
{
	root_->updateParents();
	emit s_dataChanged();
}

void List::updateInvisibleParents()
{
	invisibleGroup()->updateParents();
	emit s_dataChanged();
}

void List::updateSearchParents()
{
	searchGroup()->updateParents();
	emit s_dataChanged();
}

void List::updateOptions()
{
	root_->updateOptions();
	searchGroup_->updateOptions();
	invisibleGroup_->updateOptions();
}

void List::updateParents()
{
	root_->updateParents();
	searchGroup_->updateParents();
	emit s_dataChanged();
}
