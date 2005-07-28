/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _ACTIONS_LISTVIEW_H_
#define _ACTIONS_LISTVIEW_H_

#include <khlistview.h>

#include <action_data.h>

namespace KHotKeys
{
class Actions_listview_widget;

class Actions_listview
    : public KHListView    
    {
    Q_OBJECT
    public:
        Actions_listview( QWidget* parent_P = NULL, const char* name_P = NULL );
        Actions_listview_widget* widget();
    private:
        Actions_listview_widget* _widget;
    };

}

#endif
