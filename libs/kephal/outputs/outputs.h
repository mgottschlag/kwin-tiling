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


#ifndef KEPHAL_OUTPUTS_H
#define KEPHAL_OUTPUTS_H

#include <QObject>
#include <QSize>
#include <QPoint>
#include <QRect>
#include <QMap>

#include "kephal.h"


namespace kephal {

    class Screen;


    /**
     * An Output is the actual connector and a
     * possibly connected monitor, such as
     * VGA, LVDS, TMDS-1.
     * This is most important for changing any
     * settings of the current setup.
     */
    class Output : public QObject {
        Q_OBJECT
        public:
            Output(QObject * parent);

            /**
             * Returns the id of the Output.
             * In case of XRandR 1.2 these will
             * be the names such as: VGA, LVDS,
             * TMDS-1.
             */
            virtual QString id() = 0;

            /**
             * Returns the actual size in pixels
             * if the Output is active.
             */
            virtual QSize size() = 0;

            /**
             * Returns the actual position in
             * pixels if the Output is active.
             */
            virtual QPoint position() = 0;
            
            /**
             * Returns whether this Output is
             * currently connected to a
             * monitor.
             */
            virtual bool isConnected() = 0;
            
            /**
             * Returns whether this Output is
             * currently activated and part
             * of a Screen.
             */
            virtual bool isActivated() = 0;
            
            /**
             * Returns a list of sizes, which
             * are supported by this Output.
             * This depends on the connected
             * monitor.
             */
            virtual QList<QSize> availableSizes() = 0;
            
            /**
             * Returns the vendor-code as
             * it is part of the EDID-block.
             */
            virtual QString vendor() = 0;
            
            /**
             * Returns the product-id as
             * it is part of the EDID-block.
             */
            virtual int productId() = 0;
            
            /**
             * Returns the serial-number as
             * it is part of the EDID-block.
             */
            virtual unsigned int serialNumber() = 0;
            
            /**
             * Returns the preffered size of
             * this Output. This depends on
             * the connected monitor.
             */
            virtual QSize preferredSize() = 0;
            
            /**
             * Returns the current rotation of
             * this Output.
             */
            virtual Rotation rotation() = 0;
            
            /**
             * Returns wether this Output is
             * currently reflected over the
             * x-axis.
             */
            virtual bool reflectX() = 0;
            
            /**
             * Returns wether this Output is
             * currently reflected over the
             * y-axis.
             */
            virtual bool reflectY() = 0;
            
            /**
             * Returns the current refresh-rate
             * of this Output.
             */
            virtual float rate() = 0;
            
            /**
             * Returns all possible refresh-rates
             * of this Output.
             * This depends on the connected
             * monitor.
             */
            virtual QList<float> availableRates() = 0;
            
            /**
             * This is just a convenience
             * method for looking up the 
             * Screen this Output belongs to.
             * Returns 0 if not active.
             */
            Screen * screen();
            
            /**
             * This convenience method
             * returns size and position of
             * this Output if active.
             */
            QRect geom();
            
            /**
             * Returns all available
             * positions for this Output.
             * This depends on the
             * Configuration used and
             * the positions of the other
             * active Outputs.
             */
            QList<QPoint> availablePositions();
            
        public Q_SLOTS:
            /**
             * This calls the appropriate
             * methods in the Configuration
             * to resize this Output.
             */
            void resize(const QSize & size);

            /**
             * This calls the appropriate
             * methods in the Configuration
             * to move this Output.
             */
            void move(const QPoint & position);
            
            /**
             * This will set this Ouputs rotation
             * to the given value.
             */
            void rotate(Rotation rotation);
            
            /**
             * This will set this Output to be
             * reflected over the x-axis if reflect
             * is true.
             */
            void reflectX(bool reflect);
            
            /**
             * This will set this Output to be
             * reflected over the y-axis if reflect
             * is true.
             */
            void reflectY(bool reflect);
            
            /**
             * This will change this Outputs
             * refresh-rate to rate.
             */
            void changeRate(double rate);
    };
    


    /**
     * Outputs is the entrance-point to all Output
     * related operations.
     * Use: Outputs::instance() to obtain the currently
     * active instance.
     */
    class Outputs : public QObject {
        Q_OBJECT
        public:
            /**
             * Returns the currently active
             * instance.
             */
            static Outputs * instance();
            
            Outputs(QObject * parent);
            
            /**
             * Returns a list of all known Outputs,
             * even if they are inactive or
             * disconnected.
             */
            virtual QList<Output *> outputs() = 0;
            
            /**
             * Find an Output by its id.
             * Returns 0 if the id is not known.
             */
            virtual Output * output(const QString & id);
            
        Q_SIGNALS:
            /**
             * This signal is emitted when an Output
             * is connected.
             */
            void outputConnected(kephal::Output * o);

            /**
             * This signal is emitted when an Output
             * is disconnected.
             */
            void outputDisconnected(kephal::Output * o);

            /**
             * This signal is emitted when an Output
             * is activated.
             */
            void outputActivated(kephal::Output * o);

            /**
             * This signal is emitted when an Output
             * is deactivated.
             */
            void outputDeactivated(kephal::Output * o);

            /**
             * This signal is emitted when an Output
             * is resized from oldSize to newSize.
             */
            void outputResized(kephal::Output * o, QSize oldSize, QSize newSize);

            /**
             * This signal is emitted when an Output
             * is moved on the framebuffer from
             * oldPosition to newPosition.
             */
            void outputMoved(kephal::Output * o, QPoint oldPosition, QPoint newPosition);
            
            /**
             * This signal is emitted when the refresh-rate
             * of Output o changes.
             */
            void outputRateChanged(kephal::Output * o, float oldRate, float newRate);
            
            /**
             * This signal is emitted when the rotation of
             * Output o changes.
             */
            void outputRotated(kephal::Output * o, kephal::Rotation oldRotation, kephal::Rotation newRotation);
            
            /**
             * This signal is emitted when the reflection
             * state of Output o is changed.
             */
            void outputReflected(kephal::Output * o, bool oldX, bool oldY, bool newX, bool newY);
            
        protected:
            static Outputs * m_instance;
    };
    
}


#endif // KEPHAL_OUTPUTS_H

