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

/**
 * This widget tracks mouse movements when the left mouse button has been
 * pressed while the cursor was over the widget.
 * The events are sent to a Stroke. When the mouse button is released the Stroke
 * is instructed to process the data; the processed data will then be emitted in
 * a "recorded" signal.
 */

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

        void recorded(const KHotKeys::StrokePoints &data);

    private:
        bool _mouseButtonDown;
        KHotKeys::Stroke stroke;
    };

#endif
