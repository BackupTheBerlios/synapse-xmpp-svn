#include <QString>
#include <QWidget>
#include <QTextDocument>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>

#include "userlist.h"

class RichStatus : public QWidget
{
	Q_OBJECT
public:
	RichStatus(QWidget *parent);
	~RichStatus();

	void setTxt(QString &txt, int width);
	void paintEvent(QPaintEvent *pe);
private:
	QTextDocument *v_rs;
};

#include "ui_chatcontact.h"

class ChatContactBox : public QWidget, public Ui::ChatContactBox
{
	Q_OBJECT
public:
	ChatContactBox();

	void updateAvatar(QPixmap ava);
	void update(const PsiIcon *icon, QString nickname, const QString &statusString, UserListItem *u);
	
signals:
	void resizeToolBox(QSize size);

private:
};