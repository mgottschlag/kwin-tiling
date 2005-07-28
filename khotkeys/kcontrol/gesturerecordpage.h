/****************************************************************************

 KHotKeys
 
 Copyright (C) 2003 Mike Pilone <mpilone@slac.com>
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef GESTURE_RECORD_PAGE_H
#define GESTURE_RECORD_PAGE_H

#include <q3vbox.h>
//Added by qt3to4:
#include <QLabel>

#include "gesturedrawer.h"

class QWidget;
class QPushButton;
class QLabel;

namespace KHotKeys
{

class Gesture;
class GestureRecorder;

class GestureRecordPage : public Q3VBox
    {
    Q_OBJECT

    public:
        GestureRecordPage(const QString &gesture,
                          QWidget *parent, const char *name);
        ~GestureRecordPage();

        const QString &getGesture() const { return _gest; }

    protected slots:
        void slotRecorded(const QString &data);
         void slotResetClicked();

    signals:
        void gestureRecorded(bool);

    private:
        GestureRecorder *_recorder;
        QPushButton *_resetButton;
        GestureDrawer *_tryOne;
        GestureDrawer *_tryTwo;
        GestureDrawer *_tryThree;

        QString _gest;

        Q_UINT32 _tryCount;
    };

} // namespace KHotKeys

#endif
