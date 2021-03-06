#ifndef PSITRAYICON_H
#define PSITRAYICON_H

#include <QObject>
#include <QRgb>
#include <QSystemTrayIcon>

class PsiIcon;
class QMenu;
class QPoint;
class QPixmap;

class PsiTrayIcon : public QObject
{
	Q_OBJECT
public:
	PsiTrayIcon(const QString &tip, QMenu *popup, QObject *parent=0);
	~PsiTrayIcon();

	void setContextMenu(QMenu*);
	void setToolTip(const QString &);
	void setIcon(const PsiIcon *, bool alert = false);
	void setAlert(const PsiIcon *);
	bool isAnimating() const;

	void showMessage(const QString &title, const QString &msg, QSystemTrayIcon::MessageIcon = QSystemTrayIcon::MessageIcon(1), int msecs = 10000);

signals:
	void clicked(const QPoint &, int);
	void doubleClicked(const QPoint &);
	void closed();

public slots:
	void show();
	void hide();

private slots:
	void animate();
	void trayicon_activated(QSystemTrayIcon::ActivationReason);

protected:
	QPixmap makeIcon();
	QRgb pixelBlend(QRgb p1, QRgb p2);

private:
	PsiIcon* icon_;
	QSystemTrayIcon* trayicon_;
};


#endif
