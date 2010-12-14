/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef KEPHAL_SCREENS_H
#define KEPHAL_SCREENS_H

#include <QObject>
#include <QRect>

#include "kephal_export.h"

namespace Kephal {

    class Output;


    /**
     * A Screen is the area that is significant for
     * displaying windows. or panels.
     * Every activated Output belongs to exactly one
     * Screen and is completely contained within that
     * Screen. No two Screens overlap each other.
     *
     * Hence:
     * 1 Output -> 1 Screen
     * 2 Non-overlapping Outputs -> 2 Screen
     * 2 Overlapping Outputs -> 1 Screen
     */
    class KEPHAL_EXPORT Screen : public QObject {
        Q_OBJECT
        public:
            Screen(QObject * parent = 0);

            /**
             * Returns the id of this Screen. The id
             * is part of the Configuration and will
             * be the same whenever the same Outputs
             * are used with the same Configuration.
             */
            virtual int id() = 0;

            /**
             * The actual size of the screen in pixels.
             * This is the smallest area possible, so
             * that all Outputs are completely
             * contained.
             */
            virtual QSize size() = 0;

            /**
             * The actual position on the framebuffer
             * in pixels.
             */
            virtual QPoint position() = 0;

            /**
             * Returns whether this Screen is to be
             * considered in privacy-mode.
             * In this mode no content should be
             * displayed on that screen unless the
             * user forces this.
             */
            virtual bool isPrivacyMode() = 0;

            /**
             * Sets the state of the privacy-mode.
             */
            virtual void setPrivacyMode(bool b) = 0;

            /**
             * Return a list of Outputs that are
             * currently part of this Screen.
             */
            virtual QList<Output *> outputs() = 0;

            /**
             * Returns whether this screen is the
             * current primary screen.
             * This is just a convenience method,
             * since the real value is determined
             * by the configuration used.
             */
            bool isPrimary() const;

            /**
             * Make this Screen the primary one.
             * This just calls the appropriate
             * method in the Configuration.
             */
            void setAsPrimary();

            /**
             * Returns the position and size of the
             * Screen as QRect.
             * This is just a convenience method.
             */
            QRect geom();
    };



    /**
     * Screens is the entrance-point for all Screen-
     * related operations.
     * Use: Screens::self() for the currently
     * active instance.
     */
    class KEPHAL_EXPORT Screens : public QObject {
        Q_OBJECT
        public:
            /**
             * Returns the currently active instance.
             */
            static Screens * self();

            Screens(QObject * parent);
            virtual ~Screens();

            /**
             * Returns the list of all current
             * Screens.
             * Every Screen has at least one Output
             * and a non-zero size.
             */
            virtual QList<Screen *> screens() = 0;

            /**
             * Find a Screen by its id.
             */
            virtual Screen * screen(int id);

            /**
             * Returns the current primary Screen.
             */
            Screen * primaryScreen();

        Q_SIGNALS:
            /**
             * This signal is emitted when a new
             * Screen appears, due to an Output
             * being activated or the Configuration
             * being changed.
             */
            // QDW::screenCountChanged
            void screenAdded(Kephal::Screen * s);

            /**
             * This signal is emitted when a
             * Screen disappears, due to an Output
             * being deactivated or the
             * Configuration being changed.
             */
            // QDW::screenCountChanged
            void screenRemoved(int id);

            /**
             * This signal is emitted when the size
             * of the Screen changes.
             */
            // QDW::resized
            // TODO: move to Screen and replace with no args version
            void screenResized(Kephal::Screen * s, QSize oldSize, QSize newSize);

            /**
             * This signal is emitted when the
             * position of the Screen changes.
             */
            // QDW::resized emitted when position changes too - but not documented.
            // TODO: move to Screen and replace with no args version
            void screenMoved(Kephal::Screen * s, QPoint oldPosition, QPoint newPosition);

        protected:
            static Screens * s_instance;
    };

    /**
     * Defines a handful help methods for common tasks
     */
    class KEPHAL_EXPORT ScreenUtils {
        public:
            /** Returns the number of screens. */
            // PASSTHRU to QDW
            static int numScreens();

            /** Returns the geometry of the given screen */
            // PASSTHRU to QDW
            static QRect screenGeometry(int id);

            /** Returns the size of the given screen */
            // SYNTACTIC SUGAR around QDW
            static QSize screenSize(int id);

            /** Returns the geometry of the whole desktop */
            // SYNTACTIC SUGAR around QApplication::desktop()->geometry()
            static QRect desktopGeometry();

            /** Returns the id of the screen that contains the given point */
            // SYNTACTIC SUGAR around QDW::screenNumber()
            static int screenId(QPoint p);

            /** Returns the id of the primary screen */
            // SYNTACTIC SUGAR around QDW::primaryScreen
            static int primaryScreenId();

        private:
            static int distance(const QRect & r, const QPoint & p);
    };

}


#endif // KEPHAL_SCREENS_H

