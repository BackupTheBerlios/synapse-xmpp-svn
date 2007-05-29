#ifndef OPT_ANTIEVIL_H
#define OPT_ANTIEVIL_H

#include "optionstab.h"
#include <QDateTime>

class QWidget;
struct Options;

class OptionsTabAntiEvil : public OptionsTab
{
	Q_OBJECT
public:
	OptionsTabAntiEvil(QObject *parent);
	~OptionsTabAntiEvil();

	QWidget *widget();
	void applyOptions(Options *opt);
	void restoreOptions(const Options *opt);

private:
	QWidget *w;

private slots:
	void blockedNext(int, const QString&, const QDateTime&);
};

#endif
