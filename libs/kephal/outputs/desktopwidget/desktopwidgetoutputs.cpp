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


#include "desktopwidgetoutputs.h"
#include "../simpleoutput.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>


namespace kephal {

    DesktopWidgetOutputs::DesktopWidgetOutputs(QObject * parent)
            : Outputs(parent)
    {
        QDesktopWidget * desktop = QApplication::desktop();
        for (int i = 0; i < desktop->numScreens(); i++) {
            QRect geom = desktop->screenGeometry(i);
            qDebug() << "adding an output" << i << "with geom: " << geom;
            
            SimpleOutput * output = new SimpleOutput(this,
                    "SCREEN-" + QString::number(i),
                    geom.size(),
                    geom.topLeft(),
                    true,
                    true);
            m_outputs.append(output);
        }
        for (int i = desktop->numScreens(); i < 4; i++) {
            qDebug() << "adding a disconnected output" << i;
            
            SimpleOutput * output = new SimpleOutput(this,
                    "SCREEN-" + QString::number(i),
                    QSize(0, 0),
                    QPoint(0, 0),
                    false,
                    false);
            m_outputs.append(output);
        }
        
        connect(desktop, SIGNAL(resized(int)), this, SLOT(screenChanged(int)));
    }
    
    DesktopWidgetOutputs::~DesktopWidgetOutputs() {
        foreach(Output * output, m_outputs) {
            delete output;
        }
        m_outputs.clear();
    }

    QList<Output *> DesktopWidgetOutputs::outputs()
    {
        QList<Output *> result;
        foreach(SimpleOutput * output, m_outputs) {
            result.append(output);
        }
        return result;
    }
    
    void DesktopWidgetOutputs::screenChanged(int screen)
    {
        QDesktopWidget * desktop = QApplication::desktop();
        for(int i = m_outputs.size() - 1; i >= desktop->numScreens(); i--) {
            SimpleOutput * output = m_outputs.last();
            if (output->isConnected()) {
                qDebug() << "disconnecting output" << i;
                output->_setConnected(false);
                emit outputDisconnected(output);
            }
        }
        
        for(int i = 0; i < desktop->numScreens(); i++) {
            if (m_outputs.size() <= i) {
                m_outputs.append(new SimpleOutput(this,
                    "SCREEN-" + QString::number(i),
                    QSize(0, 0),
                    QPoint(0, 0),
                    false,
                    false));
            }
            
            SimpleOutput * output = m_outputs[i];
            QRect geom = desktop->screenGeometry(i);
            if (! output->isConnected()) {
                output->_setConnected(true);
                output->_setActivated(true);
                output->_setPosition(geom.topLeft());
                output->_setSize(geom.size());
                
                emit outputConnected(output);
            }
            if (output->position() != geom.topLeft()) {
                QPoint oldPos = output->position();
                QPoint newPos = geom.topLeft();
                qDebug() << "output" << i << "moved" << oldPos << "->" << newPos;
                
                output->_setPosition(newPos);
                emit outputMoved(output, oldPos, newPos);
            }
            if (output->size() != geom.size()) {
                QSize oldSize = output->size();
                QSize newSize = geom.size();
                qDebug() << "output" << i << "resized" << oldSize << "->" << newSize;
                
                output->_setSize(newSize);
                emit outputResized(output, oldSize, newSize);
            }
        }
    }
    
}

