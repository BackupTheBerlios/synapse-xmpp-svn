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

void OptionsTabHistory::applyOptions(Options *opt)
{
	if ( !w )
		return;

	OptHistoryUI *d = (OptHistoryUI *)w;

	opt->historyLogMessages = d->ck_logMessages->isChecked();
	opt->historyLogFileTransfers = d->ck_logFileTransfers->isChecked();

	opt->historyDBBackend = d->cb_backend->currentIndex();
	opt->historyDBHost = d->le_hostname->text();
	opt->historyDBUser = d->le_username->text();
	opt->historyDBPassword = d->le_password->text();
	opt->historyDBName = d->le_dbname->text();
	opt->historyDBPort = d->sb_port->value();
	
}

void OptionsTabHistory::restoreOptions(const Options *opt)
{
	if ( !w )
		return;

	OptHistoryUI *d = (OptHistoryUI *)w;

	d->ck_logMessages->setChecked(opt->historyLogMessages);
	d->ck_logFileTransfers->setChecked(opt->historyLogFileTransfers);

	d->cb_backend->setCurrentIndex(opt->historyDBBackend);
	backendChanged(opt->historyDBBackend);
	d->le_hostname->setText(opt->historyDBHost);
	d->le_username->setText(opt->historyDBUser);
	d->le_password->setText(opt->historyDBPassword);
	d->le_dbname->setText(opt->historyDBName);
	d->sb_port->setValue(opt->historyDBPort);
	
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
