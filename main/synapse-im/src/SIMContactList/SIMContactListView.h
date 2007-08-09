#ifndef SIMCONTACTLISTVIEW_H
#define SIMCONTACTLISTVIEW_H

#include <QTreeView>
#include "xmpp_jid.h"
#include "xmpp_status.h"

class QWidget;
class SIMContactListItem;
class QWheelEvent;

using namespace XMPP;

class SIMContactListView : public QTreeView
{
	Q_OBJECT

	enum {
		IconsOnly = 0,
		IconsAndAvatars = 1,
		IconsOnAvatars = 2
	};

public:
	SIMContactListView(QWidget* parent = 0);


	// Reimplemented
	void setModel(QAbstractItemModel* model);

	void updateOptions();

private slots:
	void qlv_doubleclick(const QModelIndex&);

protected:
	void wheelEvent(QWheelEvent *);
	bool event(QEvent *event);
	void mousePressEvent(QMouseEvent *e);

	void dragEnterEvent(QDragEnterEvent *e);
	void dragMoveEvent(QDragMoveEvent *e);
	void dropEvent(QDropEvent *e);

	virtual void doItemsLayout();

	void scActionDefault(SIMContactListItem *item);

private:
	int showIcons() const;
	void setShowIcons(int);

	bool qlv_singleclick(QMouseEvent *e);
	void resetExpandedState();
	void resizeColumns();

	int showIcons_;
};

#endif
