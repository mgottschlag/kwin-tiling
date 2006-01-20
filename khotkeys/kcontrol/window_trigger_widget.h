/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _WINDOW_TRIGGER_WIDGET_H_
#define _WINDOW_TRIGGER_WIDGET_H_

#include <window_trigger_widget_ui.h>

namespace KHotKeys
{

class Window_trigger;
class Action_data;

class Window_trigger_widget
    : public Window_trigger_widget_ui
    {
    Q_OBJECT
    public:
        Window_trigger_widget( QWidget* parent_P = NULL, const char* name_P = NULL );
        void set_data( const Window_trigger* trigger_P );
        Window_trigger* get_data( Action_data* data_P ) const;
    public Q_SLOTS:
        void clear_data();
    };


//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
