/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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
#include <plasma/applet.h>

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
        * The menu we like to display.
        */
        enum ViewType {
            Combined = 0, ///< Standard Menu
            Favorites, ///< Favorites Menu
            Applications, ///< Applications Menu
            Computer, ///< Computer Menu
            RecentlyUsed, ///< Recently Used Menu
            Leave ///< Leave Menu
        };

        /**
        * How the text of the menuitems got formatted.
        */
        enum FormatType {
            Name = 0, ///< Name only
            Description, ///< Description only
            NameDescription, ///< Name Description
            DescriptionName ///< Description (Name)
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

        void startMenuEditor();

protected:
        /**
         * Create a configuration dialog.
         */
        void createConfigurationInterface(KConfigDialog *parent);

private Q_SLOTS:
        void configAccepted();
        void toggleMenu(bool pressed);
        void toggleMenu();
        void actionTriggered(QAction *action);
        void menuDestroyed();

private:
        class Private;
        Private * const d;
};

K_EXPORT_PLASMA_APPLET(menulauncher, MenuLauncherApplet)

#endif
