#ifndef OPT_HISTORY_H
#define OPT_HISTORY_H

#include "optionstab.h"

class QWidget;

class OptionsTabHistory : public OptionsTab
{
	Q_OBJECT
public:
	OptionsTabHistory(QObject *parent);
	~OptionsTabHistory();

	QWidget *widget();
	void applyOptions();
	void restoreOptions();

public slots:
	void backendChanged(int i);

private:
	QWidget *w;
};

#endif
