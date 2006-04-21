/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <qapplication.h>
#include <qpainter.h>
#include <QDesktopWidget>

#include "userrectsel.h"
#include "userrectsel.moc"

UserRectSel::UserRectSel(const RectList& rects, const QPoint& _offset)
  : QWidget(0, Qt::X11BypassWindowManagerHint),
    rectangles(rects),
    offset(_offset)
{
    setGeometry(-10, -10, 2, 2);
}

UserRectSel::~UserRectSel()
{
}

void UserRectSel::mouseReleaseEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        qApp->exit_loop();
    }
}

void UserRectSel::mouseMoveEvent(QMouseEvent * e)
{
    PanelStrut nearest = current;
    int diff = -1;
    QPoint p = e->globalPos(); // + offset;
    for (RectList::const_iterator it = rectangles.constBegin();
         it != rectangles.constEnd();
         ++it)
    {
        PanelStrut r = *it;
        int ndiff = (r.m_rect.center() - p).manhattanLength();

        if (diff < 0 || ndiff < diff)
        {
            diff = ndiff;
            nearest = r;
        }
    }

    if (nearest != current)
    {
        paintCurrent();
        current = nearest;
        paintCurrent();
    }
}

void UserRectSel::paintCurrent()
{
    QPainter p(QApplication::desktop());
    p.setClipping(false);
    p.setPen(QPen(Qt::gray, 3 ));
#warning "KDE4: ROP needs porting"
//    p.setRasterOp(XorROP);
    p.drawRect(current.m_rect);
}

UserRectSel::PanelStrut UserRectSel::select(const RectList& rects, const QPoint& offset)
{
    UserRectSel sel(rects, offset);
    sel.show();
    sel.grabMouse();
    sel.paintCurrent();
    qApp->enter_loop();
    sel.paintCurrent();
    sel.releaseMouse();
    qApp->syncX();
    return sel.current;
}

