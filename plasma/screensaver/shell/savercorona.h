/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Chani Armitage <chanika@gmail.com>
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

#ifndef SAVERCORONA_H
#define SAVERCORONA_H

#include <QtGui/QGraphicsScene>

#include <Plasma/Corona>

class QDeclarativeEngine;

/**
 * @short A Corona for the screensaver
 */
class SaverCorona : public Plasma::Corona
{
    Q_OBJECT

public:
    explicit SaverCorona(QObject * parent = 0);

    /**
     * Loads the default (system wide) layout for this user
     **/
    void loadDefaultLayout();

    virtual int numScreens() const;
    virtual QRect screenGeometry(int id) const;

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event);

private Q_SLOTS:
    void updateActions(Plasma::ImmutabilityType immutability);
    void toggleLock();
    void unlockDesktop();
    void numScreensUpdated(int newCount);
    void greeterAccepted();
    void greeterCanceled();

private:
    enum UnlockMode {
        AppletLock,
        ScreenLock
    };
    void init();
    void createGreeter();
    void capsLocked();

    int m_numScreens;
    QDeclarativeEngine *m_engine;
    QGraphicsObject *m_greeterItem;
    UnlockMode m_mode;
    bool m_capsLocked;
};

#endif


