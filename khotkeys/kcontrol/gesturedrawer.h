/****************************************************************************

 KHotKeys
 
 Copyright (C) 2003 Mike Pilone <mpilone@slac.com>
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef GESTURE_DRAWER_H
#define GESTURE_DRAWER_H

#include <QtGui/QFrame>
#include <QString>
#include <QEvent>
#include <QPoint>
#include <QWidget>
#include <QSize>

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
        QPoint lookupCellCoords(quint32 cell);
        void drawArrowHead(QPoint &start, QPoint &end,
                           QPainter &p);


        QString _data;
    };

} // namespace KHotKeys

#endif
