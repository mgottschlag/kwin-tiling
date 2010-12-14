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

#include <QMap>
#include <QObject>
#include <QRect>

#include "kephal.h"

namespace Kephal {

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
             *
             * In case of XRandR 1.2 these will
             * be the names such as: VGA, LVDS,
             * TMDS-1.
             */
            virtual QString id() const = 0;

            /**
             * Returns the actual size in pixels
             * if the Output is active.
             */
            virtual QSize size() const = 0;

            /**
             * Returns the actual position in
             * pixels if the Output is active.
             */
            virtual QPoint position() const = 0;

            /**
             * Returns whether this Output is
             * currently connected to a
             * monitor.
             */
            virtual bool isConnected() const = 0;

            /**
             * Returns whether this Output is
             * currently activated and part
             * of a Screen.
             */
            virtual bool isActivated() const = 0;

            /**
             * Returns a list of sizes, which
             * are supported by this Output.
             * This depends on the connected
             * monitor.
             */
            virtual QList<QSize> availableSizes() const = 0;

            /**
             * Returns the vendor-code as
             * it is part of the EDID-block.
             */
            virtual QString vendor() const = 0;

            /**
             * Returns the product-id as
             * it is part of the EDID-block.
             */
            virtual int productId() const = 0;

            /**
             * Returns the serial-number as
             * it is part of the EDID-block.
             */
            virtual unsigned int serialNumber() const = 0;

            /**
             * Returns the preferred size of
             * this Output. This depends on
             * the connected monitor.
             */
            virtual QSize preferredSize() const = 0;

            /**
             * Returns the current rotation of
             * this Output.
             */
            virtual Rotation rotation() const = 0;

            /**
             * Returns whether this Output is
             * currently reflected over the
             * x-axis.
             */
            virtual bool reflectX() const = 0;

            /**
             * Returns whether this Output is
             * currently reflected over the
             * y-axis.
             */
            virtual bool reflectY() const = 0;

            /**
             * Returns the current refresh-rate
             * of this Output.
             */
            virtual float rate() const = 0;

            /**
             * Returns all possible refresh-rates
             * of this Output.
             * This depends on the connected
             * monitor.
             */
            virtual QList<float> availableRates() const = 0;

            /**
             * This is just a convenience
             * method for looking up the
             * Screen this Output belongs to.
             * Returns 0 if not active.
             * WILL Outputs should not know about Screens
             */
            Screen * screen() const;

            /**
             * This convenience method
             * returns size and position of
             * this Output if active.
             */
            QRect geom() const;

            /**
             * Returns all available
             * positions for this Output.
             * This depends on the
             * Configuration used and
             * the positions of the other
             * active Outputs.
             */
            QList<QPoint> availablePositions();

            virtual void resize(const QSize & size) = 0;

            virtual void rotate(Rotation rotation) = 0;

            virtual void setReflectX(bool reflect) = 0;

            virtual void setReflectY(bool reflect) = 0;

            virtual void changeRate(double rate) = 0;
    };



    /**
     * Outputs is the entrance-point to all Output
     * related operations.
     * Use: Outputs::self() to obtain the currently
     * active instance.
     */
    class Outputs : public QObject {
        Q_OBJECT
        public:
            /**
             * Returns the currently active
             * instance.
             */
            static Outputs * self();

            Outputs(QObject * parent);
            virtual ~Outputs();

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
            void outputConnected(Kephal::Output * o);

            /**
             * This signal is emitted when an Output
             * is disconnected.
             */
            void outputDisconnected(Kephal::Output * o);

            /**
             * This signal is emitted when an Output
             * is activated.
             */
            void outputActivated(Kephal::Output * o);

            /**
             * This signal is emitted when an Output
             * is deactivated.
             */
            void outputDeactivated(Kephal::Output * o);

            /**
             * This signal is emitted when an Output
             * is resized from oldSize to newSize.
             */
            void outputResized(Kephal::Output * o, QSize oldSize, QSize newSize);

            /**
             * This signal is emitted when an Output
             * is moved on the framebuffer from
             * oldPosition to newPosition.
             */
            void outputMoved(Kephal::Output * o, QPoint oldPosition, QPoint newPosition);

            /**
             * This signal is emitted when the refresh-rate
             * of Output o changes.
             */
            void outputRateChanged(Kephal::Output * o, float oldRate, float newRate);

            /**
             * This signal is emitted when the rotation of
             * Output o changes.
             */
            void outputRotated(Kephal::Output * o, Kephal::Rotation oldRotation, Kephal::Rotation newRotation);

            /**
             * This signal is emitted when the reflection
             * state of Output o is changed.
             */
            void outputReflected(Kephal::Output * o, bool oldX, bool oldY, bool newX, bool newY);

        protected:
            static Outputs * s_instance;
    };

}


#endif // KEPHAL_OUTPUTS_H

