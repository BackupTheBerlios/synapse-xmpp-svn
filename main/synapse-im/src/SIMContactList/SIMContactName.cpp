#include "SIMContactName.h"
#include "psioptions.h"
#include "psirichtext.h"
#include "common.h"
#include "textutil.h"
#include <QAbstractTextDocumentLayout>

#include <QPainter>

SIMContactName::SIMContactName()
{
	v_rt = new QTextDocument();
	v_rt->setUndoRedoEnabled(false);
	if (PsiOptions::instance()->getOption("options.ui.style.rosterEmoticons").toBool())
	{
		PsiRichText::install(v_rt);
	}
}

SIMContactName::SIMContactName(const QString &txt, const QColor &color) : color_(color)
{
	v_rt = new QTextDocument();
	v_rt->setUndoRedoEnabled(false);
	if (PsiOptions::instance()->getOption("options.ui.style.rosterEmoticons").toBool())
	{
		PsiRichText::install(v_rt);
	}
	setText(txt, color);
}

void SIMContactName::setText(const QString &txt, const QColor &color)
{
	color_ = color;
	if (PsiOptions::instance()->getOption("options.ui.style.rosterEmoticons").toBool())
	{
		PsiRichText::install(v_rt);
	}
	PsiRichText::setText(v_rt, txt);
	QFont fnt;
	fnt.fromString(option.font[fRoster]);
	v_rt->setDefaultFont(fnt);
}

void SIMContactName::paint(QPainter *painter, const QRect &rect, const QPalette &palette, EditMode mode) const
{
	PsiRichText::ensureTextLayouted(v_rt, rect.width()-2);
	painter->save();

	QSize size = v_rt->size().toSize();
	int h = (rect.height() - size.height())/2;
	QBrush *paper = new QBrush(option.color[cListBack], Qt::SolidPattern);
	painter->fillRect( rect.x(), rect.y(), rect.width(), rect.height(), *paper );

	painter->translate(rect.x() + 2, rect.y() + h);

	QAbstractTextDocumentLayout *layout = v_rt->documentLayout();
	QAbstractTextDocumentLayout::PaintContext context;
	
	context.palette = palette;
	context.palette.setColor(QPalette::Text, color_);
	
	layout->draw(painter, context);

	delete paper;

	painter->restore();
}

QSize SIMContactName::sizeHint(const QRect &rect) const
{
	return v_rt->size().toSize();
}