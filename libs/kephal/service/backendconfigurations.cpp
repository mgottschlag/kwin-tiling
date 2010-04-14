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


#include "backendconfigurations.h"

#include "outputs.h"
#include "backend.h"

#include <KDebug>


namespace Kephal {

    BackendConfigurations * BackendConfigurations::self() {
        return BackendConfigurations::s_instance;
    }

    BackendConfigurations::BackendConfigurations(QObject * parent)
        : Configurations(parent)/*,
        m_status(new StatusMessage(this))*/
    {
        BackendConfigurations::s_instance = this;
    }

    BackendConfigurations::~BackendConfigurations()
    {
        BackendConfigurations::s_instance = 0;
    }

    BackendConfigurations * BackendConfigurations::s_instance = 0;

    BackendConfiguration * BackendConfigurations::activeBackendConfiguration() {
        return (BackendConfiguration *) activeConfiguration();
    }



    BackendConfiguration::BackendConfiguration(QObject * parent)
        : Configuration(parent)
    {
    }

    QMap<int, QRect> BackendConfiguration::realLayout() {
        QMap<Output *, int> outputScreens;
        foreach (Output * output, Outputs::self()->outputs()) {
            int screen = Configurations::self()->screen(output);
            outputScreens.insert(output, screen);
        }
        return realLayout(outputScreens);
    }

    QMap<int, QRect> BackendConfiguration::realLayout(const QMap<Output *, int> & outputScreens) {
        QMap<int, QPoint> simpleLayout = layout();
        return realLayout(simpleLayout, outputScreens);
    }

    QMap<int, QRect> BackendConfiguration::realLayout(const QMap<int, QPoint> & sLayout, const QMap<Output *, int> & outputScreens) {
        QMap<Output *, QSize> outputSizes;
        foreach (Output * output, outputScreens.keys()) {
            outputSizes.insert(output, output->isActivated() ? output->size() : output->preferredSize());
        }
        return realLayout(sLayout, outputScreens, outputSizes);
    }

    QMap<int, QRect> BackendConfiguration::realLayout(const QMap<int, QPoint> & sLayout, const QMap<Output *, int> & outputScreens, const QMap<Output *, QSize> & outputSizes) {
        //kDebug() << "calculating real layout for:" << sLayout << outputScreens;

        QMap<int, QRect> screens;
        QMap<int, QPoint> simpleLayout = sLayout;

        QMap<int, QSize> screenSizes;
        foreach (int screen, simpleLayout.keys()) {
            screenSizes.insert(screen, QSize());
        }

        foreach (Output * output, outputScreens.keys()) {
            if (outputScreens[output] < 0) {
                continue;
            }

            if (! screenSizes.contains(outputScreens[output])) {
                INVALID_CONFIGURATION("outputs and configuration don't match");
                return screens;
            }
            screenSizes[outputScreens[output]] = screenSizes[outputScreens[output]].expandedTo(outputSizes[output]);
        }

        int begin = simpleLayout.begin().key();
        screens.insert(begin, QRect(QPoint(0, 0), screenSizes[begin]));
        simpleToReal(simpleLayout, screenSizes, begin, screens);
        Configurations::translateOrigin(screens);

        for (QMap<int, QRect>::const_iterator i = screens.constBegin(); i != screens.constEnd(); ++i) {
            for (QMap<int, QRect>::const_iterator j = (i + 1); j != screens.constEnd(); ++j) {
                if (i.value().intersects(j.value())) {
                    INVALID_CONFIGURATION("overlapping screens");
                    screens.clear();
                    return screens;
                }
            }
        }

        return screens;
    }

