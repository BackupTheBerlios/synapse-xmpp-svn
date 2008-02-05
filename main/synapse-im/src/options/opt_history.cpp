#include "opt_history.h"
#include "common.h"
#include "iconwidget.h"

#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include "ui_opt_history.h"
#include "psioptions.h"
#include "spellchecker.h"

class OptHistoryUI : public QWidget, public Ui::OptHistory
{
public:
	OptHistoryUI() : QWidget() { setupUi(this); }
};

//----------------------------------------------------------------------------
// OptionsTabHistory
//----------------------------------------------------------------------------

OptionsTabHistory::OptionsTabHistory(QObject *parent)
: OptionsTab(parent, "history", "", tr("History"), tr("Options for logging chats"), "psi/history")
{
	w = 0;
}

OptionsTabHistory::~OptionsTabHistory()
{
}

QWidget *OptionsTabHistory::widget()
{
	if ( w )
		return 0;

	w = new OptHistoryUI();
	OptHistoryUI *d = (OptHistoryUI *)w;

	d->sb_port->setRange(0,32767);

	connect(d->cb_backend, SIGNAL(currentIndexChanged(int)), this, SLOT(backendChanged(int)));

	return w;
}

void OptionsTabHistory::applyOptions()
{
	if ( !w )
		return;

	OptHistoryUI *d = (OptHistoryUI *)w;

	PsiOptions::instance()->setOption("options.history.messages", d->ck_logMessages->isChecked());
	PsiOptions::instance()->setOption("options.history.file-transfers", d->ck_logFileTransfers->isChecked());

	PsiOptions::instance()->setOption("options.history.backend", d->cb_backend->currentText());
	PsiOptions::instance()->setOption("options.history.host", d->le_hostname->text());
	PsiOptions::instance()->setOption("options.history.user", d->le_username->text());
	PsiOptions::instance()->setOption("options.history.password", d->le_password->text());
	PsiOptions::instance()->setOption("options.history.database", d->le_dbname->text());
	PsiOptions::instance()->setOption("options.history.port", d->sb_port->value());
	
}

void OptionsTabHistory::restoreOptions()
{
	if ( !w )
		return;

	OptHistoryUI *d = (OptHistoryUI *)w;

	d->ck_logMessages->setChecked(	PsiOptions::instance()->getOption("options.history.messages").toBool());
	d->ck_logFileTransfers->setChecked(PsiOptions::instance()->getOption("options.history.file-transfers").toBool());

	QString backend = PsiOptions::instance()->getOption("options.history.backend").toString();
	int i = 0;
	for(i=0; i<d->cb_backend->count(); i++)
		if(d->cb_backend->itemText(i).compare(backend) == 0)
			break;
	d->cb_backend->setCurrentIndex(i);

	d->le_hostname->setText(PsiOptions::instance()->getOption("options.history.host").toString());
	d->le_username->setText(PsiOptions::instance()->getOption("options.history.user").toString());
	d->le_password->setText(PsiOptions::instance()->getOption("options.history.password").toString());
	d->le_dbname->setText(PsiOptions::instance()->getOption("options.history.database").toString());
	d->sb_port->setValue(PsiOptions::instance()->getOption("options.history.port").toInt());
	
}

void OptionsTabHistory::backendChanged(int i)
{
	if ( !w )
		return;

	OptHistoryUI *d = (OptHistoryUI *)w;

	bool enabled = (i>0);

	d->gb_dbcon->setEnabled(enabled);
	d->le_hostname->setEnabled(enabled);
	d->le_username->setEnabled(enabled);
	d->le_password->setEnabled(enabled);
	d->le_dbname->setEnabled(enabled);
	d->sb_port->setEnabled(enabled);
}
