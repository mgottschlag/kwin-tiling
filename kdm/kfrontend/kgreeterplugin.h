/*

    Authentication method specific conversation plugin for KDE's greeter widgets

    Copyright (C) 2003 Oswald Buddenhagen <ossi@kde.org>
    Copyright (C) 2003 Fabian Kaiser <xfk@softpro.de>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KGREETERPLUGIN_H
#define KGREETERPLUGIN_H

#include <qvariant.h>

class QWidget;
class QLayoutItem;

class KSimpleConfig;

class KGreeterPluginHandler {
public:
    /* keep in sync with V_IS_* */
    enum { IsUser = 1, IsPassword = 2 };
    /**
     * reply to textPrompt
     * text: text to return to core; null to abort auth cycle
     * tag: 0 or one of Is*
     */
    virtual void gplugReturnText( const char *text, int tag ) = 0;
    /**
     * reply to binaryPrompt
     * data: data in pam_client format to return to the core;
     *  null to abort auth cycle
     */
    virtual void gplugReturnBinary( const char *data ) = 0;
    /**
     * Tell the greeter who is logging in.
     * Call this preferably before gplugStart, as otherwise the .dmrc
     * load will be delayed. Don't call at all if your plugin doesn't
     * have the Local flag set.
     */
    virtual void gplugSetUser( const QString &user ) = 0;
    /**
     * start processing. the handler has to know the mode itself.
     */
    virtual void gplugStart( const char *method ) = 0;
};

/**
 * Abstract base class for any authentication frontend modules to be used with KDM.
 */
class KGreeterPlugin {
public:
    KGreeterPlugin::KGreeterPlugin( KGreeterPluginHandler *h ) : handler( h ) {}
    virtual ~KGreeterPlugin() {}

    /**
     * Variations of the widget:
     * - Authenticate: authentication
     * - AuthChAuthTok: authentication and password change
     * - ChAuthTok: password change
     */
    enum Function { Authenticate, AuthChAuthTok, ChAuthTok };
    /**
     * The Ex* functions will be called from within a running session;
     * they must know how to obtain the currently logged in user
     * (+ domain/realm, etc.) themselves. The non-Ex variants will have a
     * fixedEntity passed in the right contexts.
     * TODO: Unlock and ChangeTok are currently not used in kdm. ExChangeTok
     * should be used by kdepasswd.
     */
    enum Context { Login, Shutdown, Unlock, ChangeTok,
		   ExUnlock, ExChangeTok };

    /**
     * Preload the widget with an (opaque to the greeter) entity. That
     * will usually be something like "user" or "user@domain".
     * Can be called only when not running.
     */
    virtual void presetEntity( const QString &entity, int field ) = 0;

    /**
     * Obtain the actually logged in entity.
     * Can be called only after succeeded() was called.
     */
    virtual QString getEntity() const = 0;

    /**
     * A user was selected in the greeter outside the conversation widget
     * (clicking into the user list or successful authentication without
     * prior gplugSetUser call).
     * Can be called only when running.
     */
    virtual void setUser( const QString &user ) = 0;

    /**
     * En-/disable any widgets.
     * Can be called only when not running.
     */
    virtual void setEnabled( bool on ) = 0;

    /**
     * Prompt the user for data. Reply by calling handler->gplugReturnText().
     * If nonBlocking is true report whatever is already available,
     * otherwise wait for user input.
     * Prompt may be null, in which case "Username"/"Password" should
     * be shown and the replies should be tagged with the respective Is*.
     */
    virtual void textPrompt( const char *prompt, bool echo, bool nonBlocking ) = 0;

    /**
     * Request binary authentication data from the plugin.<br>
     * Reply by calling handler->gplugReturnBinary().
     * If nonBlocking is true report whatever is already available,
     * otherwise wait for user input.<br>
     * Both the input and the output are expected to be in pam_client format.<br>
     * TODO:<br>
     * The plugin may choose to direct actually obtaining the authentication
     * data to the backend (which will use libpam_client); in this case the
     * returned array must consist of four zeros.
     */
    virtual void binaryPrompt( const char *prompt, bool nonBlocking ) = 0;

