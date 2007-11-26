#include <QAbstractTextDocumentLayout>

#include "psirichtext.h"
#include "iconset.h"
#include "textutil.h"
#include "common.h"

#include "chattoolbox.h"

RichStatus::RichStatus(QWidget *parent)
: QWidget(parent)
{
	v_rs = 0;
}

RichStatus::~RichStatus()
{
	delete v_rs;
}

void RichStatus::setTxt(QString &txt, int width)
{
	if(txt.isEmpty())
	{
		delete v_rs;
		v_rs = 0;
		return;
	}

	if(v_rs)
		delete v_rs;


	v_rs = new QTextDocument();
	v_rs->setUndoRedoEnabled(false);

	PsiRichText::install(v_rs);
	PsiRichText::setText(v_rs, txt);

	PsiRichText::ensureTextLayouted(v_rs, width);
}

void RichStatus::paintEvent(QPaintEvent *pe)
{
	if(v_rs)
	{
		QPainter p((QWidget*)this);
		QRect rect(pe->rect());
		p.setClipRect(rect);
		p.fillRect( rect, backgroundColor() );
		QAbstractTextDocumentLayout *layout = v_rs->documentLayout();
		QAbstractTextDocumentLayout::PaintContext context;
	
		context.palette = palette();
	
		layout->draw(&p, context);
	}
}

ChatContactBox::ChatContactBox() : QWidget() 
{ 
	setupUi(this);
	lb_keyIcon->setPsiIcon(IconsetFactory::iconPtr("psi/cryptoYes"));
	lb_keyIcon->hide();
	lb_key->hide();
}

void ChatContactBox::updateAvatar(QPixmap ava)
{
	if (ava.isNull()) {
		avatar->hide();
	}
	else {
		avatar->setPixmap(ava.scaled(QSize(100, 100), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		avatar->show();
	}
}

void ChatContactBox::update(const PsiIcon *icon, QString nickname, const QString &statusString, UserListItem *u)
{
	QString txt;
	lb_status->setPsiIcon(icon);
	lb_nickname->setText(nickname);
	QSize size(((lb_nickname->sizeHint().width() + 30) > 108) ? (lb_nickname->sizeHint().width() + 30) : 108, 0);
	resizeToolBox(size);

	if(u) {
		if (!u->mood().isNull()) {
			txt += QString("<icon name=\"psi/smile\"> <b>%1:</b>%2<br/>").arg(tr("Mood")).arg( u->mood().typeText());
			if(!u->mood().text().isEmpty())
				txt += QString("%1<br/>").arg(u->mood().text());
		}
		if (!u->tune().isEmpty())
		{
			txt += QString("<icon name=\"psi/publishTune\"> <b>%1:</b><br/>%2<br/>").arg(tr("Listening to")).arg(u->tune());
		}
		if(!statusString.isEmpty()) {
			txt += QString("<b>Status:</b><br/>");
			QString status = TextUtil::plain2rich(statusString);
			if ( option.useEmoticons )
				status = TextUtil::emoticonify(status);
			txt += status;
		}
	}
	rs_statusString->setTxt(txt, size.width()-8);
	repaint();
}
