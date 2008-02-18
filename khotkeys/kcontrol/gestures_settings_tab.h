/****************************************************************************

 KHotKeys
 
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _GESTURES_SETTINGS_TAB_H_
#define _GESTURES_SETTINGS_TAB_H_

#include <ui_gestures_settings_tab_ui.h>


class Gestures_settings_tab_ui : public QWidget, public Ui::Gestures_settings_tab_ui
{
public:
  Gestures_settings_tab_ui( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};
namespace KHotKeys
{
class Gestures_settings_tab
    : public Gestures_settings_tab_ui
    {
    Q_OBJECT
    public:
        Gestures_settings_tab( QWidget* parent = NULL, const char* name = NULL );
        void read_data();
        void write_data() const;
    public Q_SLOTS:
        void clear_data();
    };

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
