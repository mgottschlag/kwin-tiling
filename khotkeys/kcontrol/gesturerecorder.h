/****************************************************************************

 KHotKeys
 
 Copyright (C) 2003 Mike Pilone <mpilone@slac.com>
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef GESTURE_RECORDER_H
#define GESTURE_RECORDER_H

#include <QtGui/QFrame>
#include <QString>

#include <gestures.h>

class QMouseEvent;

namespace KHotKeys
{

class GestureRecorder : public QFrame
    {
    Q_OBJECT

    public:
        GestureRecorder(QWidget *parent, const char *name);
        ~GestureRecorder();

    protected:
        void mousePressEvent(QMouseEvent *);
        void mouseReleaseEvent(QMouseEvent *);
        void mouseMoveEvent(QMouseEvent *);

    Q_SIGNALS:
        void recorded(const QString &data);

    private:
        bool _mouseButtonDown;
            Stroke stroke;
    };

} // namespace KHotKeys

#endif
