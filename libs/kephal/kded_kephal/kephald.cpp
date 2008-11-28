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


#include "kephald.h"
#include "config-kephal.h"

#include <QDebug>
#include <QDBusConnection>
#include <QApplication>
#include <QAbstractEventDispatcher>

#include <KConfig>
#include <KConfigGroup>

#ifdef HAS_RANDR_1_2 
#include "xrandr12/randrdisplay.h"
#include "xrandr12/randrscreen.h"
#endif

#include "outputs/desktopwidget/desktopwidgetoutputs.h"
#include "screens/configuration/configurationscreens.h"

#ifdef HAS_RANDR_1_2
#include "outputs/xrandr/xrandroutputs.h"
#endif

#include "dbus/dbusapi_screens.h"
#include "dbus/dbusapi_outputs.h"
#include "dbus/dbusapi_configurations.h"
#include "configurations/xml/xmlconfigurations.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KApplication>

K_PLUGIN_FACTORY(KephalDFactory,
                 registerPlugin<KephalD>();
    )
K_EXPORT_PLUGIN(KephalDFactory("kephal"))



using namespace Kephal;



KephalD::KephalD(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent),
    m_noXRandR(false)
{
    qDebug() << "kephald starting up";
    init();
}

KephalD::~KephalD()
{
    if (m_eventFilter) {
        delete m_eventFilter;
    }
}

void KephalD::init() {
    KConfig config("kephalrc");
    KConfigGroup general(&config, "General");
    m_noXRandR = general.readEntry("NoXRandR", false);

    m_outputs = 0;
#ifdef HAS_RANDR_1_2
    RandRDisplay * display;
    if (! m_noXRandR) {
        display = new RandRDisplay();
    }

    if ((! m_noXRandR) && display->isValid()) {
        m_outputs = new XRandROutputs(this, display);
        if (m_outputs->outputs().size() <= 1) {
            delete m_outputs;
            m_outputs = 0;
        }
    }
#endif
    if (! m_outputs) {
        new DesktopWidgetOutputs(this);

    }

    foreach (Output * output, Outputs::self()->outputs()) {
        qDebug() << "output:" << output->id() << output->geom() << output->rotation() << output->reflectX() << output->reflectY();
    }
    
    new XMLConfigurations(this);
    new ConfigurationScreens(this);
    
    foreach (Kephal::Screen * screen, Screens::self()->screens()) {
        qDebug() << "screen:" << screen->id() << screen->geom();
    }
    
    activateConfiguration();
    connect(Outputs::self(), SIGNAL(outputDisconnected(Kephal::Output *)), this, SLOT(outputDisconnected(Kephal::Output *)));
    connect(Outputs::self(), SIGNAL(outputConnected(Kephal::Output *)), this, SLOT(outputConnected(Kephal::Output *)));
    
    QDBusConnection dbus = QDBusConnection::sessionBus();
    bool result = dbus.registerService("org.kde.Kephal");
    qDebug() << "registered the service:" << result;
    
    new DBusAPIScreens(this);
    new DBusAPIOutputs(this);
    new DBusAPIConfigurations(this);
    
    if (m_outputs) {
        m_eventFilter = new X11EventFilter(m_outputs);
        kapp->installX11EventFilter(m_eventFilter);

        m_pollTimer = new QTimer(this);
        connect(m_pollTimer, SIGNAL(timeout()), this, SLOT(poll()));
        if (Configurations::self()->polling()) {
            m_pollTimer->start(10000);
        }
    } else {
        m_pollTimer = 0;
        m_eventFilter = 0;
    }
}

void KephalD::pollingActivated() {
    if (m_pollTimer && m_outputs) {
        m_pollTimer->start(10000);
    }
}

void KephalD::pollingDeactivated() {
    if (m_pollTimer && m_outputs) {
        m_pollTimer->stop();
    }
}

void KephalD::poll() {
#ifdef HAS_RANDR_1_2
    if (m_outputs) {
        m_outputs->display()->screen(0)->pollState();
    }
#endif
}

void KephalD::activateConfiguration() {
    BackendConfigurations * configs = BackendConfigurations::self();
    Configuration * config = configs->findConfiguration();
    configs->applyOutputSettings();
    if (config) {
        config->activate();
    } else {
        qDebug() << "couldnt find matching configuration!!";
    }
}

void KephalD::outputDisconnected(Output * output) {
    Q_UNUSED(output)
    activateConfiguration();
}

void KephalD::outputConnected(Output * output) {
    Q_UNUSED(output)
    activateConfiguration();
}

#ifdef Q_WS_X11 
bool X11EventFilter::x11Event(XEvent * e) {
#ifdef HAS_RANDR_1_2
    if (m_outputs && m_outputs->display()->canHandle(e)) {
        m_outputs->display()->handleEvent(e);
    }
#endif
    return false;
}
#endif


#include "kephald.moc"
