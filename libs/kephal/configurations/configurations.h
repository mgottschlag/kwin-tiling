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


#ifndef KEPHAL_CONFIGURATIONS_H
#define KEPHAL_CONFIGURATIONS_H

#include <QObject>
#include <QMap>
#include <QPoint>
#include <QRect>


namespace kephal {

    class Output;


    /**
     * A Configuration allows to change settings that
     * control which Output belongs to which Screen or
     * is inactive.
     */
    class Configuration : public QObject {
        Q_OBJECT
        public:
            Configuration(QObject * parent);

            /**
             * The name of the Configuration.
             * This name uniquely identifies the
             * Configuration.
             */
            virtual QString name() = 0;
            
            /**
             * Returns whether this Configuration
             * can be modified.
             * This includes the layout and settings
             * like privacy-mode and primary Screen.
             */
            virtual bool isModifiable() = 0;
            
            /**
             * Returns whether this Configuration is
             * currently active.
             * Only 1 Configuration can be active at
             * any time.
             */
            virtual bool isActivated() = 0;
            
            /**
             * Returns the layout as Screens of size
             * 1x1.
             */
            virtual QMap<int, QPoint> layout() = 0;
            
            /**
             * Returns the id of the primary Screen for
             * this Configuration.
             */
            virtual int primaryScreen() = 0;
            
            /**
             * Returns the real layout, with screen-sizes
             * taken from the actual Outputs.
             *
             * @param simpleLayout The layout as returned
             *          from layout().
             * @param outputScreens A mapping of Outputs
             *          to Screens.
             */
            QMap<int, QRect> realLayout(const QMap<int, QPoint> & simpleLayout, const QMap<Output *, int> & outputScreens);

            /**
             * Returns the real layout, with screen-sizes
             * taken from the actual Outputs.
             * This will calculate the layout by calling
             * layout().
             *
             * @param outputScreens A mapping of Outputs
             *          to Screens.
             */
            QMap<int, QRect> realLayout(const QMap<Output *, int> & outputScreens);

            /**
             * Returns the real layout, with screen-sizes
             * taken from the actual Outputs.
             * This will calculate the layout by calling
             * layout() and use the Output to Screen
             * mapping as currently active if possible.
             */
            QMap<int, QRect> realLayout();
            
            /**
             * Returns a set of points covered in the
             * layout returned by layout().
             */
            QSet<QPoint> positions();
            
            /**
             * Returns the positions as in positions
             * to which the Screen can be cloned.
             */
            QSet<QPoint> clonePositions(int screen);
            
            /**
             * Returns the layout if the Screen screen
             * was to be cloned to any of the other
             * Screens.
             */
            QMap<int, QPoint> cloneLayout(int screen);
            
            /**
             * Returns the possible positions as in
             * positions() to move the Screen screen
             * to.
             */
            QSet<QPoint> possiblePositions(int screen);
            
        public Q_SLOTS:
            /**
             * Activate this Configuration.
             */
            virtual void activate() = 0;
            
        private:
            void simpleToReal(QMap<int, QPoint> & simpleLayout, const QMap<int, QSize> & screenSizes, int index, QMap<int, QRect> & screens);
            QList<QSet<QPoint> > partition(int screen);
            QSet<QPoint> border(QSet<QPoint> screens);
    };
    


    /**
     * Configurations is the entrance-point to all
     * Configuration related operations.
     * Use: Configurations::instance() to obtain the currently
     * active instance.
     */
    class Configurations : public QObject {
        Q_OBJECT
        public:
            /**
             * Returns the currently active
             * instance.
             */
            static Configurations * instance();
            
            Configurations(QObject * parent);
            
            /**
             * Returns a list of all known Configurations.
             */
            virtual QMap<QString, Configuration *> configurations() = 0;
            
            /**
             * Find the Configuration for the currently
             * connected Outputs.
             */
            virtual Configuration * findConfiguration() = 0;
            
            /**
             * Returns the currently active Configuration.
             */
            virtual Configuration * activeConfiguration() = 0;
            
            /**
             * Returns a list of all alternate Configuratios
             * for the currently connected Outputs.
             */
            virtual QList<Configuration *> alternateConfigurations() = 0;
            
            /**
             * Returns the list of all positions in pixels
             * for the Output output.
             * These are the only positions that can be
             * passed to move().
             */
            virtual QList<QPoint> possiblePositions(Output * output) = 0;
            
            /**
             * Move Output output to position on the framebuffer.
             * This will relayout all Outputs.
             */
            virtual void move(Output * output, const QPoint & position) = 0;
            
            /**
             * Resize Output output to size.
             * This will relayout all Outputs.
             */
            virtual void resize(Output * output, const QSize & size) = 0;
            
            /**
             * Find a Configuration by its name.
             * This returns 0 if the name is not known.
             */
            virtual Configuration * configuration(QString name);
            
            static void translateOrigin(QMap<int, QPoint> & layout);
            static void translateOrigin(QMap<int, QPoint> & layout, QPoint origin);
            static void translateOrigin(QMap<int, QRect> & layout);
            static void translateOrigin(QMap<int, QRect> & layout, QPoint origin);
            
        Q_SIGNALS:
            /**
             * This signal is emitted when the active
             * Configuration is changed.
             */
            void configurationActivated(Configuration * configuration);
            
        protected:
            static Configurations * m_instance;
    };
    
}


#endif // KEPHAL_CONFIGURATIONS_H

