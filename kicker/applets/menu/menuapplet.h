/*****************************************************************

Copyright (c) 2002 Siegfried Nijssen <snijssen@liacs.nl>
Copyright (c) 2003 Lubos Lunak <l.lunak@suse.cz>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef _MENUAPPLET_H_
#define _MENUAPPLET_H_

#include <assert.h>

#include <QX11EmbedWidget>

#include <kpanelapplet.h>
#include <kmanagerselection.h>


#include <karrowbutton.h>

#include "utils.h"

class KWinModule;

namespace KickerMenuApplet
{

class MenuEmbed;

/**
 * @short A central menu bar
 *
 * @description All status change, such as when an window is activated,
 * a new window popped up, etc, is received via @ref KWin::WindowInfo and @ref
 * NETWinInfo. Status changes for X selections are done via KSelectionWatcher.
 *
 * How it works in broad terms:  KickerMenuApplet gets notified as soon a window
 * changes(a new pops up etc.) and accordingly updates the list @ref menus,
 * which contains all known menus. When a new window gains focus, it looks up the
 * correct MenuEmbed in @ref menus, and then switches to that one.
 *
 * The documentation is a bit rusty -- read it with a critical eye.
 *
 * @author Siegfried Nijssen <snijssen@liacs.nl>
 * @author Lubos Lunak <l.lunak@suse.cz>
 */
class Applet : public KPanelApplet
    {
    Q_OBJECT
#ifdef __GNUC__
#warning "kde4 port it to dbus interface"
#endif	    
	//K_DCOP
    public slots:
    	/**
	 * Called by the Kicker configuration(KCM). Does in turn call
	 * readSettings().
	 */
	/*Q_NOREPLY*/ void configure();
    public:
	Applet( const QString& configFile, QWidget *parent );
        virtual ~Applet();
	virtual int widthForHeight( int height ) const;
	virtual int heightForWidth( int width ) const;
	/**
	 * Looks up @param embed in @ref menus, and removes it.
	 */
	void menuLost( MenuEmbed* embed );
        void updateMenuGeometry( MenuEmbed* embed );
    protected:
	virtual void paletteChange(const QPalette& );
        virtual void positionChange( Plasma::Position p );
    private Q_SLOTS:

	/**
	 * Called each time a window is added. When the selection is
	 * initially claimed, it is called for every window. It does the big
	 * work, and does the embedding with MenuEmbed.
	 */

	void windowAdded( WId w );
    	/**
	 * Finds @p w's menubar in @see menus, and then activates it.
	 *
	 * @param w the activated window.
	 */
	void activeWindowChanged( WId w );

	/**
	 * Called when the selection(selection_atom) is lost. Deletes the
	 * embedded menus, and starts listening for the selection again.
	 *
	 */
	void lostSelection();

	/**
	 * Reads whether a central menu bar should be used or not, basically.
	 */
        void readSettings();
        void claimSelection();
    private:

	/**
	 * Returns true if the selection is Not owned. That is, the menu applet
	 * isn't "running" and is listening for the selection to be released.
	 */
        bool isDisabled() const;

	WId tryTransientFor( WId w );

	/**
	 * Does some sanity checks, and then sets active_menu to @param embed.
	 */
        void activateMenu( MenuEmbed* embed );

	/**
	 * Creates msg_type_atom and selection_atom, and returns the latter.
	 */
	static Atom makeSelectionAtom();
        void updateTopEdgeOffset();
	KWinModule* module;

	/**
	 * List of all known menus.
	 */
	QList< MenuEmbed* > menus;

	/**
	 * A pointer to the current active menu, which is member
	 * of @ref menus.
	 */
	MenuEmbed* active_menu;

	KSelectionOwner* selection;

	/**
	 * Only the messenger. Dispatches signals to  claimSelection().
	 */
        KSelectionWatcher* selection_watcher;

	/**
	 * Whether the Desktop menu should be used, when a window
	 * with no menu is used.
	 */
        bool desktop_menu;
	/**
	 * The distance to the top of the screen.
	 */
        int topEdgeOffset;
    };

/**
 *
 * @author Siegfried Nijssen <snijssen@liacs.nl>
 * @author Lubos Lunak <l.lunak@suse.cz>
 */
class MenuEmbed : public QX11EmbedWidget
{
    Q_OBJECT
    public:

	/**
	 * Default constructor
	 *
	 * @param mainwindow window ID for the window to be plugged
	 * @param desktop true if @p mainwindow is the desktop
	 */
	MenuEmbed( WId mainwindow, bool desktop,
            QWidget* parent = NULL, const char* name = NULL );

	/**
	 * @returns the window ID for the handled window.
	 */
	WId mainWindow() const;

	/**
	 */
        bool isDesktopMenu() const;
        virtual void setMinimumSize( int w, int h );
        void setMinimumSize( const QSize& s ) { setMinimumSize( s.width(), s.height()); }
    protected:
	/**
	 * When @p w is None, that is the embedded window was lost, it calls
	 * menuLost() such that the this is deleted from @ref menus list.
	 */
	virtual void windowChanged( WId w );

	virtual bool x11Event( XEvent* ev );
    private:
        void sendSyntheticConfigureNotifyEvent();
	WId main_window;

	/**
	 * If the window is the desktop window.
	 */
        bool desktop;
    };

inline
bool Applet::isDisabled() const
    {
    assert( ( selection == NULL && selection_watcher != NULL )
        || ( selection != NULL && selection_watcher == NULL ));
    return selection == NULL;
    }

inline
WId MenuEmbed::mainWindow() const
    {
    return main_window;
    }

inline
bool MenuEmbed::isDesktopMenu() const
    {
    return desktop;
    }

} // namespace

#endif