    /**
     * This can either
     *  - Start a processing cycle. Can be called only when not running.
     *  - Restart authTok cycle - will be called while running and implies
     *    revive. PAM is a bit too clever, so we need this.
     */
    virtual void start() = 0;

    /**
     * Request to interrupt the auth.
     * This must bring the core into a serving state.
     * Can be called only if running within Login context.
     */
    virtual void suspend() = 0;

    /**
     * Request to continue the auth, if possible from the point it was
     * interrupted at.
     * Can be called only when suspended.
     */
    virtual void resume() = 0;

    /**
     * The "login" button was pressed in the greeter.
     * This might call gplugReturn* or gplugStart.
     * Can be called only when running.
     */
    virtual void next() = 0;

    /**
     * Abort auth cycle.
     * Can be called only when running.
     */
    virtual void abort() = 0;

    /**
     * Indicate successful end of the current phase.
     * This is more or less a request to disable editable widgets
     * responsible for the that phase.
     * There will be no further attempt to enter that phase until the
     * widget is destroyed.
     * Can be called only when running.
     */
    virtual void succeeded() = 0;

    /**
     * Indicate unsuccessful end of the current phase.
     * This is mostly a request to disable all editable widgets.
     * The widget will be treated as dead until revive() is called.
     * Can be called only when running.
     */
    virtual void failed() = 0;

    /**
     * Prepare retrying the previously failed phase.
     * This is mostly a request to re-enable all editable widgets failed()
     * disabled previously, and to set the input focus.
     * Can be called only when not running and failed() was called before.
     */
    virtual void revive() = 0;

    /**
     * Clear any edit widgets, particularily anything set by setUser.
     * Can be called at any time, particularily even while suspended.
     */
    virtual void clear() = 0;

    /**
     * Obtain the QLayoutItem containg the widget(s) to actually handle the
     * greeting. See QLayout and QWidgetItem for possible implementations.
     */
    QLayoutItem *getLayoutItem() const { return layoutItem; }

protected:
    KGreeterPluginHandler *handler;
    QLayoutItem *layoutItem;
};

extern "C" {

struct kgreeterplugin_info {
    /**
     * Human readable name of this plugin (should be a little more
     * informative than just the libary name). Must be I18N_NOOP()ed.
     */
    const char *name;

    /*
     * Capabilities.
     */
    enum {
	/**
	 * All users exist on the local system permanently (will be listed
	 * by getpwent()); no domain/realm needs to be set interactively.
	 * Effectively means that setUser/gplugSetUser can be used and a
	 * userlist should be shown at all.
	 */
	Local = 1
    };
    /*
     * Capability flags.
     */
    int flags;
    
    /**
     * Call after loading the plugin.
     * If it returns false, unload the plugin again (don't call done() first).
     *
     * getConf can be used to obtain configuration items from the greeter;
     * you have to pass it the ctx pointer.
     * The only predefined key is "EchoMode", which is an int (in fact,
     * KPasswordEdit::EchoModes).
     * Other keys are obtained from the PluginOptions option; see kdmrc
     * for details.
     * If the key is unknown, dflt is returned.
     */
    bool (*init)( QVariant (*getConf)( void *ctx, const char *key, const QVariant &dflt ), void *ctx = 0 );
    /**
     * Call before unloading the plugin.
     */
    void (*done)( void );

    /**
     * Factory method to create an instance of the plugin.
     * Note that multiple instances can exist at one time, but only
     * one of them is active at any moment (the others would be suspended
     * or not running at all).
     * If fixedEntity is non-null, the plugin must present this information
     * somehow; possibly by keeping the normal layout but replacing line edits
     * with labels.
     */
    KGreeterPlugin *(*create)( KGreeterPluginHandler *handler,
			       QWidget *parent, QWidget *predecessor,
			       const QString &fixedEntity,
			       KGreeterPlugin::Function func,
			       KGreeterPlugin::Context ctx );

    /**
     * Determine whether this plugin is capable of handling the authentication
     * method. If method is null, it must be determined heuristically from the
     * environment. The return value is a certainity factor in percent -
     * effectively a priority.
     */
    int (*capable)( const char *method );
};

}

#endif
