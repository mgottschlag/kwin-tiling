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


#include "xrandroutputs.h"
#include "edid.h"

#include "xrandr12/randrscreen.h"
#include "xrandr12/randroutput.h"

#include <X11/Xatom.h>

#include <QX11Info>

#include <KDebug>
#include <QX11Info>

namespace Kephal {

    XRandROutputs::XRandROutputs(QObject * parent, RandRDisplay * display)
            : BackendOutputs(parent)
    {
        m_display = display;
        init();
    }

    QList<Output *> XRandROutputs::outputs() {
        QList<Output *> result;
        foreach (XRandROutput * output, m_outputs) {
            result.append(output);
        }
        return result;
    }

    void XRandROutputs::init() {
        kDebug();
        RandRScreen * screen = m_display->screen(0);
        foreach (RandROutput * output, screen->outputs()) {
            XRandROutput * o = new XRandROutput(this, output->id());

            connect(o, SIGNAL(outputConnected(Kephal::Output*)),
                    this, SIGNAL(outputConnected(Kephal::Output*)));

            connect(o, SIGNAL(outputDisconnected(Kephal::Output*)),
                    this, SIGNAL(outputDisconnected(Kephal::Output*)));

            connect(o, SIGNAL(outputActivated(Kephal::Output*)),
                    this, SIGNAL(outputActivated(Kephal::Output*)));

            connect(o, SIGNAL(outputDeactivated(Kephal::Output*)),
                    this, SIGNAL(outputDeactivated(Kephal::Output*)));

            connect(o, SIGNAL(outputResized(Kephal::Output*,QSize,QSize)),
                    this, SIGNAL(outputResized(Kephal::Output*,QSize,QSize)));

            connect(o, SIGNAL(outputMoved(Kephal::Output*,QPoint,QPoint)),
                    this, SIGNAL(outputMoved(Kephal::Output*,QPoint,QPoint)));

            connect(o, SIGNAL(outputRateChanged(Kephal::Output*,float,float)),
                    this, SIGNAL(outputRateChanged(Kephal::Output*,float,float)));

            connect(o, SIGNAL(outputRotated(Kephal::Output*,Kephal::Rotation,Kephal::Rotation)),
                    this, SIGNAL(outputRotated(Kephal::Output*,Kephal::Rotation,Kephal::Rotation)));
            connect(o, SIGNAL(outputReflected(Kephal::Output*,bool,bool,bool,bool)),
                    this, SIGNAL(outputReflected(Kephal::Output*,bool,bool,bool,bool)));

            kDebug() << "  added output " << output->id();
            m_outputs.insert(o->id(), o);
        }
    }

    RandROutput * XRandROutputs::output(RROutput rrId) {
        return m_display->screen(0)->outputs()[rrId];
    }

    RandRDisplay * XRandROutputs::display() {
        return m_display;
    }

    XRandROutput::XRandROutput(XRandROutputs * parent, RROutput rrId)
            : BackendOutput(parent), m_productId(-1), m_serialNumber(0)
    {
        m_outputs = parent;
        m_rrId = rrId;

        parseEdid();

        saveAsPrevious();

        connect(output(), SIGNAL(outputChanged(RROutput,int)),
                this, SLOT(outputChanged(RROutput,int)));
        //connect(this, SLOT(_activate()), output(), SLOT(slotEnable()));
        //connect(this, SLOT(_deactivate()), output(), SLOT(slotDisable()));
    }

    void XRandROutput::parseEdid() {

        Atom atom = XInternAtom (QX11Info::display(), "EDID_DATA", false);
        Atom type;
        unsigned char * data;
        unsigned long size;
        unsigned long after;
        int format;

        XRRGetOutputProperty(QX11Info::display(), m_rrId, atom, 0, 100,
                False, False, AnyPropertyType,
                &type, &format, &size, &after, &data);

        if (type == XA_INTEGER && format == 8 && EDID_TEST_HEADER(data)) {
            //kDebug() << "got a valid edid block...";

            /**
             * parse the 3 letter vendor code
             */
            char * vendor = new char[4];

            vendor[0] = EDID_VENDOR_1(data);
            vendor[1] = EDID_VENDOR_2(data);
            vendor[2] = EDID_VENDOR_3(data);
            vendor[3] = 0x00;
            m_vendor = vendor;

            kDebug() << "vendor code:" << m_vendor;

            delete[] vendor;

            /**
             * parse the 16bit product id
             */
            m_productId = EDID_PRODUCT_ID(data);

            kDebug() << "product id:" << m_productId;

            /**
             * parse the 32bit serial number
             */
            m_serialNumber = EDID_SERIAL_NUMBER(data);

            kDebug() << "serial number:" << m_serialNumber;
        } else {
            m_vendor = QString();
            m_productId = -1;
            m_serialNumber = 0;
        }

        XFree(data);
    }

