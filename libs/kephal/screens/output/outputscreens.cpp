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

#include "outputscreens.h"


namespace kephal {

    OutputScreens::OutputScreens(QObject * parent)
        : Screens(parent)
    {
        init();
    }
    
    QList<Screen *> OutputScreens::screens() {
        QList<Screen *> result;
        foreach(OutputScreen * screen, m_screens) {
            result.append(screen);
        }
        return result;
    }
    
    int OutputScreens::findId() {
        for (int i = 0; i < m_screens.size(); ++i) {
            if (! m_screens.contains(i)) {
                return i;
            }
        }
        return m_screens.size();
    }
    
    void OutputScreens::init() {
        buildScreens();
        
        connect(Outputs::instance(), SIGNAL(outputResized(kephal::Output *, QSize, QSize)), this, SLOT(outputResized(kephal::Output *, QSize, QSize)));
    }
    
    void OutputScreens::outputActivated(kephal::Output * o) {
        Q_UNUSED(o)
        
        rebuildScreens();
    }

    void OutputScreens::outputDeactivated(kephal::Output * o) {
        Q_UNUSED(o)
        
        rebuildScreens();
    }

    void OutputScreens::outputMoved(kephal::Output * o, QPoint oldPosition, QPoint newPosition) {
        Q_UNUSED(o)
        Q_UNUSED(oldPosition)
        Q_UNUSED(newPosition)
        
        rebuildScreens();
    }

    void OutputScreens::outputResized(kephal::Output * o, QSize oldSize, QSize newSize) {
        Q_UNUSED(o)
        Q_UNUSED(oldSize)
        Q_UNUSED(newSize)
        
        rebuildScreens();
    }
    
    void OutputScreens::buildScreens() {
        foreach (Output * output, Outputs::instance()->outputs()) {
            if (! output->isConnected() || ! output->isActivated()) {
                continue;
            }
            
            bool found = false;
            foreach (OutputScreen * screen, m_screens) {
                if (screen->geom().intersects(output->geom())) {
                    screen->add(output);
                    found = true;
                    break;
                }
            }
            if (! found) {
                foreach (OutputScreen * screen, m_screens) {
                    if (screen->outputs().empty()) {
                        screen->add(output);
                        found = true;
                        break;
                    }
                }
            }
            if (! found) {
                OutputScreen * screen = new OutputScreen(this);
                screen->_setId(findId());
                screen->add(output);
                m_screens.insert(screen->id(), screen);
            }
        }
        
        for (QMap<int, OutputScreen *>::iterator i = m_screens.begin(); i != m_screens.end();) {
            if (i.value()->outputs().empty()) {
                i = m_screens.erase(i);
            } else {
                ++i;
            }
        }
        
        bool changed;
        do {
            changed = false;
            for (QMap<int, OutputScreen *>::iterator i = m_screens.begin(); i != m_screens.end(); ++i) {
                bool deleted = false;
                for (QMap<int, OutputScreen *>::iterator j = i + 1; j != m_screens.end(); ++j) {
                    if (i.value()->geom().intersects(j.value()->geom())) {
                        OutputScreen * to = i.value();
                        OutputScreen * from = j.value();
                        
                        foreach (Output * output, from->outputs()) {
                            to->add(output);
                        }
                        
                        changed = true;
                        deleted = true;
                        m_screens.erase(j);
                        delete from;
                        break;
                    }
                }
                if (deleted) {
                    break;
                }
            }
        } while (changed);
    }
    
    void OutputScreens::rebuildScreens() {
        qDebug() << "OutputScreens::rebuildScreens()";
        
        QMap<int, QRect> geoms;
        for (QMap<int, OutputScreen *>::const_iterator i = m_screens.constBegin(); i != m_screens.constEnd(); ++i) {
            geoms.insert(i.key(), i.value()->geom());
            
            i.value()->clearOutputs();
        }
        
        prepareScreens(m_screens);
        buildScreens();
        
        for (QMap<int, OutputScreen *>::const_iterator i = m_screens.constBegin(); i != m_screens.constEnd(); ++i) {
            if (! geoms.contains(i.key())) {
                emit screenAdded(i.value());
            } else if (geoms[i.key()] != i.value()->geom()) {
                if (geoms[i.key()].topLeft() != i.value()->geom().topLeft()) {
                    emit screenMoved(i.value(), geoms[i.key()].topLeft(), i.value()->geom().topLeft());
                }
                if (geoms[i.key()].size() != i.value()->geom().size()) {
                    emit screenResized(i.value(), geoms[i.key()].size(), i.value()->geom().size());
                }
            }
        }
        
        for (QMap<int, QRect>::const_iterator i = geoms.constBegin(); i != geoms.constEnd(); ++i) {
            if (! m_screens.contains(i.key())) {
                emit screenRemoved(i.key());
            }
        }
    }
    
    void OutputScreens::prepareScreens(QMap<int, OutputScreen *> & screens) {
    }
    
    
    
    OutputScreen::OutputScreen(QObject * parent)
        : SimpleScreen(parent)
    {
    }
    
    void OutputScreen::add(Output * output) {
        m_outputs.append(output);
        
        QRect geom = this->geom();
        if (geom.isEmpty()) {
            geom = output->geom();
        } else {
            geom = geom.unite(output->geom());
        }
        
        _setSize(geom.size());
        _setPosition(geom.topLeft());
    }
    
    QList<Output *> OutputScreen::outputs() {
        return m_outputs;
    }
    
    void OutputScreen::remove(Output * output) {
        m_outputs.removeAll(output);
    }
    
    void OutputScreen::clearOutputs() {
        m_outputs.clear();
        _setSize(QSize(0, 0));
        _setPosition(QPoint(0, 0));
    }
    
}