    void BackendConfiguration::simpleToReal(QMap<int, QPoint> & simpleLayout, const QMap<int, QSize> & screenSizes, int index, QMap<int, QRect> & screens) const {
        QPoint pos = simpleLayout.take(index);

        // to the right
        QPoint nextPos(pos.x() + 1, pos.y());
        int nextIndex = simpleLayout.key(nextPos, -1);
        if (nextIndex >= 0) {
            screens.insert(nextIndex, QRect(screens[index].topRight() + QPoint(1, 0), screenSizes[nextIndex]));
            simpleToReal(simpleLayout, screenSizes, nextIndex, screens);
        }

        // to the left
        nextPos = QPoint(pos.x() - 1, pos.y());
        nextIndex = simpleLayout.key(nextPos, -1);
        if (nextIndex >= 0) {
            QSize screenSize = screenSizes[nextIndex];
            screens.insert(nextIndex, QRect(screens[index].topLeft() - QPoint(screenSize.width(), 0), screenSize));
            simpleToReal(simpleLayout, screenSizes, nextIndex, screens);
        }

        // to the bottom
        nextPos = QPoint(pos.x(), pos.y() + 1);
        nextIndex = simpleLayout.key(nextPos, -1);
        if (nextIndex >= 0) {
            screens.insert(nextIndex, QRect(screens[index].bottomLeft() + QPoint(0, 1), screenSizes[nextIndex]));
            simpleToReal(simpleLayout, screenSizes, nextIndex, screens);
        }

        // to the top
        nextPos = QPoint(pos.x(), pos.y() - 1);
        nextIndex = simpleLayout.key(nextPos, -1);
        if (nextIndex >= 0) {
            QSize screenSize = screenSizes[nextIndex];
            screens.insert(nextIndex, QRect(screens[index].topLeft() - QPoint(0, screenSize.height()), screenSize));
            simpleToReal(simpleLayout, screenSizes, nextIndex, screens);
        }
    }

    QMap<int, QPoint> BackendConfiguration::cloneLayout(int screen) {
        QSet<QPoint> positions = clonePositions(screen);
        QMap<int, QPoint> layout;
        int i = 0;
        foreach (const QPoint& p, positions) {
            layout.insert(i, p);
            ++i;
        }

        Configurations::translateOrigin(layout);
        return layout;
    }

    QSet<QPoint> BackendConfiguration::clonePositions(int screen) {
        QList<QSet<QPoint> > partitions = partition(screen);
        if (partitions.size() == 1) {
            return partitions[0];
        }
        return QSet<QPoint>();
    }

    QSet<QPoint> BackendConfiguration::positions() {
        QSet<QPoint> result;
        foreach (const QPoint& p, layout()) {
            result << p;
        }
        return result;
    }

    QSet<QPoint> BackendConfiguration::possiblePositions(int screen) const {
        QList<QSet<QPoint> > partitions = partition(screen);
        QSet<QPoint> result = border(partitions[0]);
        foreach (const QSet<QPoint> &partition, partitions) {
            result.intersect(border(partition));
        }
        return result;
    }

    /*WILL: used by clonePositions and possiblePositions*/
    QList<QSet<QPoint> > BackendConfiguration::partition(int screen) const {
        QHash<QPoint, QSet<QPoint> * > partitions;
        QMap<int, QPoint> layout = this->layout();
        bool exclude = layout.contains(screen);
        QPoint excludePoint;
        if (exclude) {
            excludePoint = layout[screen];
        }
        foreach (const QPoint& p, layout) {
            if (exclude && (p == excludePoint)) {
                continue;
            }
            partitions.insert(p, new QSet<QPoint>());
            partitions[p]->insert(p);
        }

        foreach (const QPoint& p, layout) {
            if (exclude && (p == excludePoint)) {
                continue;
            }
            QList<QPoint> connected;
            if (partitions.contains(p + QPoint(1, 0))) {
                connected.append(p + QPoint(1, 0));
            }
            if (partitions.contains(p + QPoint(0, 1))) {
                connected.append(p + QPoint(0, 1));
            }
            foreach (const QPoint& c, connected) {
                if (partitions[p] == partitions[c]) {
                    continue;
                }
                partitions[p]->unite(* (partitions[c]));
                delete partitions[c];
                partitions[c] = partitions[p];
            }
        }

        QSet<QSet<QPoint> * > unique;
        foreach (QSet<QPoint> * partition, partitions) {
            unique.insert(partition);
        }

        QList<QSet<QPoint> > result;
        foreach (QSet<QPoint> * partition, unique) {
            result.append(* partition);
            delete partition;
        }

        return result;
    }

    QSet<QPoint> BackendConfiguration::border(QSet<QPoint> screens) const {
        QSet<QPoint> result;
        QList<QPoint> borders;
        borders << QPoint(1, 0) << QPoint(0, 1) << QPoint(-1, 0) << QPoint(0, -1);
        foreach (const QPoint& p, screens) {
            foreach (const QPoint& border, borders) {
                if (! screens.contains(p + border)) {
                    result.insert(p + border);
                }
            }
        }

        return result;
    }

}

#ifndef NO_KDE
#include "backendconfigurations.moc"
#endif

