/****************************************************************************

 KHotKeys

 Copyright (C) 2003 Mike Pilone <mpilone@slac.com>
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef GESTURE_RECORDER_H
#define GESTURE_RECORDER_H

#include "triggers/gestures.h"

#include <QtGui/QFrame>

class QMouseEvent;

class GestureRecorder : public QFrame
    {
    Q_OBJECT

    public:

        GestureRecorder(QWidget *parent, const char *name="FIXXXXXMMEEEEEEEEEEEEE");
        ~GestureRecorder();

    protected:

        void mousePressEvent(QMouseEvent *);
        void mouseReleaseEvent(QMouseEvent *);
        void mouseMoveEvent(QMouseEvent *);

    Q_SIGNALS:

        void recorded(const QString &data);

    private:
        bool _mouseButtonDown;
        KHotKeys::Stroke stroke;
    };

#endif
