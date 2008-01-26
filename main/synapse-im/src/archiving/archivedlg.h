#ifndef ARCHIVEDLG_H
#define ARCHIVEDLG_H

#include <QObject>
#include <QDialog>
#include <QDateTime>
#include "ui_archivedlg.h"
#include "xmpp_jid.h"

class PsiAccount;
class GetCollectionListTask;
class GetCollectionTask;

class ArchiveDlg : public QDialog, public Ui::ArchiveDlg
{
	Q_OBJECT
public:
	ArchiveDlg(const XMPP::Jid& j, PsiAccount* pa);
	~ArchiveDlg();

public slots:
	void dateChanged(int year, int month);
	void dateSelected();
	void collectionSelected();

	void prevPage();
	void nextPage();

	void busy();
	void collectionListRetrieved();
	void collectionListError();

	void collectionMsg(int sec, bool direction, const QString &body);
	void collectionRetrieved(int count);

private:
	PsiAccount *pa_;
	XMPP::Jid jid_;

	QDateTime last_;
	int page_;
	int max;

	GetCollectionListTask *gclt_;
	GetCollectionTask *gct_;
};

#endif
