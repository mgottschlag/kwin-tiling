/****************************************************************************

 KHotKeys

 Copyright (C) 2003 Mike Pilone <mpilone@slac.com>
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef GESTURE_DRAWER_H
#define GESTURE_DRAWER_H

#include "triggers/gestures.h"

#include <QtGui/QFrame>

#include <QEvent>
#include <QPoint>
#include <QWidget>
#include <QSize>


/**
 * This widget provides the service of drawing the processed point data of the
 * gesture.
 */

class GestureDrawer : public QFrame
    {
    Q_OBJECT

    public:

        GestureDrawer(QWidget *parent, const char *name = 0);
        ~GestureDrawer();

        void setPointData(const KHotKeys::StrokePoints &data);
        KHotKeys::StrokePoints pointData() const;


        virtual QSize sizeHint() const { return QSize(30, 30); }

    protected:
        void paintEvent(QPaintEvent *ev);

    private:
        KHotKeys::StrokePoints _data;
    };

#endif
