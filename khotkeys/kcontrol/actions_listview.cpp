/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include "actions_listview.h"
#include "actions_listview_widget.h"

namespace KHotKeys
{
Actions_listview::Actions_listview( QWidget* parent_P, const char* name_P )
    : KHListView( parent_P, name_P ), _widget( static_cast< Actions_listview_widget* >( parent_P->parent()))
    {
    // this relies on the way designer creates the .cpp file from .ui (yes, I'm lazy)
    assert( dynamic_cast< Actions_listview_widget_ui* >( parent_P->parent()));
    setDragEnabled( true );
    setDropVisualizer( true );
    setAcceptDrops( true );
    }
}
