/* 
   Copyright (c) 2001 Nikolas Zimmermann <wildfox@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qrect.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <kdebug.h>
#include <kalphapainter.h>
#include "kalphalabel.moc"

KAlphaLabel::KAlphaLabel(QWidget *parent, const char *name, WFlags f)
	: QLabel(parent, name, f),
	  m_image(0)
{
}

KAlphaLabel::KAlphaLabel(const QString &text, QWidget *parent, const char *name, WFlags f)
	: QLabel(text, parent, name, f),
	  m_image(0)
{
}
	

KAlphaLabel::KAlphaLabel(QWidget *buddy, const QString &text, QWidget *parent, const char *name, WFlags f)
	: QLabel(buddy, text, parent, name, f),
	  m_image(0)
{
}

KAlphaLabel::~KAlphaLabel()
{
}

void KAlphaLabel::setPixmap(const QPixmap &p)
{
	QLabel::setPixmap(p);

	m_image = new QImage(pixmap()->convertToImage());
}

void KAlphaLabel::drawContents(QPainter *p)
{
	QRect cr = contentsRect();

	int m = indent();
	if(m < 0)
	{
		// This is ugly.
		if(frameWidth() > 0)
			m = p->fontMetrics().width('x') / 2;
		else
			m = 0;
	}
	
	if(m > 0)
	{
		if(alignment() & AlignLeft)
			cr.setLeft(cr.left() + m);
		if(alignment() & AlignRight)
			cr.setRight(cr.right() - m);
		if(alignment() & AlignTop)
			cr.setTop(cr.top() + m);
		if(alignment() & AlignBottom )
			cr.setBottom(cr.bottom() - m);
	}

    QPixmap *pix = pixmap();

	if(pix->size() != cr.size())
		pix->convertFromImage(m_image->smoothScale(cr.width(), cr.height()));
	
	KAlphaPainter::draw(p, *pix, QPixmap(), cr.x(), cr.y());
}

