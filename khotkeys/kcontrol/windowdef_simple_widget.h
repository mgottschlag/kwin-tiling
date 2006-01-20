/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _WINDOWDEF_SIMPLE_WIDGET_H_
#define _WINDOWDEF_SIMPLE_WIDGET_H_

#include <windowdef_simple_widget_ui.h>

namespace KHotKeys
{

class Windowdef_simple;
class Windowdef_list_item;

class Windowdef_simple_widget
    : public Windowdef_simple_widget_ui
    {
    Q_OBJECT
    public:
        Windowdef_simple_widget( QWidget* parent_P = NULL, const char* name_P = NULL );
        void set_data( const Windowdef_simple* data_P );
        Windowdef_simple* get_data() const;
        void set_autodetect( QObject* obj_P, const char* slot_P );
    public Q_SLOTS:
        void clear_data();
    protected:
        void autodetect();
    protected Q_SLOTS:
        virtual void autodetect_clicked();
        virtual void window_role_combo_changed( int item_P );
        virtual void window_class_combo_changed( int item_P );
        virtual void window_title_combo_changed( int item_P );
        void autodetect_window_selected( WId window );
    Q_SIGNALS:
        void autodetect_signal();
    };

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
