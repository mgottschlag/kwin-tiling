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

#include "backendoutputs.h"


namespace kephal {

    BackendOutput::BackendOutput(QObject * parent)
        : Output(parent)
    {
    }
    
    void BackendOutput::mark() {
        m_markedActive = isActivated();
        if (m_markedActive) {
            m_markedGeom = geom();
            m_markedRate = rate();
            m_markedRotation = rotation();
            m_markedReflectX = reflectX();
            m_markedReflectY = reflectY();
        }
    }
    
    void BackendOutput::revert() {
        if (m_markedActive) {
            applyGeom(m_markedGeom, m_markedRate);
            applyOrientation(m_markedRotation, m_markedReflectX, m_markedReflectY);
        } else {
            deactivate();
        }
    }
    
    
    
    BackendOutputs * BackendOutputs::m_instance = 0;
    
    BackendOutputs * BackendOutputs::instance() {
        return m_instance;
    }
    
    BackendOutputs::BackendOutputs(QObject * parent)
        : Outputs(parent)
    {
        m_instance = this;
    }
    
    QList<BackendOutput *> BackendOutputs::backendOutputs() {
        QList<BackendOutput *> result;
        foreach (Output * output, outputs()) {
            result << (BackendOutput *) output;
        }
        return result;
    }
    
    BackendOutput * BackendOutputs::backendOutput(const QString & id) {
        foreach (BackendOutput * output, backendOutputs()) {
            if (output->id() == id) {
                return output;
            }
        }
        return 0;
    }
    
    void BackendOutputs::activateLayout(const QMap<Output *, QRect> & layout) {
        qDebug() << "activate layout:" << layout;
        
        QList<BackendOutput *> outputs = backendOutputs();
        foreach (BackendOutput * output, outputs) {
            output->mark();
            if (! layout.contains(output)) {
                qDebug() << "deactivating output:" << output->id();
                output->deactivate();
            }
        }

        for (QMap<Output *, QRect>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            BackendOutput * output = (BackendOutput *) i.key();
            qDebug() << "setting output" << output->id() << "to" << i.value();
            
            if (! output->applyGeom(i.value(), 0)) {
                qDebug() << "setting" << output->id() << "to" << i.value() << "failed!!";
                for (--i; i != layout.constBegin(); --i) {
                    output = (BackendOutput *) i.key();
                    qDebug() << "trying to revert output" << output->id();
                    output->revert();
                }
                break;
            }
        }
    }
    
}
