/*****************************************************************

Copyright (c) 2000-2001 Matthias Elter <elter@kde.org>
Copyright (c) 2001 Richard Moore <rich@kde.org>

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

#ifndef TASKP_H
#define TASKP_H

#include "task.h"

#include <QTime>
#include <NETWinInfo>

namespace TaskManager
{

static const unsigned long windowInfoFlags = NET::WMState | NET::XAWMState | NET::WMDesktop |
        NET::WMVisibleName | NET::WMGeometry |
        NET::WMWindowType | NET::WM2WindowClass | NET::WM2AllowedActions;
static const unsigned long windowInfoFlags2 = NET::WM2WindowClass | NET::WM2AllowedActions;

class Task::Private
{
public:
    Private(WId w)
        : win(w),
          frameId(w),
          info(KWindowSystem::windowInfo(w, windowInfoFlags, windowInfoFlags2)),
          lastWidth(0),
          lastHeight(0),
          cachedChanges(0, 0),
          cachedChangesTimerId(0),
          active(false),
          lastResize(false),
          demandedAttention(false) {
    }

    WId win;
    WId frameId;
    KWindowInfo info;
    WindowList transients;
    WindowList transientsDemandingAttention;
    QStringList activities;

    int lastWidth;
    int lastHeight;
    QIcon icon;

    QRect iconGeometry;

    QTime lastUpdate;
    Task::WindowProperties cachedChanges;
    int cachedChangesTimerId;
    QPixmap pixmap;
    QPixmap lastIcon;
    bool active : 1;
    bool lastResize : 1;
    bool demandedAttention : 1;
};
}

#endif
