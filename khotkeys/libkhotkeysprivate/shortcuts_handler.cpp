/*
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config-X11.h>

#include "input.h"
#include "shortcuts_handler.h"
#include "windows_handler.h"
#include "khotkeysglobal.h"


#include <X11/X.h>
// #include <X11/Xutil.h>
#include <QX11Info>
#include <fixx11h.h>

#include <KDE/KAction>
#include <KDE/KDebug>

#include <QtCore/QUuid>
#include <kkeyserver.h>

namespace KHotKeys {


ShortcutsHandler::ShortcutsHandler( HandlerType type, QObject *parent )
        : QObject(parent)
         ,_type(type)
         ,_actions(new KActionCollection(this))
    {
    _actions->setComponentData(KComponentData("khotkeys"));
    }


ShortcutsHandler::~ShortcutsHandler()
    {
    _actions->clear();
    delete _actions;
    }


KAction *ShortcutsHandler::addAction(
        const QString &id,
        const QString &text,
        const KShortcut &shortcut )
    {
#ifdef KHOTKEYS_TRACE
    kDebug() << id << text << shortcut;
#endif
    QString realId(id);
    // HACK: Do this correctly. Remove uuid on importing / exporting
    // On import it can happen that id is already taken. Create it under a
    // different name then.
    if (_actions->action(id))
        {
        qDebug() << id << " already present. Using new id!";
        realId = QUuid::createUuid();
        }

    // Create the action
    KAction *newAction = _actions->addAction(realId);
    if (!newAction)
        {
        return 0;
        }
    // If our HandlerType is configuration we have to tell kdedglobalaccel
    // that this action is only for configuration purposes.
    // see KAction::~KAction
    if (_type==Configuration)
        {
        newAction->setProperty("isConfigurationAction", QVariant(true));
        }
    newAction->setText(text);
    newAction->setGlobalShortcut( shortcut, KAction::ActiveShortcut );
    // Enable global shortcut. If that fails there is no sense in proceeding
    if (!newAction->isGlobalShortcutEnabled())
        {
        kWarning() << "Failed to enable global shortcut for '" 
                   << text << "' " << id;
        _actions->removeAction(newAction);
        return 0;
        }
    Q_ASSERT(newAction->isEnabled());

    connect(
        newAction, SIGNAL(globalShortcutChanged(QKeySequence)),
        this, SIGNAL(shortcutChanged()));

    return newAction;
    }


QAction *ShortcutsHandler::getAction( const QString &id )
    {
    return _actions->action(id);
    }


bool ShortcutsHandler::removeAction( const QString &id )
    {
    QAction *action = getAction( id );
    if (!action)
        {
        return false;
        }
    else
        {
        // This will delete the action.
        _actions->removeAction(action);

        return true;
        }
    }


#ifdef HAVE_XTEST

} // namespace KHotKeys
#include <X11/extensions/XTest.h>
namespace KHotKeys
{

static bool xtest_available = false;
static bool xtest_inited = false;
static bool xtest()
    {
    if( xtest_inited )
        return xtest_available;
    xtest_inited = true;
    int dummy1, dummy2, dummy3, dummy4;
    xtest_available =
        ( XTestQueryExtension( QX11Info::display(), &dummy1, &dummy2, &dummy3, &dummy4 ) == True );
    return xtest_available;
    }
#endif

bool ShortcutsHandler::send_macro_key( const QKeySequence &key, Window window_P )
    {
    kDebug() << key << key.count() << window_P;

    if (key.isEmpty())
        return false;

    unsigned int keysym = key[0];
    int x_keycode;
    KKeyServer::keyQtToCodeX(keysym, &x_keycode);
    if( x_keycode == NoSymbol )
        return false;

    unsigned int x_mod;
    KKeyServer::keyQtToModX(keysym, &x_mod );
#ifdef HAVE_XTEST
    if( xtest() && window_P == None )
        {
        // CHECKME tohle jeste potrebuje modifikatory
        bool ret = XTestFakeKeyEvent( QX11Info::display(), x_keycode, True, CurrentTime );
        ret = ret && XTestFakeKeyEvent( QX11Info::display(), x_keycode, False, CurrentTime );
        return ret;
        }
#endif
    if( window_P == None || window_P == InputFocus )
        window_P = windows_handler->active_window();
    if( window_P == None ) // CHECKME tohle cele je ponekud ...
        window_P = InputFocus;
    XEvent ev;
    ev.type = KeyPress;
    ev.xkey.display = QX11Info::display();
    ev.xkey.window = window_P;
    ev.xkey.root = QX11Info::appRootWindow();   // I don't know whether these have to be set
    ev.xkey.subwindow = None;       // to these values, but it seems to work, hmm
    ev.xkey.time = CurrentTime;
    ev.xkey.x = 0;
    ev.xkey.y = 0;
    ev.xkey.x_root = 0;
    ev.xkey.y_root = 0;
    ev.xkey.keycode = x_keycode;
    ev.xkey.state = x_mod;
    ev.xkey.same_screen = True;
    bool ret = XSendEvent( QX11Info::display(), window_P, True, KeyPressMask, &ev );
#if 1
    ev.type = KeyRelease;  // is this actually really needed ??
    ev.xkey.display = QX11Info::display();
    ev.xkey.window = window_P;
    ev.xkey.root = QX11Info::appRootWindow();
    ev.xkey.subwindow = None;
    ev.xkey.time = CurrentTime;
    ev.xkey.x = 0;
    ev.xkey.y = 0;
    ev.xkey.x_root = 0;
    ev.xkey.y_root = 0;
    ev.xkey.state = x_mod;
    ev.xkey.keycode = x_keycode;
    ev.xkey.same_screen = True;
    ret = ret && XSendEvent( QX11Info::display(), window_P, True, KeyReleaseMask, &ev );
#endif
    // Qt's autorepeat compression is broken and can create "aab" from "aba"
    // XSync() should create delay longer than Qt's max autorepeat interval
    XSync( QX11Info::display(), False );
    return ret;
    }


bool Mouse::send_mouse_button( int button_P, bool release_P )
    {
#ifdef HAVE_XTEST
    if( xtest())
        {
        // CHECKME tohle jeste potrebuje modifikatory
        // a asi i spravnou timestamp misto CurrentTime
        bool ret = XTestFakeButtonEvent( QX11Info::display(), button_P, True, CurrentTime );
        if( release_P )
            ret = ret && XTestFakeButtonEvent( QX11Info::display(), button_P, False, CurrentTime );
        return ret;
        }
#endif
    return false;
    }

} // namespace KHotKeys

#include "moc_shortcuts_handler.cpp"
