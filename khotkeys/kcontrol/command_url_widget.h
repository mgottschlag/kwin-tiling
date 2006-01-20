/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _COMMAND_URL_WIDGET_H_
#define _COMMAND_URL_WIDGET_H_

#include <command_url_widget_ui.h>

namespace KHotKeys
{

class Command_url_action;
class Action_data;

class Command_url_widget
    : public Command_url_widget_ui
    {
    Q_OBJECT
    public:
        Command_url_widget( QWidget* parent_P = NULL, const char* name_P = NULL );
        void set_data( const Command_url_action* data_P );
        Command_url_action* get_data( Action_data* data_P ) const;
    public Q_SLOTS:
        void clear_data();
    protected Q_SLOTS:
        virtual void browse_pressed();
    };

typedef Command_url_widget Command_url_tab;

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
