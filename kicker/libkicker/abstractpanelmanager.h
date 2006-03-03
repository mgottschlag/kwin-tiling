/*****************************************************************
 *
 * Copyright (c) 2005 Aaron J. Seigo <aseigo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *****************************************************************/

#include "utils.h"

#ifndef abstractpanelmanager_h
#define abstractpanelmanager_h

class ExtensionContainer;

class KDE_EXPORT AbstractPanelManager
{
    public:

        /**
         * Returns the manager for the panels
         * Must be initialized the a subclass as self() does not do any
         * initializing itself but merely return m_manager
         */
        static AbstractPanelManager* self() { return m_manager; }

        /**
         * Returns the next panel number in the stacking order
         */
        virtual int nextPanelOrder() = 0;

        /**
         * Given a preferred position, returns the position that the extension
         * should appear at when first created.
         *
         * @param preferred the position that this extension would start at
         *        given the option of doing so
         */
         virtual Plasma::Position initialPanelPosition(Plasma::Position preferred) = 0;

        /**
         * This method returns the rectangular area defining the available
         * desktop space available to this container after removing all reserved
         * spaces due to, for instance, other panels with a higher stacking
         * priority
         *
         * @param container the container
         * @param XineramaScreen the screen that this extension is on. Use
         *        Plasma::AllXineramaScreens to signal that it is not on any
         *        specific screen
         */
        virtual QRect workArea(ExtensionContainer* container,
                               int XineramaScreen) = 0;

        /**
         * Given an extension, this method returns whether it is being used as
         * a universal menu bar
         *
         * @param container the container to check for being the menu bar
         * @return returns true if container is a menu bar, otherwise false
         */
        virtual bool isMenuBar(const QWidget* panel) const = 0;

    protected:
        AbstractPanelManager();
        virtual ~AbstractPanelManager();

        static AbstractPanelManager* m_manager;

        class Private;
        Private* d;
};

#endif // multiple inclusion guard
