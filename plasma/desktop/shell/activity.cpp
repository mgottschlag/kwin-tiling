/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
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

#include <QPixmap>
#include <QString>
#include <QSize>

#include <KIcon>

#include <Plasma/Containment>

#include "activity.h"

Activity::Activity(const QString &id, QObject *parent)
    :QObject(parent),
    m_id(id)
{
    m_name = id; //TODO get it from libactivities

    Plasma::Corona *corona = PlasmaApp::self()->corona();

    //find your containments
    foreach (Plasma::Containment *cont, corona->containments()) {
        if ((cont->containmentType() == Plasma::Containment::DesktopContainment ||
            cont->containmentType() == Plasma::Containment::CustomContainment) &&
                !corona->offscreenWidgets().contains(cont) && cont->activity() == id) {
            m_containments << cont;
            break;
        }
    }
    kDebug() << m_containments.size();

}

Activity::~Activity()
{
}


QString Activity::id()
{
    return m_id;
}

QString Activity::name()
{
    return m_name;
}

QPixmap Activity::thumbnail(const QSize &size)
{
    //TODO
    return KIcon("plasma").pixmap(size);
}

bool Activity::isActive()
{
    //TODO
    return false;
}

bool isRunning()
{
    return ! m_containments.isEmpty();
}

void Activity::destroy()
{
    //TODO
    //-kill the activity in nepomuk
    //-destroy all our containments
}

void Activity::activate()
{
    if (m_containments.isEmpty()) {
        //TODO load them
        return;
    }
    //figure out where we are
    int currentScreen = Kephal::ScreenUtils::screenId(QCursor::pos());
    int currentDesktop = -1;
    if (AppSettings::perVirtualDesktopViews()) {
        currentDesktop = KWindowSystem::currentDesktop()-1;
    }
    //and bring the containment to where we are
    m_containments.first()->setScreen(currentScreen, currentDesktop);
    //TODO handle other screens

    //TODO libactivities stuff
}

void Activity::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    emit nameChanged(name);
    //TODO libactivities stuff
    //propogate change to ctmts
}


// vim: sw=4 sts=4 et tw=100
