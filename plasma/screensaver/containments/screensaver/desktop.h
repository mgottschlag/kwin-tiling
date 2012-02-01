/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2008 by Chani Armitage <chanika@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
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

#ifndef PLASMA_SAVERDESKTOP_H
#define PLASMA_SAVERDESKTOP_H

#include <QList>
#include <QObject>

#include <plasma/containment.h>

class QAction;

/**
 * SaverDesktop
 *
 * This is a minimal containment for plasma-overlay.
 */

class SaverDesktop : public Plasma::Containment
{
    Q_OBJECT

public:
    SaverDesktop(QObject *parent, const QVariantList &args);
    ~SaverDesktop();
    void init();

private slots:
    void newApplet(Plasma::Applet *applet, const QPointF &pos);

private:
    QAction *m_lockDesktopAction;
    QAction *m_appletBrowserAction;
};

#endif
