/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _INPUT_H_
#define _INPUT_H_

#include <qobject.h>
#include <qwindowdefs.h>
#include <qmap.h>
#include <qwidget.h>
#include <q3valuelist.h>
#include <kshortcut.h>

#include <X11/X.h>
#include <fixx11h.h>

class KGlobalAccel;

namespace KHotKeys
{

class Kbd_receiver
    {
    public:
		virtual ~Kbd_receiver() {};
        virtual bool handle_key( const KShortcut& shortcut_P ) = 0;
    };

class Kbd
    : public QObject
    {
    Q_OBJECT
    public:
	Kbd( bool grabbing_enabled_P, QObject* parent_P );
        virtual ~Kbd();
	void insert_item( const KShortcut& shortcut_P, Kbd_receiver* receiver_P );
        void remove_item( const KShortcut& shortcut_P, Kbd_receiver* receiver_P );
        void activate_receiver( Kbd_receiver* receiver_P );
        void deactivate_receiver( Kbd_receiver* receiver_P );
        static bool send_macro_key( unsigned int keycode, Window window_P = InputFocus );
    protected:
        bool x11EventFilter( const XEvent* );                                                              
        void grab_shortcut( const KShortcut& shortcut_P );
        void ungrab_shortcut( const KShortcut& shortcut_P );
    private slots:
        void key_slot( QString key_P );
        void update_connections();
    private:
        struct Receiver_data
            {
            Receiver_data();
            Q3ValueList< KShortcut > shortcuts;
            bool active;
            };
        QMap< Kbd_receiver*, Receiver_data > receivers;
        QMap< KShortcut, int > grabs;
        KGlobalAccel* kga;
    };

class Mouse
    {
    public:
        static bool send_mouse_button( int button_P, bool release_P );
    };


//***************************************************************************
// Inline
//***************************************************************************

// Kbd::Receiver_data

inline
Kbd::Receiver_data::Receiver_data()
    : active( false )
    {
    }

} // namespace KHotKeys

#endif
