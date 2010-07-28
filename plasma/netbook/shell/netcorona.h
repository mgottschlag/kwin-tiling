/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef NETVCORONA_H
#define NETVCORONA_H

#include <QtGui/QGraphicsScene>

#include <Plasma/Corona>

namespace Plasma
{
    class Applet;
} // namespace Plasma

/**
 * @short A Corona with desktop-y considerations
 */
class NetCorona : public Plasma::Corona
{
    Q_OBJECT

public:
    NetCorona(QObject * parent);

    /**
     * Loads the default (system wide) layout for this user
     **/
    void loadDefaultLayout();

    Plasma::Containment *findFreeContainment() const;

    bool loadDefaultLayoutScripts();
    void processUpdateScripts();

    virtual int numScreens() const;
    virtual QRect screenGeometry(int id) const;
    virtual QRegion availableScreenRegion(int id) const;

protected Q_SLOTS:
    void screenResized(int);
    void evaluateScripts(const QStringList &scripts);
    void printScriptError(const QString &error);
    void printScriptMessage(const QString &error);
    void containmentAdded(Plasma::Containment *cont);
    void addPage();

private:
    void init();
    Plasma::Applet *loadDefaultApplet(const QString &pluginName, Plasma::Containment *c);

    int m_numScreens;
};

#endif


