/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _DBUS_WIDGET_H_
#define _DBUS_WIDGET_H_

#include <ui_dbus_widget_ui.h>

namespace KHotKeys
{

class Dbus_action;
class Action_data;

class Dbus_widget_ui : public QWidget, public Ui::Dbus_widget_ui
{
public:
   Dbus_widget_ui( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};

class Dbus_widget
    : public Dbus_widget_ui
    {
    Q_OBJECT
    public:
        Dbus_widget( QWidget* parent_P = NULL, const char* name_P = NULL );
        void set_data( const Dbus_action* data_P );
        Dbus_action* get_data( Action_data* data_P ) const;
    public Q_SLOTS:
        void clear_data();
    protected Q_SLOTS:
        virtual void run_dbus_browser_pressed();
        virtual void try_pressed();
    };
    
typedef Dbus_widget Dbus_tab;
        
//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
