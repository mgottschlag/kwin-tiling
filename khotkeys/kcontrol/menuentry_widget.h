/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _MENUENTRY_WIDGET_H_
#define _MENUENTRY_WIDGET_H_

#include <menuentry_widget_ui.h>

namespace KHotKeys
{

class Menuentry_action;
class Action_data;

class Menuentry_widget
    : public Menuentry_widget_ui
    {
    Q_OBJECT
    public:
        Menuentry_widget( QWidget* parent_P = NULL, const char* name_P = NULL );
        void set_data( const Menuentry_action* data_P );
        Menuentry_action* get_data( Action_data* data_P ) const;
    public Q_SLOTS:
        void clear_data();
    protected Q_SLOTS:
        virtual void browse_pressed();
    };

typedef Menuentry_widget Menuentry_tab;

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
