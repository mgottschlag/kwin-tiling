/* -------------------------------------------------------------

   clipboardpoll.cpp (part of Klipper - Cut & paste history for KDE)

   (C) 2003 by Lubos Lunak <l.lunak@kde.org>

   Licensed under the GNU GPL Version 2

 ------------------------------------------------------------- */

#ifndef _CLIPBOARDPOLL_H_
#define _CLIPBOARDPOLL_H_

#include <qwidget.h>
#include <qtimer.h>
#include <X11/Xlib.h>
#include <fixx11h.h>

class ClipboardPoll
    : public QWidget
    {
    Q_OBJECT
    public:
        ClipboardPoll( QWidget* parent );
    signals:
        void clipboardChanged( bool selectionMode );
    protected:
        virtual bool x11Event( XEvent* );
    private slots:
        void timeout();
    private:
        struct SelectionData
        {
            Atom atom;
            Atom sentinel_atom;
            Atom timestamp_atom;
            Window last_owner;
            bool owner_is_qt;
            Time last_change;
            bool waiting_for_timestamp;
            Time waiting_x_time;
        };
        void updateQtOwnership( SelectionData& data );
        bool checkTimestamp( SelectionData& data );
        bool changedTimestamp( SelectionData& data, const XEvent& e );
        QTimer timer;
        SelectionData selection;
        SelectionData clipboard;
        Atom xa_clipboard;
        Atom xa_timestamp;
    };
    
#endif
