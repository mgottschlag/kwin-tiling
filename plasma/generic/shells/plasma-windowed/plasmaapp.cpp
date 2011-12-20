/*
 *   Copyright 2006-2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "plasmaapp.h"

#include <unistd.h>

#include <QPixmapCache>
#include <QtDBus/QtDBus>

#include <KCrash>
#include <KColorUtils>
#include <KDebug>
#include <KCmdLineArgs>
#include <KWindowSystem>

#include <Plasma/Containment>
#include <Plasma/ContainmentActions>
#include <Plasma/Theme>
#include <Plasma/Corona>
#include <Plasma/Applet>
#include <Plasma/Wallpaper>
#include <Plasma/WindowEffects>

#include "singleview.h"


PlasmaApp* PlasmaApp::self()
{
    if (!kapp) {
        return new PlasmaApp();
    }

    return qobject_cast<PlasmaApp*>(kapp);
}

PlasmaApp::PlasmaApp()
    : KUniqueApplication(),
      m_corona(0),
      m_maxId(0)
{
    KGlobal::locale()->insertCatalog("plasma-standaloneplasmoids");
    KCrash::setFlags(KCrash::AutoRestart);

    KConfigGroup cg(KGlobal::config(), "General");
    Plasma::Theme::defaultTheme()->setFont(cg.readEntry("desktopFont", font()));

    corona();

    KConfigGroup applets = storedConfig(0);
    foreach (const QString &group, applets.groupList()) {
        KConfigGroup appletGroup(&applets, group);

        int id = appletGroup.name().toInt();
        QString pluginName = appletGroup.readEntry("plugin", QString());
        if (id != 0 && !pluginName.isEmpty()) {
            m_storedApplets.insert(pluginName, id);
            m_maxId = qMax(id, m_maxId);
        }
    }

    //newInstance();
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
    setQuitOnLastWindowClosed(true);
}

PlasmaApp::~PlasmaApp()
{
}

KConfigGroup PlasmaApp::storedConfig(int appletId)
{
    KConfigGroup cg(m_corona->config(), "StoredApplets");

    if (appletId > 0) {
        cg = KConfigGroup(&cg, QString::number(appletId));
    }

    return cg;
}

int  PlasmaApp::newInstance()
{
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    if (args->count() == 0) {
        KCmdLineArgs::usage();
        return 0;
    }

    QString pluginName;
    if (args->count() > 0) {
        pluginName = args->arg(0);
    }

    //is the applet already running?
    if (m_viewForPlugin.contains(pluginName)) {
        m_viewForPlugin.value(pluginName)->activateWindow();
        m_viewForPlugin.value(pluginName)->raise();
        return 0;
    }

    QVariantList appletArgs;
    for (int i = 1; i < args->count(); ++i) {
        appletArgs << args->arg(i);
    }

    int appletId;
    Plasma::Containment *containment = m_corona->addContainment("null");
    containment->setFormFactor(Plasma::Planar);
    containment->setLocation(Plasma::Floating);
    appletId = ++m_maxId;

    if (m_storedApplets.contains(pluginName)) {
        int storedAppletId = m_storedApplets.values(pluginName).first();
        KConfigGroup config = storedConfig(storedAppletId);

        KConfigGroup actualConfig(containment->config());
        actualConfig = KConfigGroup(&actualConfig, "Applets");
        actualConfig = KConfigGroup(&actualConfig, QString::number(appletId));

        config.copyTo(&actualConfig);
        config.deleteGroup();
        m_storedApplets.remove(pluginName, storedAppletId);
    }

    SingleView *view = new SingleView(m_corona, containment, pluginName, appletId, appletArgs);

    if (!view->applet()) {
        delete view;
        return 0;
    }

    connect(view, SIGNAL(storeApplet(Plasma::Applet*)), this, SLOT(storeApplet(Plasma::Applet*)));
    connect(view, SIGNAL(destroyed(QObject*)), this, SLOT(viewDestroyed(QObject*)));

    if (args->isSet("border")) {
        view->setBackgroundBrush(KColorUtils::mix(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor), Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor), 0.15));
        connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(themeChanged()));
        view->applet()->setBackgroundHints(Plasma::Applet::NoBackground);
    } else {
        view->setWindowFlags(Qt::FramelessWindowHint);
        view->setAttribute(Qt::WA_TranslucentBackground);
        view->setAutoFillBackground(false);
        view->viewport()->setAutoFillBackground(false);
        view->setAttribute(Qt::WA_NoSystemBackground);
        view->viewport()->setAttribute(Qt::WA_NoSystemBackground);
        Plasma::WindowEffects::overrideShadow(view->winId(), true);
    }

    if (args->isSet("fullscreen")) {
        view->setWindowState(Qt::WindowFullScreen);
    }

    args->clear();

    m_viewForPlugin[pluginName] = view;
    m_pluginForView[view] = pluginName;
    KWindowSystem::setOnDesktop(view->winId(), KWindowSystem::currentDesktop());
    view->show();
    view->raise();

    return 0;
}


void PlasmaApp::cleanup()
{
    if (m_corona) {
        m_corona->saveLayout();
    }

    qDeleteAll(m_viewForPlugin);

    delete m_corona;
    m_corona = 0;

    //TODO: This manual sync() should not be necessary?
    syncConfig();
}

void PlasmaApp::syncConfig()
{
    KGlobal::config()->sync();
}

void PlasmaApp::themeChanged()
{
    foreach (SingleView *view, m_viewForPlugin) {
        if (view->autoFillBackground()) {
            view->setBackgroundBrush(KColorUtils::mix(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor), Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor), 0.15));
        }
    }
}

Plasma::Corona* PlasmaApp::corona()
{
    if (!m_corona) {
        m_corona = new Plasma::Corona(this);
        connect(m_corona, SIGNAL(configSynced()), this, SLOT(syncConfig()));

        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        //m_corona->initializeLayout();
    }

    return m_corona;
}

bool PlasmaApp::hasComposite()
{
    return KWindowSystem::compositingActive();
}


void PlasmaApp::storeApplet(Plasma::Applet *applet)
{
    m_storedApplets.insert(applet->name(), applet->id());
    KConfigGroup storage = storedConfig(0);
    KConfigGroup cg(applet->containment()->config());
    cg = KConfigGroup(&cg, "Applets");
    cg = KConfigGroup(&cg, QString::number(applet->id()));
    delete applet;
//    kDebug() << "storing" << applet->name() << applet->id() << "to" << storage.name() << ", applet config is" << cg.name();
    cg.reparent(&storage);
}

void PlasmaApp::viewDestroyed(QObject *view)
{
    SingleView *sView = static_cast<SingleView *>(view);
    
    m_viewForPlugin.remove(m_pluginForView.value(sView));
    m_pluginForView.remove(sView);
    if (m_viewForPlugin.isEmpty()) {
        quit();
    }
}

#include "plasmaapp.moc"
