#include "opt_antievil.h"

#include "antievil.h"
#include "psioptions.h"
#include "ui_opt_antievil.h"

#include <QWidget>
#include <QWhatsThis>

class OptAntiEvilUI : public QWidget, public Ui::OptAntiEvil
{
public:
	OptAntiEvilUI() : QWidget() { setupUi(this); }
};

//----------------------------------------------------------------------------
// OptionsTabAntiEvil
//----------------------------------------------------------------------------

OptionsTabAntiEvil::OptionsTabAntiEvil(QObject *parent)
: OptionsTab(parent, "antievil", "", tr("AntiEvil"), tr("Anti evil scanner"), "psi/advanced")
{
	w = 0;
}

OptionsTabAntiEvil::~OptionsTabAntiEvil()
{
}

QWidget *OptionsTabAntiEvil::widget()
{
	if ( w )
		return 0;

	w = new OptAntiEvilUI();
	OptAntiEvilUI *d = (OptAntiEvilUI *)w;

	QWhatsThis::add(d->ck_enable,
		tr("Check this option to protect you Psi from evil stanzas."));


	const AntiEvil::Stats *s = AntiEvil::stats();
	d->lb_statsScanned->setNum(s->scanned);
	if (s->blocked) {
		blockedNext(s->blocked, s->lastBlockedFrom, s->lastBlockedTime);
	}

	connect(AntiEvil::stats(), SIGNAL(scannedNext(int)), d->lb_statsScanned, SLOT(setNum(int)));
	connect(AntiEvil::stats(), SIGNAL(blockedNext(int, QString, QDateTime)), this, SLOT(blockedNext(int, QString, QDateTime)));

	return w;
}

void OptionsTabAntiEvil::applyOptions(Options *opt)
{
	if ( !w )
		return;

	OptAntiEvilUI *d = (OptAntiEvilUI *)w;

	PsiOptions::instance()->setOption("options.anti-evil.enable" ,d->ck_enable->isChecked());
}

void OptionsTabAntiEvil::restoreOptions(const Options *opt)
{
	if ( !w )
		return;

	OptAntiEvilUI *d = (OptAntiEvilUI *)w;

	d->ck_enable->setChecked(PsiOptions::instance()->getOption("options.anti-evil.enable").toBool());
}

void OptionsTabAntiEvil::blockedNext(int n, const QString &j, const QDateTime &dt)
{
	OptAntiEvilUI *d = (OptAntiEvilUI *)w;
	d->lb_statsBlocked->setNum(n);
	d->lb_lastFrom->setText(j);
	d->lb_lastTime->setText(dt.toString(Qt::TextDate));
}


