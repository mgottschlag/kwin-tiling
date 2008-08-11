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
    
    void XRandROutputs::activateLayout(QMap<Output *, QRect> layout) {
        qDebug() << "activate layout:" << layout;
        
        foreach (XRandROutput * output, m_outputs) {
            if ((! layout.contains(output)) && output->isActivated()) {
                output->_deactivate();
            }
        }

        /*foreach (XRandROutput * output, m_outputs) {
            if (layout.contains(output) && ! (output->isActivated())) {
                qDebug() << "enabling output" << output->id();
                output->_activate();
            }
        }*/
            
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
    
    /*bool XRandROutputs::relayout(XRandROutput * output, QMap<Position, Output *> anchors, QSize size)
    {
        QMap<QString, QRect> layout = relayout(output->id(), anchors, size, this->layout());
        
        if (layout.isEmpty()) {
            return false;
        } else {
            qDebug() << "relayout to:" << layout;
            return true;
        }
    }
    
    bool XRandROutputs::checkLayout(XRandROutput * output, QMap<Position, Output *> anchors, QSize size)
    {
        return checkLayout(output->id(), anchors, size, layout());
    }
    
    QMap<QString, QRect> XRandROutputs::relayout(QString output, QMap<Position, Output *> anchors, QSize size, QMap<QString, QRect> layout)
    {
        QMap<QString, QRect> result;
        
        for (QMap<QString, QRect>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            if (i.key() == output) {
                continue;
            }
            
            result.insert(i.key(), i.value());
            abc
        }
        
        return result;
    }
    
    bool XRandROutputs::checkLayout(QString output, QMap<Position, Output *> anchors, QSize size, QMap<QString, QRect> layout)
    {
    }
    
    bool XRandROutputs::checkLayout(QMap<QString, QRect> layout)
    {
        QMap<QString, QSet<QString> > anchored;
        for (QMap<QString, QRect>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            for (QMap<QString, QRect>::const_iterator j = layout.constBegin(); j != layout.constEnd(); ++j) {
                if (i.key() == j.key()) {
                    continue;
                }
                
                QString id1 = i.key();
                QString id2 = j.key();
                QRect g1 = i.value();
                QRect g2 = j.value();
                
                if ((g1.topLeft() != g2.topLeft()) && g1.intersects(g2)) {
                    return false;
                } else if ((g1.topLeft == g2.topLeft)
                        || (g1.topLeft == g2.topRight)
                        || (g1.topRight == g2.topLeft)
                        || (g1.topLeft == g2.bottomLeft)
                        || (g1.bottomLeft == g2.topLeft)) {
                    if ((! anchored.contains(id1)) || (! anchored[id1].contains(id2))) {
                        QSet<QString> g1Set;
                        QSet<QString> g2Set;
                        if (! anchored.contains(id1)) {
                            g1Set.insert(id1);
                            anchored.insert(id1, g1Set);
                        } else {
                            g1Set = anchored[id1];
                        }
                        
                        if (! anchored.contains(id2)) {
                            g2Set.insert(id2);
                            anchored.insert(id2, g2Set);
                        } else {
                            g2Set = anchored[id2];
                        }
                        
                        g1Set.unite(g2Set);
                        foreach (QString id, g1Set) {
                            if (id == id1) {
                                continue;
                            }
                            
                            anchored.insert(id, g1Set);
                        }
                    }
                }
            }
        }
        
        if (anchored.beginConst().value().size() == layout.size()) {
            return true;
        }
        
        return true;
    }
    
    QMap<QString, QRect> XRandROutputs::layout()
    {
        QMap<QString, QRect> layout;
        for (QMap<QString, XRandROutput *>::const_iterator i = m_outputs.constBegin(); i != m_outputs.constEnd(); ++i) {
            layout.insert(i.key(), i.value()->geom());
        }
    }
    
    /*QMap<Position, QString> XRandROutputs::anchors(QString output, QMap<QString, QRect> layout)
    {
        QMap<Position, Output *> anchors;
        QRect geom = layout[output];
        for (QMap<QString, QRect>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            if (i.key() == output) {
                continue;
            }
            
            if (geom.topRight() == i.value().topLeft()) {
                anchors.insert(LeftOf, i.key());
            } else if (geom.topLeft() == i.value().topRight()) {
                anchors.insert(RightOf, i.key());
            } else if (geom.topLeft() == i.value().bottomLeft()) {
                anchors.insert(BottomOf, i.key());
            } else if (geom.bottomLeft() == i.value().topLeft()) {
                anchors.insert(TopOf, i.key());
            } else if (geom.topLeft() == i.value().topLeft()) {
                anchors.insert(SameAs, i.key());
            }
        }
    }
    
    QMap<Position, QString> XRandROutputs::anchors(QString output)
    {
        return anchors(output, layout());
    }*/
    
    
    
    XRandROutput::XRandROutput(XRandROutputs * parent, RROutput rrId)
            : Output(parent)
    {
        m_outputs = parent;
        m_rrId = rrId;
        
        parseEdid();
        
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
    
    /*void XRandROutput::_activate() {
        //qDebug() << "rates:" << output()->refreshRate() << "-" << output()->refreshRates(size());
        output()->slotEnable();
    }*/
    
    RandROutput * XRandROutput::output() {
        return m_outputs->output(m_rrId);
    }
    
    QString XRandROutput::id() {
        return output()->name();
    }

    QSize XRandROutput::size() {
        return output()->rect().size();
    }
    
    QList<QSize> XRandROutput::availableSizes() {
        QList<QSize> sizes = output()->sizes();
        /*foreach (QSize size, sizes) {
            qDebug() << "available size:" << id() << size;
        }*/
        return sizes;
    }
    
    void XRandROutput::setSize(QSize size) {
        qDebug() << "XRandROutput::setSize() called:" << size;
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
    
    void XRandROutput::setPosition(QMap<Position, Output *> anchors) {
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
    
}

