/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _DCOP_WIDGET_H_
#define _DCOP_WIDGET_H_

#include <dcop_widget_ui.h>

namespace KHotKeys
{

class Dcop_action;
class Action_data;

class Dcop_widget
    : public Dcop_widget_ui
    {
    Q_OBJECT
    public:
        Dcop_widget( QWidget* parent_P = NULL, const char* name_P = NULL );
        void set_data( const Dcop_action* data_P );
        Dcop_action* get_data( Action_data* data_P ) const;
    public Q_SLOTS:
        void clear_data();
    protected Q_SLOTS:
        virtual void run_kdcop_pressed();
        virtual void try_pressed();
    };
    
typedef Dcop_widget Dcop_tab;
        
//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
