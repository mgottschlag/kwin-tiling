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
#include <QStyleOptionGraphicsItem>

#include <plasma/containment.h>
//#include <plasma/animator.h>
//#include <plasma/widgets/widget.h>

class QAction;
class QDBusMessage;
class QDBusError;

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

    QList<QAction*> contextualActions();

    /**
     * Paints a default background colour. Nothing fancy, but that's what plugins
     * are for. Reimplemented from Plasma::Containment;
     */
    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect& contentsRect);

signals:
    void locked();
    void unlocked();

public slots:
    void toggleLock();
    void unlock(QDBusMessage reply);
    void dbusError(QDBusError error);

private:
    QAction *m_lockDesktopAction;
    QAction *m_appletBrowserAction;
};

#endif
