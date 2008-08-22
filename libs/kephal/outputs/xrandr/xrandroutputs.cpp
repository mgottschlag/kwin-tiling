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


#include <QDebug>

#include "xrandroutputs.h"
#include "edid.h"

#include "xrandr12/randrscreen.h"
#include "xrandr12/randroutput.h"

#include <X11/Xatom.h>


namespace kephal {

    XRandROutputs::XRandROutputs(QObject * parent, RandRDisplay * display)
            : Outputs(parent)
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
        RandRScreen * screen = m_display->screen(0);
        foreach (RandROutput * output, screen->outputs().values()) {
            XRandROutput * o = new XRandROutput(this, output->id());
            m_outputs.insert(o->id(), o);
        }
    }
    
    RandROutput * XRandROutputs::output(RROutput rrId) {
        return m_display->screen(0)->outputs()[rrId];
    }
    
    RandRDisplay * XRandROutputs::display() {
        return m_display;
    }
    
    void XRandROutputs::pollState() {
        foreach (XRandROutput * o, m_outputs) {
            output(o->_id())->pollState();
        }
    }
    
    void XRandROutputs::activateLayout(const QMap<Output *, QRect> & layout) {
        qDebug() << "activate layout:" << layout;
        
        foreach (XRandROutput * output, m_outputs) {
            if (! layout.contains(output)) {
                output->_deactivate();
            }
        }

        for (QMap<Output *, QRect>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            XRandROutput * output = (XRandROutput *) i.key();
            if (! output->_apply(i.value())) {
                qDebug() << "setting" << output->id() << "to" << i.value() << "failed!!";
                for (--i; i != layout.constBegin(); --i) {
                    output = (XRandROutput *) i.key();
                    qDebug() << "trying to revert output" << output->id();
                    output->_revert();
                }
                break;
            }
        }
    }
    
    void XRandROutputs::outputChanged(RROutput id, int changes)
    {
        Q_UNUSED(changes)
        qDebug() << "output changed:" << id;
        
        foreach (XRandROutput * output, m_outputs) {
            if (output->_id() == id) {
                output->_changed();
            }
        }
    }
    
    
    
    XRandROutput::XRandROutput(XRandROutputs * parent, RROutput rrId)
            : Output(parent)
    {
        m_outputs = parent;
        m_rrId = rrId;
        
        parseEdid();
        
        saveAsPrevious();
        
        connect(this, SIGNAL(outputConnected(kephal::Output *)), parent, SIGNAL(outputConnected(kephal::Output *)));
        connect(this, SIGNAL(outputDisconnected(kephal::Output *)), parent, SIGNAL(outputDisconnected(kephal::Output *)));
        connect(this, SIGNAL(outputActivated(kephal::Output *)), parent, SIGNAL(outputActivated(kephal::Output *)));
        connect(this, SIGNAL(outputDeactivated(kephal::Output *)), parent, SIGNAL(outputDeactivated(kephal::Output *)));
        connect(this, SIGNAL(outputResized(kephal::Output *, QSize, QSize)), parent, SIGNAL(outputResized(kephal::Output *, QSize, QSize)));
        connect(this, SIGNAL(outputMoved(kephal::Output *, QPoint, QPoint)), parent, SIGNAL(outputMoved(kephal::Output *, QPoint, QPoint)));
        
        connect(output(), SIGNAL(outputChanged(RROutput, int)), parent, SLOT(outputChanged(RROutput, int)));
        //connect(this, SLOT(_activate()), output(), SLOT(slotEnable()));
        //connect(this, SLOT(_deactivate()), output(), SLOT(slotDisable()));
    }
    
    void XRandROutput::parseEdid() {
        m_vendor = "";
        m_productId = -1;
        m_serialNumber = 0;
        
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
            qDebug() << "got a valid edid block...";
            /*for (int i = 0; i < size; ++i) {
                qDebug() << data[i];
            }*/
            
            /**
             * parse the 3 letter vendor code
             */
            char * vendor = new char[4];
            
            vendor[0] = EDID_VENDOR_1(data);
            vendor[1] = EDID_VENDOR_2(data);
            vendor[2] = EDID_VENDOR_3(data);
            vendor[3] = 0x00;
            m_vendor = vendor;
            
            qDebug() << "vendor code:" << m_vendor;
            
            delete vendor;
            
            /**
             * parse the 16bit product id
             */
            m_productId = EDID_PRODUCT_ID(data);
            
            qDebug() << "product id:" << m_productId;
            
            /**
             * parse the 32bit serial number
             */
            m_serialNumber = EDID_SERIAL_NUMBER(data);
            
            qDebug() << "serial number:" << m_serialNumber;
        } else {
            m_vendor = "";
            m_productId = -1;
            m_serialNumber = 0;
        }
        
        XFree(data);
    }
    
    void XRandROutput::_changed() {
        qDebug() << "XRandROutput::_changed()" << isConnected() << isActivated() << geom();
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
        saveAsPrevious();
        if (size() != previousGeom.size()) {
            emit outputResized(this, previousGeom.size(), size());
        }
        if (position() != previousGeom.topLeft()) {
            emit outputMoved(this, previousGeom.topLeft(), position());
        }
    }
    
    void XRandROutput::saveAsPrevious() {
        m_previousConnected = isConnected();
        m_previousActivated = isActivated();
        m_previousGeom = geom();
    }
    
    bool XRandROutput::_apply(QRect geom) {
        output()->proposeRect(geom);
        /*float rate = output()->refreshRate();
        QList<float> rates = output()->refreshRates(geom.size());
        if (! rates.contains(rate)) {
            output()->proposeRefreshRate(rates[0]);
        }*/
        return output()->applyProposed();
    }
    
    void XRandROutput::_revert() {
        output()->proposeOriginal();
        output()->applyProposed();
    }
    
    void XRandROutput::_deactivate() {
        output()->slotDisable();
    }
    
    RandROutput * XRandROutput::output() {
        return m_outputs->output(m_rrId);
    }
    
    QString XRandROutput::id() {
        return output()->name();
    }

    QSize XRandROutput::size() {
        return output()->rect().size();
    }
    
    QSize XRandROutput::preferredSize() {
        if (! output()->preferredMode().size().isEmpty()) {
            return output()->preferredMode().size();
        }
        return QSize(800, 600);
    }
    
    QList<QSize> XRandROutput::availableSizes() {
        QList<QSize> sizes = output()->sizes();
        /*foreach (QSize size, sizes) {
            qDebug() << "available size:" << id() << size;
        }*/
        return sizes;
    }
    
    QPoint XRandROutput::position() {
        return output()->rect().topLeft();
    }
    
    bool XRandROutput::isConnected() {
        return output()->isConnected();
    }
    
    bool XRandROutput::isActivated() {
        return output()->isActive();
    }
    
    QString XRandROutput::vendor() {
        return m_vendor;
    }
    
    int XRandROutput::productId() {
        return m_productId;
    }
    
    unsigned int XRandROutput::serialNumber() {
        return m_serialNumber;
    }
    
    RROutput XRandROutput::_id() {
        return m_rrId;
    }
    
}

