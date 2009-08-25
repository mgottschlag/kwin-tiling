/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#include "containment.h"

#include <Plasma/Containment>

Containment::Containment(Plasma::Containment *containment, QObject *parent)
    : QObject(parent),
      m_containment(containment)
{
}

Containment::~Containment()
{
}

QString Containment::location() const
{
    if (!m_containment) {
        return "floating";
    }

    switch (m_containment->location()) {
        case Plasma::Floating:
            return "floating";
            break;
        case Plasma::Desktop:
            return "desktop";
            break;
        case Plasma::FullScreen:
            return "fullscreen";
            break;
        case Plasma::TopEdge:
            return "top";
            break;
        case Plasma::BottomEdge:
            return "bottom";
            break;
        case Plasma::LeftEdge:
            return "left";
            break;
        case Plasma::RightEdge:
            return "right";
            break;
    }

    return "floating";
}

void Containment::setLocation(const QString &location)
{
    if (!m_containment) {
        return;
    }

    const QString lower = location.toLower();
    if (location == "floating") {
        m_containment->setLocation(Plasma::Floating);
    } else if (location == "desktop") {
        m_containment->setLocation(Plasma::Desktop);
    } else if (location == "fullscreen") {
        m_containment->setLocation(Plasma::FullScreen);
    } else if (location == "top") {
        m_containment->setLocation(Plasma::TopEdge);
    } else if (location == "bottom") {
        m_containment->setLocation(Plasma::BottomEdge);
    } else if (location == "left") {
        m_containment->setLocation(Plasma::LeftEdge);
    } else if (location == "right") {
        m_containment->setLocation(Plasma::RightEdge);
    }
}

int Containment::screen() const
{
    if (!m_containment) {
        return -1;
    }

    return m_containment->screen();
}

void Containment::setScreen(int screen)
{
    if (m_containment) {
        m_containment->setScreen(screen);
    }
}

int Containment::desktop() const
{
    if (!m_containment) {
        return -1;
    }

    return m_containment->desktop();
}

void Containment::setDesktop(int desktop)
{
    if (m_containment) {
        m_containment->setScreen(m_containment->screen(), desktop);
    }
}

QString Containment::formFactor() const
{
    if (!m_containment) {
        return "Planar";
    }

    switch (m_containment->formFactor()) {
        case Plasma::Planar:
            return "planar";
            break;
        case Plasma::MediaCenter:
            return "mediacenter";
            break;
        case Plasma::Horizontal:
            return "horizontal";
            break;
        case Plasma::Vertical:
            return "vertical";
            break;
    }

    return "Planar";
}

QList<int> Containment::widgetIds() const
{
    //FIXME: the ints could overflow since Applet::id() returns a uint,
    //       however QScript deals with QList<uint> very, very poory
    QList<int> w;

    if (m_containment) {
        foreach (const Plasma::Applet *applet, m_containment->applets()) {
            w.append(applet->id());
        }
    }

    return w;
}

uint Containment::id() const
{
    if (!m_containment) {
        return 0;
    }

    return m_containment->id();
}

QString Containment::name() const
{
    if (!m_containment) {
        return QString();
    }

    return m_containment->activity();
}

void Containment::setName(const QString &name)
{
    if (m_containment) {
        m_containment->setActivity(name);
    }
}

QString Containment::type() const
{
    if (!m_containment) {
        return QString();
    }

    return m_containment->pluginName();
}

Widget *Containment::addWidget(const QString &name)
{
    if (!m_containment) {
        return 0;
    }

    return 0; // FIXME
}

void Containment::remove()
{
    m_containment->destroy();
}

#include "containment.moc"

