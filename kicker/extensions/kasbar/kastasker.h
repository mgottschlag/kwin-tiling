// -*- c++ -*-

/* kastasker.h
**
** Copyright (C) 2001-2004 Richard Moore <rich@kde.org>
** Contributor: Mosfet
**     All rights reserved.
**
** KasBar is dual-licensed: you can choose the GPL or the BSD license.
** Short forms of both licenses are included below.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/

#ifndef KASTASKER_H
#define KASTASKER_H

#include "kasbar.h"

#include <kdemacros.h>
#include <taskmanager.h>

class KConfig;
class KMenu;
class KAction;
class KToggleAction;

class Task;
class Startup;

class KasTaskItem;
class KasStartupItem;
class KasGroupItem;
class KasTasker;
class KasGrouper;

/**
 * A KasBar that provides a taskbar using the TaskManager API.
 *
 * @author Richard Moore, rich@kde.org
 */
class KDE_EXPORT KasTasker : public KasBar
{
    Q_OBJECT
    Q_PROPERTY( bool isTopLevel READ isTopLevel )
    Q_PROPERTY( bool showClock READ showClock )
    Q_PROPERTY( bool showLoad READ showLoad )

public:
    /** Create a KasTasker widget. */
    KasTasker( Qt::Orientation o, QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0 );

    /**
     * Create a KasTasker widget that is slaved to another KasTasker. The
     * created widget will inherit the settings of the parent, but will
     * not connect to the signals of the TaskManager.
     */
    KasTasker( Qt::Orientation o, KasTasker *master,
	       QWidget *parent=0, const char *name=0, Qt::WFlags f=0 );

    /** Cleans up. */
    virtual ~KasTasker();

    /** Factory method that returns the default menu for items in the bar. */
    virtual KMenu *contextMenu();

    virtual KasTasker *createChildBar( Qt::Orientation o, QWidget *parent, const char *name=0 );

    /**
     * Returns true if this is the top KasTasker. Note that it is possible for
     * the top KasTasker to be a child of another KasBar derived class, so
     * this can return a different result to KasBar::isTopLevel().
     */
    bool isTopLevel() const { return dynamic_cast<KasTasker *>( KasBar::master() ) ? true : false; }
    KasTasker *master() const { return dynamic_cast<KasTasker *>( KasBar::master() ); }

    /** Finds the item representing a task (if there is one). */
    KasTaskItem *findItem( Task::TaskPtr  );

    /** Finds the item representing a startup (if there is one). */
    KasStartupItem *findItem( Startup::StartupPtr s );

    /** Returns true iff thumbnails are enabled. */
    bool thumbnailsEnabled() const { return enableThumbs_; }

    /** Returns true iff thumbnails should be shown in the boxes instead of icons. */
    bool embedThumbnails() const { return embedThumbs_; }

    bool showClock() const { return showClock_; }
    bool showLoad() const { return showLoad_; }

    /** Returns the proportions of the window thumbnails. */
    double thumbnailSize() const { return thumbnailSize_; }

    /** Returns true iff the startup notifier is enabled. */
    bool notifierEnabled() const { return enableNotifier_; }

    /** Returns true iff the modified flag should be shown. */
    bool showModified() const { return showModified_; }

    /** Returns true iff a progress bar should be shown for progress windows. */
    bool showProgress() const { return showProgress_; }

    /** Returns true iff we should indicate when a window demands attention. */
    bool showAttention() const { return showAttention_; }

    /** Returns true iff windows from all desktops should be displayed. */
    bool showAllWindows() const { return showAllWindows_; }

    /** Returns the delay between thumbnail updates (in seconds). */
    int thumbnailUpdateDelay() const { return thumbUpdateDelay_; }

    /** Returns true iff windows should be grouped together. */
    bool groupWindows() const { return groupWindows_; }

    /** Returns true iff windows on inactive desktops should be grouped together. */
    bool groupInactiveDesktops() const { return groupInactiveDesktops_; }

    /** Returns true iff we should only show windows that are minimized. */
    bool onlyShowMinimized() const { return onlyShowMinimized_; }

    /** Returns true if this bar is floating. */
    bool isStandAlone() const { return standalone_; }

    //
    // Internal stuff
    //

    /**
     * Converts the item for a task into a group item to which additional
     * tasks can be added.
     */
    KasGroupItem *convertToGroup( Task::TaskPtr t );

    /** Moves an item from a group into the main bar. */
    void moveToMain( KasGroupItem *gi, Task::TaskPtr t );

    /** Moves all the items from a group into the main bar and removes the group. */
    void moveToMain( KasGroupItem *gi );

public Q_SLOTS:
    /** Adds a task to the bar. */
    void addTask( Task::TaskPtr  );

    /** Removes a task from the bar. */
    void removeTask( Task::TaskPtr  );

    /** Adds a startup item to the bar. */
    void addStartup( Startup::StartupPtr  );

    /** Removes a startup item from the bar. */
    void removeStartup( Startup::StartupPtr  );

    void refreshAll();
    void refreshAllLater();
    void refreshIconGeometry();

    void setNotifierEnabled( bool enable );
    void setShowModified( bool enable );
    void setShowProgress( bool enable );
    void setShowAttention( bool enable );

    void setShowAllWindows( bool enable );
    void setGroupWindows( bool enable );
    void setGroupInactiveDesktops( bool enable );
    void setOnlyShowMinimized( bool enable );

    void setThumbnailSize( double size );
    void setThumbnailSize( int percent );
    void setThumbnailsEnabled( bool enable );
    void setThumbnailUpdateDelay( int secs );
    void setEmbedThumbnails( bool enable );

    void setShowClock( bool enable );
    void setShowLoad( bool enable );

    void showPreferences();
    void showAbout();

    /** Sets the current KConfig object. */
    void setConfig( KConfig *config );
    KConfig *config() const { return conf; }

    /** Reads the settings from the current KConfig. */
    void readConfig();
    void writeConfig();
    void writeConfigLater();
    void writeLayout();

    /** Writes the settings of this bar to the specified KConfig. */
    void writeConfig( KConfig *conf );

    void setStandAlone( bool enable );

Q_SIGNALS:
    void showAllWindowsChanged( bool );
    void groupWindowsChanged( bool );
    void showClockChanged( bool );
    void showLoadChanged( bool );

protected Q_SLOTS:
    /** Load settings from the specified configuration. */
    void readConfig( KConfig *conf );

private:
    KMenu *menu;
    KConfig *conf;
    KasGrouper *grouper;
    KToggleAction *toggleDetachedAction;
    KToggleAction *showAllWindowsAction;
    KToggleAction *groupWindowsAction;
    KToggleAction *showClockAction;
    KToggleAction *showLoadAction;
    KAction *rotateBarAction;
    bool standalone_;

    bool passive_;
    bool enableThumbs_;
    bool embedThumbs_;
    double thumbnailSize_;
    bool enableNotifier_;
    bool showModified_;
    bool showProgress_;
    bool showAllWindows_;
    int thumbUpdateDelay_;
    bool groupWindows_;
    bool groupInactiveDesktops_;
    bool showAttention_;
    bool onlyShowMinimized_;
    bool showClock_;
    KasItem *clockItem;
    bool showLoad_;
    KasItem *loadItem;
};

#endif // KASTASKER_H

