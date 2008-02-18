/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _GENERAL_TAB_H_
#define _GENERAL_TAB_H_

#include <ui_general_tab_ui.h>

namespace KHotKeys
{

class Action_data;

class General_tab_ui : public QWidget, public Ui::General_tab_ui
{
public:
  General_tab_ui( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};

class General_tab
    : public General_tab_ui
    {
    Q_OBJECT
    public:
        General_tab( QWidget* parent_P = NULL, const char* name_P = NULL );
        void set_data( const Action_data* data_P );
        void get_data( QString& name_O, QString& comment_O, bool& enabled_O );
    public Q_SLOTS:
        void clear_data();
    Q_SIGNALS:
        void action_type_changed( int type_P );
    protected Q_SLOTS:
        void action_name_changed( const QString& name_P );
    };
    
//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
