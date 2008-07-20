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

#include "xrandr12/randrscreen.h"
#include "xrandr12/randroutput.h"


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
            m_outputs.append(o);
        }
    }
    
    RandROutput * XRandROutputs::output(RROutput rrId) {
        return m_display->screen(0)->outputs()[rrId];
    }
    
    
    
    XRandROutput::XRandROutput(XRandROutputs * parent, RROutput rrId)
            : Output(parent)
    {
        m_outputs = parent;
        m_rrId = rrId;
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
    
    void XRandROutput::setSize(QSize size) {
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
    
}

