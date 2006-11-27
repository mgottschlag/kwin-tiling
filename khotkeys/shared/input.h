/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _INPUT_H_
#define _INPUT_H_

#include <QObject>
#include <qwindowdefs.h>
#include <QHash>
#include <QWidget>

#include <kshortcut.h>

#include <X11/X.h>
#include <fixx11h.h>
#include <QList>

class KActionCollection;
class KAction;

namespace KHotKeys
{

class Kbd_receiver
    {
    public:
		virtual ~Kbd_receiver() {}
        virtual bool handle_key( const KShortcut& shortcut_P ) = 0;
    };

class Kbd : public QObject
{
    Q_OBJECT
public:
	Kbd( bool grabbing_enabled_P, QObject* parent_P );
    virtual ~Kbd();
	void insert_item( const KShortcut& shortcut_P, Kbd_receiver* receiver_P );
    void remove_item( const KShortcut& shortcut_P, Kbd_receiver* receiver_P );
    void activate_receiver( Kbd_receiver* receiver_P );
    void deactivate_receiver( Kbd_receiver* receiver_P );
    static bool send_macro_key( const QString& key, Window window_P = InputFocus );
protected:
    bool x11EventFilter( const XEvent* );                                                              
    void grab_shortcut( const KShortcut& shortcut_P );
    void ungrab_shortcut( const KShortcut& shortcut_P );
private Q_SLOTS:
    void actionTriggered( KAction* action );
private:
    struct Receiver_data
        {
        Receiver_data();
        QList< KShortcut > shortcuts;
        bool active;
        };

    QHash< Kbd_receiver*, Receiver_data > receivers;
    QHash< KShortcut, int > grabs;
    KActionCollection* kga;
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
