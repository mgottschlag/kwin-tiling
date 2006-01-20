/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _INFO_TAB_H_
#define _INFO_TAB_H_

#include <info_tab_ui.h>

namespace KHotKeys
{

class Info_tab
    : public Info_tab_ui
    {
    Q_OBJECT
    public:
        Info_tab( QWidget* parent_P = NULL, const char* name_P = NULL );
    public Q_SLOTS:
        void clear_data();
    };

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