    void XRandROutput::outputChanged(RROutput id, int changes) {
        Q_ASSERT(id == m_rrId);
        kDebug() << isConnected() << isActivated() << geom();
        if (isConnected() != m_previousConnected) {
            if (isConnected()) {
                saveAsPrevious();
                parseEdid();
                emit outputConnected(this);
                if (isActivated()) {
                    emit outputActivated(this);
                }
            } else {
                if (m_previousActivated) {
                    saveAsPrevious();
                    emit outputDeactivated(this);
                }
                saveAsPrevious();
                emit outputDisconnected(this);
            }
            return;
        }
        if (! isConnected()) {
            return;
        }
        if (isActivated() != m_previousActivated) {
            saveAsPrevious();
            if (isActivated()) {
                emit outputActivated(this);
            } else {
                emit outputDeactivated(this);
            }
            return;
        }

        QRect previousGeom = m_previousGeom;
        Rotation previousRotation = m_previousRotation;
        float previousRate = m_previousRate;
        bool previousReflectX = m_previousReflectX;
        bool previousReflectY = m_previousReflectY;
        saveAsPrevious();
        if (size() != previousGeom.size()) {
            emit outputResized(this, previousGeom.size(), size());
        }
        if (position() != previousGeom.topLeft()) {
            emit outputMoved(this, previousGeom.topLeft(), position());
        }
        if (rotation() != previousRotation) {
            emit outputRotated(this, previousRotation, rotation());
        }
        if (rate() != previousRate) {
            emit outputRateChanged(this, previousRate, rate());
        }
        if ((reflectX() != previousReflectX) || (reflectY() != previousReflectY)) {
            emit outputReflected(this, previousReflectX, previousReflectY, reflectX(), reflectY());
        }
    }

    void XRandROutput::saveAsPrevious() {
        m_previousConnected = isConnected();
        m_previousActivated = isActivated();
        m_previousGeom = geom();
        m_previousRotation = rotation();
        m_previousRate = rate();
        m_previousReflectX = reflectX();
        m_previousReflectY = reflectY();
    }

    bool XRandROutput::applyGeom(const QRect & geom, float rate) {
        if ((geom == this->geom()) && ((rate < 1) || (qFuzzyCompare(rate, this->rate())))) {
            return true;
        }

        output()->proposeRect(geom);
        if (rate < 1) {
            rate = output()->refreshRate();
        }
        bool found = false;
        QList<float> rates = output()->refreshRates(geom.size());
        foreach (float r, rates) {
            if (qFuzzyCompare(rate, r)) {
                rate = r;
                found = true;
                break;
            }
        }
        if ((! found) && (! rates.empty())) {
            rate = rates[0];
        }
        if (rate > 1) {
            output()->proposeRefreshRate(rate);
        }

        return output()->applyProposed();
    }

    bool XRandROutput::applyOrientation(Rotation rotation, bool reflectX, bool reflectY) {
        if ((rotation == this->rotation()) && (reflectX == this->reflectX()) && (reflectY == this->reflectY())) {
            return true;
        }

        int orientation = 0;

        switch (rotation) {
            case RotateRight:
                orientation |= RandR::Rotate90;
                break;
            case RotateLeft:
                orientation |= RandR::Rotate270;
                break;
            case RotateInverted:
                orientation |= RandR::Rotate180;
                break;
            default:
                orientation |= RandR::Rotate0;
        }

        if (reflectX) {
            orientation |= RandR::ReflectX;
        }
        if (reflectY) {
            orientation |= RandR::ReflectY;
        }

        output()->proposeRotation(orientation);
        return output()->applyProposed();
    }

    void XRandROutput::deactivate()
    {
        output()->slotDisable();
    }

    RandROutput * XRandROutput::output() const
    {
        return m_outputs->output(m_rrId);
    }

    QString XRandROutput::id() const
    {
        return output()->name();
    }

    QSize XRandROutput::size() const
    {
        return output()->rect().size();
    }

    QSize XRandROutput::preferredSize() const
    {
        if (output()->preferredMode().size().isEmpty()) {
            return QSize();
        } else {
            return output()->preferredMode().size();
        }
    }

    QList<QSize> XRandROutput::availableSizes() const {
        QList<QSize> sizes = output()->sizes();
        return sizes;
    }

    QPoint XRandROutput::position() const {
        return output()->rect().topLeft();
    }

    bool XRandROutput::isConnected() const {
        return output()->isConnected();
    }

    bool XRandROutput::isActivated() const
    {
        return output()->isActive();
    }

    QString XRandROutput::vendor() const
    {
        return m_vendor;
    }

    int XRandROutput::productId() const
    {
        return m_productId;
    }

    unsigned int XRandROutput::serialNumber() const
    {
        return m_serialNumber;
    }

    RROutput XRandROutput::_id() const {
        return m_rrId;
    }

    Rotation XRandROutput::rotation() const
    {
        switch (output()->rotation() & RandR::RotateMask) {
            case RandR::Rotate90:
                return RotateRight;
            case RandR::Rotate180:
                return RotateInverted;
            case RandR::Rotate270:
                return RotateLeft;
            default:
                return RotateNormal;
        }
    }

    bool XRandROutput::reflectX() const
    {
        return (output()->rotation() & RandR::ReflectX);
    }

    bool XRandROutput::reflectY() const
    {
        return (output()->rotation() & RandR::ReflectY);
    }

    float XRandROutput::rate() const
    {
        return output()->refreshRate();
    }

    QList<float> XRandROutput::availableRates() const
    {
        return output()->refreshRates(size());
    }

    void XRandROutput::resize(const QSize & size)
    {
        Q_UNUSED(size)
#warning implement!
    }

    void XRandROutput::rotate(Rotation rotation)
    {
        Q_UNUSED(rotation)
#warning implement!
    }

    void XRandROutput::setReflectX(bool reflect)
    {
        Q_UNUSED(reflect)
#warning implement!
    }

    void XRandROutput::setReflectY(bool reflect)
    {
        Q_UNUSED(reflect)
#warning implement!
    }

    void XRandROutput::changeRate(double rate)
    {
        Q_UNUSED(rate)
#warning implement!
    }

}

