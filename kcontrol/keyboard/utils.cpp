/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "utils.h"

#include <QtGui/QImage>
#include <QtGui/QRgb>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>

#include <cmath>

// This code was copied from plasma utils and too common to belong here
// Hopefully such text effects can be placed in some common library some day

template<int aprec, int zprec>
static inline void blurinner(unsigned char *bptr, int &zR, int &zG, int &zB, int &zA, int alpha)
{
  int R, G, B, A;
  R = *bptr;
  G = *(bptr + 1);
  B = *(bptr + 2);
  A = *(bptr + 3);

  zR += (alpha * ((R << zprec) - zR)) >> aprec;
  zG += (alpha * ((G << zprec) - zG)) >> aprec;
  zB += (alpha * ((B << zprec) - zB)) >> aprec;
  zA += (alpha * ((A << zprec) - zA)) >> aprec;

  *bptr =     zR >> zprec;
  *(bptr+1) = zG >> zprec;
  *(bptr+2) = zB >> zprec;
  *(bptr+3) = zA >> zprec;
}

template<int aprec,int zprec>
static inline void blurrow(QImage &im, int line, int alpha)
{
  int zR, zG, zB, zA;

  QRgb *ptr = (QRgb *)im.scanLine(line);
  int width = im.width();

  zR = *((unsigned char *)ptr    ) << zprec;
  zG = *((unsigned char *)ptr + 1) << zprec;
  zB = *((unsigned char *)ptr + 2) << zprec;
  zA = *((unsigned char *)ptr + 3) << zprec;

  for (int index=1; index<width; index++) {
      blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }
  for (int index=width-2; index>=0; index--) {
      blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }
}



template<int aprec, int zprec>
static inline void blurcol(QImage &im, int col, int alpha)
{
  int zR, zG, zB, zA;

  QRgb *ptr = (QRgb *)im.bits();
  ptr += col;
  int height = im.height();
  int width = im.width();

  zR = *((unsigned char *)ptr    ) << zprec;
  zG = *((unsigned char *)ptr + 1) << zprec;
  zB = *((unsigned char *)ptr + 2) << zprec;
  zA = *((unsigned char *)ptr + 3) << zprec;

  for (int index=width; index<(height-1)*width; index+=width) {
      blurinner<aprec,zprec>((unsigned char *)&ptr[index], zR, zG, zB, zA, alpha);
  }

  for (int index=(height-2)*width; index>=0; index-=width) {
      blurinner<aprec,zprec>((unsigned char *)&ptr[index], zR, zG, zB, zA, alpha);
  }
}

template<int aprec, int zprec>
static inline void blurcol(QImage &im, int col, int alpha);

/*
*  expblur(QImage &img, int radius)
*
*  In-place blur of image 'img' with kernel
*  of approximate radius 'radius'.
*
*  Blurs with two sided exponential impulse
*  response.
*
*  aprec = precision of alpha parameter
*  in fixed-point format 0.aprec
*
*  zprec = precision of state parameters
*  zR,zG,zB and zA in fp format 8.zprec
*/
template<int aprec,int zprec>
static void expblur(QImage &img, int radius)
{
  if (radius < 1) {
    return;
  }

  img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);

  /* Calculate the alpha such that 90% of
     the kernel is within the radius.
     (Kernel extends to infinity)
  */
  int alpha = (int)((1 << aprec) * (1.0f - std::exp(-2.3f / (radius + 1.f))));

  int height = img.height();
  int width = img.width();
  for (int row=0; row<height; row++) {
    blurrow<aprec,zprec>(img, row, alpha);
  }

  for (int col=0; col<width; col++) {
    blurcol<aprec,zprec>(img, col, alpha);
  }
  return;
}


static
void shadowBlur(QImage &image, int radius, const QColor &color)
{
    if (radius < 1) {
        return;
    }
    if (image.isNull()) {
        return;
    }

    expblur<16, 7>(image, radius);

    QPainter p(&image);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(image.rect(), color);
    p.end();
}


QPixmap Utils::shadowText(const QString& text, const QFont &font, const QColor& textColor, const QColor& shadowColor, const QPoint& offset, int radius)
{
    //don't try to paint stuff on a future null pixmap because the text is empty
    if (text.isEmpty()) {
        return QPixmap();
    }

    // Draw text
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(text);
    QPixmap textPixmap(textRect.width(), fm.height());
    textPixmap.fill(Qt::transparent);
    QPainter p(&textPixmap);
    p.setPen(textColor);
    p.setFont(font);
    // FIXME: the center alignment here is odd: the rect should be the size needed by
    //        the text, but for some fonts and configurations this is off by a pixel or so
    //        and "centering" the text painting 'fixes' that. Need to research why
    //        this is the case and determine if we should be painting it differently here,
    //        doing soething different with the boundingRect call or if it's a problem
    //        in Qt itself
    p.drawText(textPixmap.rect(), Qt::AlignCenter, text);
    p.end();

    //Draw blurred shadow
    QImage img(textRect.size() + QSize(radius * 2, radius * 2),
    QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    p.begin(&img);
    p.drawImage(QPoint(radius, radius), textPixmap.toImage());
    p.end();
    shadowBlur(img, radius, shadowColor);

    //Compose text and shadow
    int addSizeX;
    int addSizeY;
    if (offset.x() > radius) {
        addSizeX = abs(offset.x()) - radius;
    } else {
        addSizeX = 0;
    }
    if (offset.y() > radius) {
        addSizeY = abs(offset.y()) - radius;
    } else {
        addSizeY = 0;
    }

    QPixmap finalPixmap(img.size() + QSize(addSizeX, addSizeY));
    finalPixmap.fill(Qt::transparent);
    p.begin(&finalPixmap);
    QPointF offsetF(offset);
    QPointF textTopLeft(finalPixmap.rect().topLeft() +
                        QPointF ((finalPixmap.width() - textPixmap.width()) / 2.0, (finalPixmap.height() - textPixmap.height()) / 2.0) -
                        (offsetF / 2.0));
    QPointF shadowTopLeft(finalPixmap.rect().topLeft() +
                          QPointF ((finalPixmap.width() - img.width()) / 2.0, (finalPixmap.height() - img.height()) / 2.0) +
                          (offsetF / 2.0));

    p.drawImage(shadowTopLeft, img);
    p.drawPixmap(textTopLeft, textPixmap);
    p.end();

    return finalPixmap;
}

