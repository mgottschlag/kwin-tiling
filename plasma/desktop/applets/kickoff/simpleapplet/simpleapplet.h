/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2008-2009 Sebastian Sauer <mail@dipe.org>

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

#ifndef SIMPLEAPPLET_H
#define SIMPLEAPPLET_H

// Plasma
#include <Plasma/Applet>

class QAction;

/**
* The MenuLauncherApplet class implements an applet that provides the traditional
* aka classic KDE3 like KMenu application launcher using the Kickoff functionality.
*/
class MenuLauncherApplet : public Plasma::Applet
{
    Q_OBJECT
    Q_ENUMS(ViewType)
    Q_ENUMS(FormatType)
public:

    /**
    * The content we like to display within the menu.
    */
    enum ViewType {
        RecentlyUsedApplications, ///< Recently Used Applications Menu
        RecentlyUsedDocuments, ///< Recently Used Documents Menu
        Applications, ///< Applications Menu
        Favorites, ///< Favorites Menu
        Bookmarks, ///< Bookmarks Menu
        Computer, ///< Computer Menu
        RecentlyUsed, ///< Recently Used Menu
        Settings, ///< Settings Menu
        RunCommand, ///< Run Command Action
        SwitchUser, ///< Switch User Action
        SaveSession, ///< Save Session Action (only enabled if restoreSavedSession is enabled)
        LockScreen, ///< Lock Screen Action
        Standby, ///< Standby Action
        SuspendDisk, ///< Suspend to Disk Action
        SuspendRAM, ///< Suspend to RAM Action
        Restart, ///< Restart Action
        Shutdown, ///< Shutdown Action
        Logout, ///< Logout Action
        Leave ///< Leave Menu
    };

    /**
    * How the text of the menuitems got formatted.
    */
    enum FormatType {
        Name = 0, ///< Name only
        Description, ///< Description only
        NameDescription, ///< Name Description
        DescriptionName, ///< Description (Name)
        NameDashDescription ///< Name - Description
    };

    /**
    * Constructor.
    *
    * \param parent The parent QObject.
    * \param args The optional list of arguments.
    */
    MenuLauncherApplet(QObject *parent, const QVariantList &args);

    /**
    * Destructor.
    */
    virtual ~MenuLauncherApplet();

    /**
     * This method is called once the applet is loaded and added to a Corona.
     **/
    void init();

    /**
     * Called when any of the geometry constraints have been updated.
     *
     * @param constraints the type of constraints that were updated
     */
    void constraintsEvent(Plasma::Constraints constraints);

    /**
     * Returns a list of context-related QAction instances.
     */
    virtual QList<QAction*> contextualActions();

public Q_SLOTS:
    /**
     * Switch the menu style from the traditional aka classic KDE3 like
     * KMenu to the new Kickoff menu.
     */
    void switchMenuStyle();

    /**
     * Start the menu editor by launching kmenuedit.
     */
    void startMenuEditor();

    /**
     * Show a custom context menu for the selected action.
     */
    void customContextMenuRequested(QMenu* menu, const QPoint& pos);

    /**
     * Save config values stored on Kickoff after a menu switch
     */
    void saveConfigurationFromKickoff(const KConfigGroup & configGroup,
                                      const KConfigGroup & globalConfigGroup);

    /**
     * Reload configuration values
     * Useful to load previously stored configurations after a menu switch
     */
    void configChanged();

protected:
    /**
     * Create a configuration dialog.
     */
    void createConfigurationInterface(KConfigDialog *parent);

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

private Q_SLOTS:
    /// Configuration-dialog accepted.
    void configAccepted();
    /// The menu got toggled or activated.
    void toggleMenu();
    /// Shows the menu due to being toggled / activated.
    void showMenu(bool pressed);
    /// An action within the menu got triggered.
    void actionTriggered(QAction *action);
    /// Icon size setting changed
    void iconSizeChanged(int group);
    /// Menu is hidden, reset the UI
    void menuHiding();

private:
    class Private;
    Private * const d;
};

K_EXPORT_PLASMA_APPLET(menulauncher, MenuLauncherApplet)

#endif
