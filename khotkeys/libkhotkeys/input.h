/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef KHOTKEYS_INPUT_H
#define KHOTKEYS_INPUT_H

namespace KHotKeys
{

class Mouse
    {
    public:
        static bool send_mouse_button( int button_P, bool release_P );
    };

} // namespace KHotKeys

#endif
