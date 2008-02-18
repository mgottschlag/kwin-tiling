/****************************************************************************

 KHotKeys
 
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _GENERAL_SETTINGS_TAB_H_
#define _GENERAL_SETTINGS_TAB_H_

#include <ui_general_settings_tab_ui.h>

class General_settings_tab_ui : public QWidget, public Ui::General_settings_tab_ui
{
public:
   General_settings_tab_ui( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};

namespace KHotKeys
{

class General_settings_tab
    : public General_settings_tab_ui
    {
    Q_OBJECT
    public:
        General_settings_tab( QWidget* parent = NULL, const char* name = NULL );
        void read_data();
        void write_data() const;
    public Q_SLOTS:
        void clear_data();
    protected Q_SLOTS:
        virtual void import_clicked();
    };

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
