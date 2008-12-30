/*
 *   Copyright 2008 Chani Armitage <chani@kde.org>
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

#include "appletinterface.h"

#include "qscript.h"

#include <Plasma/Plasma>
#include <Plasma/Applet>
#include <Plasma/Context>
#include <Plasma/DataEngine>

AppletInterface::AppletInterface(QScriptApplet *parent)
    :QObject(parent),
    applet(parent)
{
    connect(this, SIGNAL(releaseVisualFocus()), applet->applet(), SIGNAL(releaseVisualFocus()));
    connect(this, SIGNAL(configNeedsSaving()), applet->applet(), SIGNAL(configNeedsSaving()));
}

AppletInterface::~AppletInterface()
{
}

KConfigGroup AppletInterface::config()
{
    return applet->applet()->config();
}

Plasma::DataEngine* AppletInterface::dataEngine(const QString &name)
{
    return applet->applet()->dataEngine(name);
}

AppletInterface::FormFactor AppletInterface::formFactor()
{
    return static_cast<FormFactor>(applet->applet()->formFactor());
}

AppletInterface::Location AppletInterface::location()
{
    return static_cast<Location>(applet->applet()->location());
}

QString AppletInterface::currentActivity()
{
    return applet->applet()->context()->currentActivity();
}

AppletInterface::AspectRatioMode AppletInterface::aspectRatioMode()
{
    return static_cast<AspectRatioMode>(applet->applet()->aspectRatioMode());
}

void AppletInterface::setAspectRatioMode(AppletInterface::AspectRatioMode mode)
{
    applet->applet()->setAspectRatioMode(static_cast<Plasma::AspectRatioMode>(mode));
}

bool AppletInterface::shouldConserveResources()
{
    return applet->applet()->shouldConserveResources();
}

void AppletInterface::setFailedToLaunch(bool failed, const QString &reason)
{
    applet->setFailedToLaunch(failed, reason);
}

bool AppletInterface::isBusy()
{
    return applet->applet()->isBusy();
}

void AppletInterface::setBusy(bool busy)
{
    applet->applet()->setBusy(busy);
}

void AppletInterface::setConfigurationRequired(bool needsConfiguring, const QString &reason)
{
    applet->setConfigurationRequired(needsConfiguring, reason);
}

    //TODO actions

void AppletInterface::resize(qreal w, qreal h)
{
    applet->applet()->resize(w,h);
}

void AppletInterface::setMinimumSize(qreal w, qreal h)
{
    applet->applet()->setMinimumSize(w,h);
}

void AppletInterface::setPreferredSize(qreal w, qreal h)
{
    applet->applet()->setPreferredSize(w,h);
}


#include "appletinterface.moc"
