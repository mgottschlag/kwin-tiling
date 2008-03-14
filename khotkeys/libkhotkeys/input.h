/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _INPUT_H_
#define _INPUT_H_

#include <QObject>
#include <QtGui/QWidgetList>
#include <QHash>
#include <QWidget>

#include <kshortcut.h>

#include <X11/X.h>
#include <fixx11h.h>
#include <QList>

#include "shortcuts_handler.h"

namespace KHotKeys
{

class Mouse
    {
    public:
        static bool send_mouse_button( int button_P, bool release_P );
    };

} // namespace KHotKeys

#endif
