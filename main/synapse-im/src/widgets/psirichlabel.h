#ifndef PSIRICHLABEL_H
#define PSIRICHLABEL_H
#include <QLabel>
#include <QEvent>
#include <QSize>
#include <QFont>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>
#include <QTextCursor>
#include <QAbstractTextDocumentLayout>
#include <QTextFrame>
#include "psirichtext.h"

class PsiRichLabel : public QLabel
{
public:
	PsiRichLabel(const QString& text, QWidget *parent);
	~PsiRichLabel();
	
	void setText(const QString& text);
	
	QSize sizeHint() const;
	void setFont(QFont& font);

protected:
	void paintEvent(QPaintEvent *e);
	QSize sizeForWidth(int w);

private:
	QTextDocument *doc;
	QString text_;
	bool isRichText;
	int margin;
	QFont font_;
};

#endif
