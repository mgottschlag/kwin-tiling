/*
  Copyright (c) 2000-2001 Trolltech AS (info@trolltech.com)

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#ifndef LIGHTSTYLE_V2_H
#define LIGHTSTYLE_V2_H


#include <kstyle.h>
#include <Q3PopupMenu>
#include <QtGui/QStyleOption>

#ifdef QT_PLUGIN
#  define Q_EXPORT_STYLE_LIGHT_V2
#else
#  define Q_EXPORT_STYLE_LIGHT_V2 Q_EXPORT
#endif // QT_PLUGIN


class Q_EXPORT_STYLE_LIGHT_V2 LightStyleV2 : public KStyle
{
    Q_OBJECT

public:
    LightStyleV2();
    virtual ~LightStyleV2();

    void polishPopupMenu( Q3PopupMenu * );

    void drawPrimitive(PrimitiveElement, const QStyleOption *, QPainter *, const QWidget * widget = 0 ) const;

    void drawControl(ControlElement, const QStyleOption *, QPainter *, const QWidget * widget = 0 ) const;
    void drawControlMask(ControlElement, QPainter *, const QWidget *, const QRect &,
			 const QStyleOption & = QStyleOption::SO_Default) const;

    QRect subElementRect(SubElement, const QStyleOption * option, const QWidget *) const;

    void drawComplexControl(ComplexControl, QPainter *, const QWidget *,
			    SCFlags = SC_All, SCFlags = SC_None ) const;

    QRect querySubControlMetrics(ComplexControl, const QWidget *, SubControl ) const;

    SubControl querySubControl(ComplexControl, const QWidget *, const QPoint & ) const;

    int pixelMetric(PixelMetric, const QWidget * = 0 ) const;

    QSize sizeFromContents(ContentsType, const QWidget *, const QSize & ) const;

    int styleHint(StyleHint, const QWidget * = 0,
		  QStyleHintReturn * = 0 ) const;

    QPixmap standardPixmap( StandardPixmap standardpixmap,
			 const QWidget* widget = 0 ) const;
};


#endif // LIGHTSTYLE_V2_H
