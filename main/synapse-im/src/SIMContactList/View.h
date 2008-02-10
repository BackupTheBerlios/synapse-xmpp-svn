#ifndef VIEW_H
#define VIEW_H

#include <QTreeView>
#include <QComboBox>
#include "xmpp_jid.h"
#include "xmpp_status.h"

class QWidget;
class QWheelEvent;

using namespace XMPP;

namespace SIMContactList {

class Item;

class View : public QTreeView
{
	Q_OBJECT

	enum {
		IconsOnly = 0,
		IconsAndAvatars = 1,
		IconsOnAvatars = 2
	};

public:
	View(QWidget* parent = 0, QComboBox *seach = 0);


	// Reimplemented
	void setModel(QAbstractItemModel* model);

public slots:
	void optionsUpdate();

signals:
	void resizeEventNotifier(QWidget* w);

private slots:
	void qlv_doubleclick(const QModelIndex&);

protected:
	bool event(QEvent *event);
	void mousePressEvent(QMouseEvent *e);

	void dragEnterEvent(QDragEnterEvent *e);
	void dragMoveEvent(QDragMoveEvent *e);
	void dropEvent(QDropEvent *e);

	virtual void resizeEvent(QResizeEvent *e);

	virtual void doItemsLayout();

	void scActionDefault(Item *item);

private:
	int showIcons() const;
	void setShowIcons(int);

	bool qlv_singleclick(QMouseEvent *e);
	void resetExpandedState();
	void resizeColumns();

	int showIcons_;
	QComboBox *search_;
};

};

#endif
