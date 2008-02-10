#include "View.h"
#include "Model.h"
#include "Name.h"
#include "psioptions.h"
#include "psirichtext.h"
#include "common.h"
#include "textutil.h"
#include <QAbstractTextDocumentLayout>

#include <QPainter>

using namespace SIMContactList;

Name::Name() : clv_(0)
{
	v_rt = new QTextDocument();
	v_rt->setUndoRedoEnabled(false);
	if (PsiOptions::instance()->getOption("options.ui.style.rosterEmoticons").toBool())
	{
		PsiRichText::install(v_rt);
	}
}

Name::Name(const QString &txt, const QColor &color, View *clv) : color_(color), clv_(clv)
{
	v_rt = new QTextDocument();
	v_rt->setUndoRedoEnabled(false);
	if (PsiOptions::instance()->getOption("options.ui.style.rosterEmoticons").toBool())
	{
		PsiRichText::install(v_rt);
	}
	setText(txt, color_, clv);
}

void Name::setText(const QString &txt, QColor color, View *clv)
{
	color_ = color;
	clv_ = clv;

	if (PsiOptions::instance()->getOption("options.ui.style.rosterEmoticons").toBool())
	{
		PsiRichText::install(v_rt);
	}
	PsiRichText::setText(v_rt, txt);
	QFont fnt;
	fnt.fromString(PsiOptions::instance()->getOption("options.ui.look.font.contactlist").toString());
	v_rt->setDefaultFont(fnt);
	PsiRichText::ensureTextLayouted(v_rt, clv_->columnWidth(Model::NameColumn)-2);
}

void Name::paint(QPainter *painter, const QRect &rect, const QPalette &palette, EditMode mode) const
{
//	PsiRichText::ensureTextLayouted(v_rt, rect.width()-2);
	painter->save();

	QSize size = v_rt->size().toSize();
	int h = (rect.height() - size.height())/2;
	QBrush *paper = new QBrush(PsiOptions::instance()->getOption("options.ui.look.colors.contactlist.background").value<QColor>(), Qt::SolidPattern);
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

QSize Name::sizeHint(const QRect &rect) const
{
	PsiRichText::ensureTextLayouted(v_rt, clv_->columnWidth(Model::NameColumn)-2);
	return v_rt->size().toSize();
}