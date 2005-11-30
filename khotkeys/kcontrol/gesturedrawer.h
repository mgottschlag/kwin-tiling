/****************************************************************************

 KHotKeys
 
 Copyright (C) 2003 Mike Pilone <mpilone@slac.com>
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef GESTURE_DRAWER_H
#define GESTURE_DRAWER_H

#include <qframe.h>
#include <qstring.h>
#include <qevent.h>
#include <qpoint.h>
#include <qwidget.h>
#include <qsize.h>

namespace KHotKeys
{

class GestureDrawer : public QFrame
    {
    Q_OBJECT
    public:
        GestureDrawer(QWidget *parent, const char *name = 0);
        ~GestureDrawer();

        void setData(const QString &data);

        virtual QSize sizeHint() const { return QSize(30, 30); }

    protected:
        void paintEvent(QPaintEvent *ev);

    private:
        QPoint lookupCellCoords(Q_UINT32 cell);
        void drawArrowHead(QPoint &start, QPoint &end,
                           QPainter &p);


        QString _data;
    };

} // namespace KHotKeys

#endif
