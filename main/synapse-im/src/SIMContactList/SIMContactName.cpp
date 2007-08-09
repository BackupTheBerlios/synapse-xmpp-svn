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

SIMContactName::SIMContactName(const QString &txt, const QColor &color, int width) : color_(color)
{
	v_rt = new QTextDocument();
	v_rt->setUndoRedoEnabled(false);
	if (PsiOptions::instance()->getOption("options.ui.style.rosterEmoticons").toBool())
	{
		PsiRichText::install(v_rt);
	}
	setText(txt, color, width);
}

void SIMContactName::setText(const QString &txt, const QColor &color, int width)
{
	color_ = color;
/*	if (PsiOptions::instance()->getOption("options.ui.style.rosterEmoticons").toBool())
	{
		txt = TextUtil::emoticonify(txt);
		PsiRichText::install(v_rt);
	}*/
	PsiRichText::setText(v_rt, txt);
	v_rt->setDefaultFont(option.font[fStatus]);
	PsiRichText::ensureTextLayouted(v_rt, width - 2);
}

void SIMContactName::paint(QPainter *painter, const QRect &rect, const QPalette &palette, EditMode mode) const
{
	painter->save();

	QSize size = v_rt->size().toSize();
	int h = (rect.height() - size.height())/2;
	painter->translate(rect.x() + 2, rect.y() + h);

	QAbstractTextDocumentLayout *layout = v_rt->documentLayout();
	QAbstractTextDocumentLayout::PaintContext context;
	
	context.palette = palette;
	context.palette.setColor(QPalette::Text, color_);

	layout->draw(painter, context);

	//delete v_rt;

	painter->restore();
}

QSize SIMContactName::sizeHint(const QRect &rect) const
{
	return v_rt->size().toSize();
}