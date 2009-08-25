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

int Containment::screen() const
{
    if (!m_containment) {
        return -1;
    }

    return m_containment->screen();
}

int Containment::desktop() const
{
    if (!m_containment) {
        return -1;
    }

    return m_containment->desktop();
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

    return m_containment->name();
}

QString Containment::type() const
{
    if (!m_containment) {
        return QString();
    }

    return m_containment->pluginName();
}

void Containment::setName(const QString &name)
{
    if (!m_containment) {
        return;
    }

    return m_containment->setActivity(name);
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

