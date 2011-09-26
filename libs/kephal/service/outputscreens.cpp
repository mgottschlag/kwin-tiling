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


#include "outputscreens.h"

#include <QTimer>

#include <KDebug>

namespace Kephal {

    OutputScreens::OutputScreens(QObject * parent)
        : Screens(parent),
        m_rebuildTimer(new QTimer(this))
    {
        m_rebuildTimer->setSingleShot(true);
        connect(m_rebuildTimer, SIGNAL(timeout()), this, SLOT(rebuildTimeout()));
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

        connect(Outputs::self(), SIGNAL(outputResized(Kephal::Output*,QSize,QSize)), this, SLOT(outputResized(Kephal::Output*,QSize,QSize)));
        connect(Outputs::self(), SIGNAL(outputMoved(Kephal::Output*,QPoint,QPoint)), this, SLOT(outputMoved(Kephal::Output*,QPoint,QPoint)));
        connect(Outputs::self(), SIGNAL(outputActivated(Kephal::Output*)), this, SLOT(outputActivated(Kephal::Output*)));
        connect(Outputs::self(), SIGNAL(outputDeactivated(Kephal::Output*)), this, SLOT(outputDeactivated(Kephal::Output*)));
    }

    void OutputScreens::outputActivated(Kephal::Output * o) {
        Q_UNUSED(o)
        kDebug();
        triggerRebuildScreens();
    }

    void OutputScreens::outputDeactivated(Kephal::Output * o) {
        Q_UNUSED(o)
        kDebug();
        triggerRebuildScreens();
    }

    void OutputScreens::outputMoved(Kephal::Output * o, QPoint oldPosition, QPoint newPosition) {
        Q_UNUSED(o)
        Q_UNUSED(oldPosition)
        Q_UNUSED(newPosition)
        kDebug();
        triggerRebuildScreens();
    }

    void OutputScreens::outputResized(Kephal::Output * o, QSize oldSize, QSize newSize) {
        Q_UNUSED(o)
        Q_UNUSED(oldSize)
        Q_UNUSED(newSize)
        kDebug();
        triggerRebuildScreens();
    }

    void OutputScreens::buildScreens() {
        foreach (Output * output, Outputs::self()->outputs()) {
            // for each connected and active output,
            if (! output->isConnected() || ! output->isActivated()) {
                continue;
            }

            // look for any screen which intersects this output, and add it to the screen
            bool found = false;
            foreach (OutputScreen * screen, m_screens) {
                if (screen->geom().intersects(output->geom())) {
                    screen->add(output);
                    found = true;
                    break;
                }
            }
            // if the output did not intersect, add it to the first empty screen found
            if (! found) {
                foreach (OutputScreen * screen, m_screens) {
                    if (screen->outputs().empty()) {
                        screen->add(output);
                        found = true;
                        break;
                    }
                }
            }
            // if no empty screen found, generate one and add this output to it.
            if (! found) {
                OutputScreen * screen = new OutputScreen(this);
                screen->_setId(findId());
                screen->add(output);
                m_screens.insert(screen->id(), screen);
            }
        }

        // remove any remaining empty screens 
        for (QMap<int, OutputScreen *>::iterator i = m_screens.begin(); i != m_screens.end();) {
            if (i.value()->outputs().empty()) {
                i = m_screens.erase(i);
            } else {
                ++i;
            }
        }

        // remove any intersecting Screens and add removed Screens' Outputs to the intersected Screen
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

        // I think this tries to renumber m_screens from 0, removing any gaps due to removed
        // intersecting screens
        for (int i = 0; i < m_screens.size(); ++i) {
            if (! m_screens.contains(i)) {
                // get the lowest numbered screen in m_screens
                int min = -1;
                for (QMap<int, OutputScreen *>::iterator it = m_screens.begin(); it != m_screens.end(); ++it) {
                    if ((min == -1) || (it.key() < min)) {
                        min = it.key();
                    }
                }
                OutputScreen * screen = m_screens.take(min);
                screen->_setId(i);
                m_screens.insert(i, screen);
            }
        }
    }

    void OutputScreens::triggerRebuildScreens() {
        kDebug();
        m_rebuildTimer->start(200);
    }

    void OutputScreens::rebuildTimeout() {
        rebuildScreens();
    }

    void OutputScreens::rebuildScreens() {
        kDebug();

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
                kDebug() << "emitted screenAdded " << i.key();
            } else if (geoms[i.key()] != i.value()->geom()) {
                if (geoms[i.key()].topLeft() != i.value()->geom().topLeft()) {
                    emit screenMoved(i.value(), geoms[i.key()].topLeft(), i.value()->geom().topLeft());
                    kDebug() << "emitted screenMoved " << i.key() << " - old " << geoms[i.key()] << " - new " << i.value()->geom();
                }
                if (geoms[i.key()].size() != i.value()->geom().size()) {
                    emit screenResized(i.value(), geoms[i.key()].size(), i.value()->geom().size());
                    kDebug() << "emitted screenResized " << i.key() << " - old " << geoms[i.key()] << " - new " << i.value()->geom();
                }
            }
        }

        for (QMap<int, QRect>::const_iterator i = geoms.constBegin(); i != geoms.constEnd(); ++i) {
            if (! m_screens.contains(i.key())) {
                emit screenRemoved(i.key());
                kDebug() << "emitted screenRemoved " << i.key();
            }
        }
    }

    void OutputScreens::prepareScreens(QMap<int, OutputScreen *> & screens) {
        Q_UNUSED(screens)
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

#ifndef NO_KDE
#include "outputscreens.moc"
#endif
