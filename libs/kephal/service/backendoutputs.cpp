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


#include "backendoutputs.h"

#include <KDebug>

namespace Kephal {

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



    BackendOutputs * BackendOutputs::s_instance = 0;

    BackendOutputs * BackendOutputs::self() {
        return s_instance;
    }

    BackendOutputs::BackendOutputs(QObject * parent)
        : Outputs(parent)
    {
        s_instance = this;
    }

    BackendOutputs::~BackendOutputs()
    {
        s_instance = 0;
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

    bool BackendOutputs::activateLayout(const QMap<Output *, QRect> & layout) {
        kDebug() << "activate layout:" << layout;

        QList<BackendOutput *> outputs = backendOutputs();
        foreach (BackendOutput * output, outputs) {
            //output->mark();
            if (! layout.contains(output)) {
                kDebug() << "deactivating output:" << output->id();
                output->deactivate();
            }
        }

        for (QMap<Output *, QRect>::const_iterator i = layout.constBegin(); i != layout.constEnd(); ++i) {
            BackendOutput * output = (BackendOutput *) i.key();
            kDebug() << "setting output" << output->id() << "to" << i.value();

            if (! output->applyGeom(i.value(), 0)) {
                kDebug() << "setting" << output->id() << "to" << i.value() << "failed!!";
                return false;
            }
        }

        return true;
    }

}

