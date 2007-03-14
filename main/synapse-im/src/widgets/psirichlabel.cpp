#include "psirichlabel.h"

PsiRichLabel::PsiRichLabel(const QString& text, QWidget *parent)
: QLabel(parent)
{
	margin = 0;
	setFrameStyle(QFrame::NoFrame);
	
	doc = new QTextDocument(this);
	doc->setUndoRedoEnabled(false);
	doc->setDefaultFont(font());
	
	ensurePolished();
	
	text_ = text;
	isRichText = false;
	if (Qt::mightBeRichText(text_)) {
		isRichText = true;
		PsiRichText::install(doc);
		PsiRichText::setText(doc, text_);
	} else {
		doc->setPlainText(text_);
	}
	resize(sizeHint());
};

PsiRichLabel::~PsiRichLabel()
{
}

void PsiRichLabel::setText(const QString& text)
{
	text_ = text;
	if (!isRichText && Qt::mightBeRichText(text_))
		PsiRichText::install(doc);
		
	isRichText = false;
	if (Qt::mightBeRichText(text_)) {
		isRichText = true;
		PsiRichText::setText(doc, text_);
	} else {
		doc->setPlainText(text_);
	}
	resize(sizeHint());
}

QSize PsiRichLabel::sizeHint() const
{
	QRect br;
	
	int hextra = 2 * margin;
	int vextra = hextra;
	
	if (isRichText) {
		hextra = 1;
		vextra = 1;
	}
	
	QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
	fmt.setMargin(0);
	doc->rootFrame()->setFrameFormat(fmt);
	
//	doc->adjustSize();
	br = QRect(QPoint(0, 0), doc->size().toSize());
	
	QFontMetrics fm(font());
	QSize extra(hextra + 1, vextra);
	if (fm.descent() == 2 && fm.ascent() >= 11)
		vextra++;
		
	const QSize contentsSize(br.width() + hextra, br.height() + vextra);
	return contentsSize;
}

QSize PsiRichLabel::sizeForWidth(int w)
{
	QRect br;
	
	int hextra = 2 * margin;
	int vextra = hextra;
	
	if (isRichText) {
		hextra = 1;
		vextra = 1;
	}
	
	PsiRichText::ensureTextLayouted(doc, w);
	const qreal oldTextWidth = doc->textWidth();
	
	doc->adjustSize();
	br = QRect(QPoint(0, 0), doc->size().toSize());
	doc->setTextWidth(oldTextWidth);
	
	QFontMetrics fm(font());
	QSize extra(hextra + 1, vextra);
	if (fm.descent() == 2 && fm.ascent() >= 11)
		vextra++;
		
	const QSize contentsSize(br.width() + hextra, br.height() + vextra);
	return contentsSize;
	
}

void PsiRichLabel::setFont(QFont& font)
{
	font_ = font;
}

void PsiRichLabel::paintEvent(QPaintEvent *e)
{
//	QStylePainter p(this);
//	QStyleOptionFrame opt;
//	opt.init(this);
//	p.end();
	
	QPainter painter(this);
	drawFrame(&painter);
	QRect cr = contentsRect();
	cr.adjust(margin, margin, -margin, -margin);
	
	PsiRichText::ensureTextLayouted(doc, width());
	doc->setDefaultFont(font_);
	QAbstractTextDocumentLayout *layout = doc->documentLayout();
	QRect lr = rect();
	
	QAbstractTextDocumentLayout::PaintContext context;
	
	context.palette = palette();
	
	painter.save();
	painter.translate(lr.x() + 1, lr.y() + 1);
	painter.setClipRect(lr.translated(-lr.x() - 1, -lr.y() - 1));
	layout->draw(&painter, context);
	painter.restore();
}

