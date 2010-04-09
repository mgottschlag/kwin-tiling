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

#include "desktopcorona.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QGraphicsLayout>
#include <QTimer>
#include <QMenu>
#include <QSignalMapper>

#include <KDebug>
#include <KDialog>
#include <KGlobal>
#include <KGlobalSettings>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KSycoca>
#include <KWindowSystem>

#include <Plasma/AbstractToolBox>
#include <Plasma/Containment>
#include <plasma/containmentactionspluginsconfig.h>
#include <Plasma/DataEngineManager>
#include <Plasma/Package>

#include <kephal/screens.h>

#include <scripting/layouttemplatepackagestructure.h>

#include "panelview.h"
#include "plasmaapp.h"
#include "plasma-shell-desktop.h"
#include "scripting/desktopscriptengine.h"

static const QString s_panelTemplatesPath("plasma-layout-templates/panels/*");

DesktopCorona::DesktopCorona(QObject *parent)
    : Plasma::Corona(parent),
      m_addPanelAction(0),
      m_addPanelsMenu(0)
{
    init();
}

DesktopCorona::~DesktopCorona()
{
    delete m_addPanelsMenu;
}

void DesktopCorona::init()
{
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "DesktopCorona init start" << "(line:" << __LINE__ << ")";
    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenAdded(Kephal::Screen *)), SLOT(screenAdded(Kephal::Screen *)));
    connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SIGNAL(availableScreenRegionChanged()));

    Plasma::ContainmentActionsPluginsConfig desktopPlugins;
    desktopPlugins.addPlugin(Qt::NoModifier, Qt::Vertical, "switchdesktop");
    desktopPlugins.addPlugin(Qt::NoModifier, Qt::MidButton, "paste");
    desktopPlugins.addPlugin(Qt::NoModifier, Qt::RightButton, "contextmenu");
    Plasma::ContainmentActionsPluginsConfig panelPlugins;
    panelPlugins.addPlugin(Qt::NoModifier, Qt::RightButton, "contextmenu");

    setContainmentActionsDefaults(Plasma::Containment::DesktopContainment, desktopPlugins);
    setContainmentActionsDefaults(Plasma::Containment::PanelContainment, panelPlugins);
    setContainmentActionsDefaults(Plasma::Containment::CustomPanelContainment, panelPlugins);

    checkAddPanelAction();

    connect(this, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
            this, SLOT(updateImmutability(Plasma::ImmutabilityType)));
    connect(KSycoca::self(), SIGNAL(databaseChanged(QString)), this, SLOT(checkAddPanelAction(QString)));

    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "DesktopCorona init end" << "(line:" << __LINE__ << ")";
}

void DesktopCorona::checkAddPanelAction(const QString &sycocaChanges)
{
    if (sycocaChanges != "services") {
        return;
    }

    delete m_addPanelAction;
    m_addPanelAction = 0;

    delete m_addPanelsMenu;
    m_addPanelsMenu = 0;

    KPluginInfo::List panelContainmentPlugins = Plasma::Containment::listContainmentsOfType("panel");
    //FIXME: this will have to become a dynamic choice between a menu and  simple action, i think
    const QString constraint = QString("[X-Plasma-Shell] == '%1'")
                                      .arg(KGlobal::mainComponent().componentName());
    KService::List templates = KServiceTypeTrader::self()->query("Plasma/LayoutTemplate", constraint);

    if (panelContainmentPlugins.count() + templates.count() == 1) {
        m_addPanelAction = new QAction(i18n("Add Panel"), this);
        m_addPanelAction->setData(Plasma::AbstractToolBox::AddTool);
        connect(m_addPanelAction, SIGNAL(triggered(bool)), this, SLOT(addPanel()));
    } else if (!panelContainmentPlugins.isEmpty()) {
        m_addPanelsMenu = new QMenu;
        m_addPanelAction = m_addPanelsMenu->menuAction();
        m_addPanelAction->setText(i18n("Add Panel"));
        m_addPanelAction->setData(Plasma::AbstractToolBox::AddTool);
        kDebug() << "populateAddPanelsMenu" << panelContainmentPlugins.count();
        connect(m_addPanelsMenu, SIGNAL(aboutToShow()), this, SLOT(populateAddPanelsMenu()));
        connect(m_addPanelsMenu, SIGNAL(triggered(QAction*)), this, SLOT(addPanel(QAction*)));
    }

    if (m_addPanelAction) {
        m_addPanelAction->setIcon(KIcon("list-add"));
        addAction("add panel", m_addPanelAction);
    }
}

void DesktopCorona::updateImmutability(Plasma::ImmutabilityType immutability)
{
    if (m_addPanelAction) {
        m_addPanelAction->setEnabled(immutability == Plasma::Mutable);
    }
}

void DesktopCorona::checkScreens(bool signalWhenExists)
{
    // quick sanity check to ensure we have containments for each screen
    int numScreens = Kephal::ScreenUtils::numScreens();
    for (int i = 0; i < numScreens; ++i) {
        checkScreen(i, signalWhenExists);
    }
}

void DesktopCorona::checkScreen(int screen, bool signalWhenExists)
{
    if (AppSettings::perVirtualDesktopViews()) {
        int numDesktops = KWindowSystem::numberOfDesktops();

        for (int j = 0; j < numDesktops; ++j) {
            Plasma::Containment *c = containmentForScreen(screen, j);

            kDebug() << screen << j << (QObject*)c;
            if (!c) {
                addDesktopContainment(screen, j);
            } else if (signalWhenExists) {
                emit containmentAdded(c);
            }
        }
    } else {
        Plasma::Containment *c = containmentForScreen(screen);
        if (!containmentForScreen(screen)) {
            addDesktopContainment(screen);
        } else if (signalWhenExists) {
            emit containmentAdded(c);
        }
    }

    if (signalWhenExists) {
        foreach (Plasma::Containment * c, containments()) {
            if (c->screen() != screen) {
                continue;
            }

            Plasma::Containment::Type t = c->containmentType();
            if (t == Plasma::Containment::PanelContainment ||
                t == Plasma::Containment::CustomPanelContainment) {
                emit containmentAdded(c);
            }
        }
    }
}

Plasma::Containment *DesktopCorona::findFreeContainment() const
{
    foreach (Plasma::Containment *cont, containments()) {
        if ((cont->containmentType() == Plasma::Containment::DesktopContainment ||
            cont->containmentType() == Plasma::Containment::CustomContainment) &&
            cont->screen() == -1 && !offscreenWidgets().contains(cont)) {
            return cont;
        }
    }

    return 0;
}

void DesktopCorona::addDesktopContainment(int screen, int desktop)
{
    kDebug() << screen << desktop;

    Plasma::Containment* c = findFreeContainment();
    if (!c) {
        // first try for "desktop", if it doesn't exist then we try for any 
        // desktopy containment
        c = addContainment("desktop");

        if (!c) {
            KPluginInfo::List desktopPlugins = Plasma::Containment::listContainmentsOfType("desktop");

            if (!desktopPlugins.isEmpty()) {
                c = addContainment(desktopPlugins.first().pluginName());
            }
        }

        if (!c) {
            kWarning() << "complete failure to load a desktop containment!";
            return;
        }

        c->setActivity(i18n("Desktop"));
    } else {
        kDebug() << "found an existing containment:" << c->screen() << c->desktop();
    }

    c->setScreen(screen, desktop);
    c->setFormFactor(Plasma::Planar);
    c->flushPendingConstraintsEvents();
    emit containmentAdded(c);
    requestConfigSync();
}

int DesktopCorona::numScreens() const
{
    return Kephal::ScreenUtils::numScreens();
}

QRect DesktopCorona::screenGeometry(int id) const
{
    return Kephal::ScreenUtils::screenGeometry(id);
}

QRegion DesktopCorona::availableScreenRegion(int id) const
{
    QRegion r(screenGeometry(id));
    foreach (PanelView *view, PlasmaApp::self()->panelViews()) {
        if (view->screen() == id && view->visibilityMode() == PanelView::NormalPanel) {
            r = r.subtracted(view->geometry());
        }
    }

    return r;
}

QRect DesktopCorona::availableScreenRect(int id) const
{
    QRect r(screenGeometry(id));
    foreach (PanelView *view, PlasmaApp::self()->panelViews()) {
        if (view->screen() == id && view->visibilityMode() == PanelView::NormalPanel) {
            QRect v = view->geometry();
            switch (view->location()) {
                case Plasma::TopEdge:
                    if (v.bottom() > r.top()) {
                        r.setTop(v.bottom());
                    }
                    break;

                case Plasma::BottomEdge:
                    if (v.top() < r.bottom()) {
                        r.setBottom(v.top());
                    }
                    break;

                case Plasma::LeftEdge:
                    if (v.right() > r.left()) {
                        r.setLeft(v.right());
                    }
                    break;

                case Plasma::RightEdge:
                    if (v.left() < r.right()) {
                        r.setRight(v.left());
                    }
                    break;

                default:
                    break;
            }
        }
    }

    return r;
}

void DesktopCorona::processUpdateScripts()
{
    evaluateScripts(ScriptEngine::pendingUpdateScripts());
}

void DesktopCorona::evaluateScripts(const QStringList &scripts)
{
    foreach (const QString &script, scripts) {
        DesktopScriptEngine scriptEngine(this);
        connect(&scriptEngine, SIGNAL(printError(QString)), this, SLOT(printScriptError(QString)));
        connect(&scriptEngine, SIGNAL(print(QString)), this, SLOT(printScriptMessage(QString)));
        connect(&scriptEngine, SIGNAL(createPendingPanelViews()), PlasmaApp::self(), SLOT(createWaitingPanels()));

        QFile file(script);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
            QString code = file.readAll();
            kDebug() << "evaluating startup script:" << script;
            scriptEngine.evaluateScript(code);
        }
    }
}

void DesktopCorona::printScriptError(const QString &error)
{
    kWarning() << "Startup script errror:" << error;
}

void DesktopCorona::printScriptMessage(const QString &error)
{
    kDebug() << "Startup script: " << error;
}

void DesktopCorona::loadDefaultLayout()
{
    evaluateScripts(ScriptEngine::defaultLayoutScripts());
    if (!containments().isEmpty()) {
        return;
    }

    QString defaultConfig = KStandardDirs::locate("appdata", "plasma-default-layoutrc");
    if (!defaultConfig.isEmpty()) {
        kDebug() << "attempting to load the default layout from:" << defaultConfig;
        loadLayout(defaultConfig);

        if (!containments().isEmpty()) {
            return;
        }
    }

    kDebug() << "number of screens is" << Kephal::ScreenUtils::numScreens();
    int topLeftScreen = 0;
    QPoint topLeftCorner = Kephal::ScreenUtils::screenGeometry(0).topLeft();

    // find our "top left" screen, use it as the primary
    for (int i = 0; i < Kephal::ScreenUtils::numScreens(); ++i) {
        QRect g = Kephal::ScreenUtils::screenGeometry(i);
        kDebug() << "     screen " << i << "geometry is" << g;

        if (g.x() <= topLeftCorner.x() && g.y() >= topLeftCorner.y()) {
            topLeftCorner = g.topLeft();
            topLeftScreen = i;
        }
    }

    // create a containment for each screen
    for (int i = 0; i < Kephal::ScreenUtils::numScreens(); ++i) {
        // passing in an empty string will get us whatever the default
        // containment type is!
        Plasma::Containment* c = addContainmentDelayed(QString());

        if (!c) {
            continue;
        }

        c->init();
        c->setScreen(i, -1);
        c->resize(screenGeometry(i).size());
        c->setWallpaper("image", "SingleImage");
        c->setFormFactor(Plasma::Planar);
        c->updateConstraints(Plasma::StartupCompletedConstraint);
        c->flushPendingConstraintsEvents();

        // put a folder view on the first screen
        if (i == topLeftScreen) {
            QString desktopPath = KGlobalSettings::desktopPath();
            QDir desktopFolder(desktopPath);
            if (desktopPath != QDir::homePath() && desktopFolder.exists()) {
                Plasma::Applet *folderView =  Plasma::Applet::load("folderview", c->id() + 1);
                if (folderView) {
                    c->addApplet(folderView, QPointF(KDialog::spacingHint(), KDialog::spacingHint()), true);
                    KConfigGroup config = folderView->config();
                    config.writeEntry("url", "desktop:/");
                }
            }
        }

        emit containmentAdded(c);
    }

    // make a panel at the bottom
    Plasma::Containment *panel = addContainmentDelayed("panel");

    if (!panel) {
        return;
    }

    panel->init();
    panel->setScreen(topLeftScreen);
    panel->setLocation(Plasma::BottomEdge);

    const int newHeight = 28;
    panel->resize(QSize((int)panel->size().width(), newHeight));
    panel->setMinimumSize(QSize((int)panel->minimumSize().width(), newHeight));
    panel->setMaximumSize(QSize((int)panel->maximumSize().width(), newHeight));

    panel->updateConstraints(Plasma::StartupCompletedConstraint);
    panel->flushPendingConstraintsEvents();

    // some default applets to get a usable UI
    Plasma::Applet *applet = loadDefaultApplet("launcher", panel);
    if (applet) {
        applet->setGlobalShortcut(KShortcut("Alt+F1"));
    }

    loadDefaultApplet("pager", panel);
    loadDefaultApplet("tasks", panel);
    Plasma::Applet *sysTray = loadDefaultApplet("systemtray", panel);

    if (sysTray) {
        QAction *addDefaultApplets = sysTray->action("add default applets");
        if (addDefaultApplets) {
            addDefaultApplets->trigger();
        }
    }

    loadDefaultApplet("digital-clock", panel);
    emit containmentAdded(panel);

    QTimer::singleShot(1000, this, SLOT(saveDefaultSetup()));
}

void DesktopCorona::saveDefaultSetup()
{
    // a "null" KConfigGroup is used to force a save into the config file
    KConfigGroup invalidConfig;

    foreach (Plasma::Containment *containment, containments()) {
        containment->save(invalidConfig);

        foreach (Plasma::Applet* applet, containment->applets()) {
            applet->init();
            applet->updateConstraints(Plasma::AllConstraints | Plasma::StartupCompletedConstraint);
            applet->flushPendingConstraintsEvents();
            applet->save(invalidConfig);
        }
    }

    requestConfigSync();
}

Plasma::Applet *DesktopCorona::loadDefaultApplet(const QString &pluginName, Plasma::Containment *c)
{
    QVariantList args;
    Plasma::Applet *applet = Plasma::Applet::load(pluginName, 0, args);

    if (applet) {
        c->addApplet(applet);
    }

    return applet;
}

void DesktopCorona::screenAdded(Kephal::Screen *s)
{
    kDebug() << s->id();
    checkScreen(s->id(), true);
}

void DesktopCorona::populateAddPanelsMenu()
{
    m_addPanelsMenu->clear();
    const KPluginInfo emptyInfo;

    KPluginInfo::List panelContainmentPlugins = Plasma::Containment::listContainmentsOfType("panel");
    QMap<QString, QPair<KPluginInfo, KService::Ptr> > sorted;
    foreach (const KPluginInfo &plugin, panelContainmentPlugins) {
        sorted.insert(plugin.name(), qMakePair(plugin, KService::Ptr(0)));
    }

    const QString constraint = QString("[X-Plasma-Shell] == '%1'")
                                      .arg(KGlobal::mainComponent().componentName());
    KService::List templates = KServiceTypeTrader::self()->query("Plasma/LayoutTemplate", constraint);
    foreach (const KService::Ptr &service, templates) {
        sorted.insert(service->name(), qMakePair(emptyInfo, service));
    }

    QMapIterator<QString, QPair<KPluginInfo, KService::Ptr> > it(sorted);
    Plasma::PackageStructure::Ptr templateStructure(new LayoutTemplatePackageStructure);
    while (it.hasNext()) {
        it.next();
        QPair<KPluginInfo, KService::Ptr> pair = it.value();
        if (pair.first.isValid()) {
            KPluginInfo plugin = pair.first;
            QAction *action = m_addPanelsMenu->addAction(plugin.name());
            if (!plugin.icon().isEmpty()) {
                action->setIcon(KIcon(plugin.icon()));
            }

            action->setData(plugin.pluginName());
        } else {
            //FIXME: proper names
            KPluginInfo info(pair.second);
            Plasma::PackageStructure::Ptr structure(new LayoutTemplatePackageStructure);
            const QString path = KStandardDirs::locate("data", structure->defaultPackageRoot() + '/' + info.pluginName() + '/');
            if (!path.isEmpty()) {
                Plasma::Package package(path, structure);
                const QString scriptFile = package.filePath("mainscript");
                if (!scriptFile.isEmpty()) {
                    QAction *action = m_addPanelsMenu->addAction(info.name());
                    action->setData("plasma-desktop-template:" + scriptFile);
                }
            }
        }
    }
}

void DesktopCorona::addPanel()
{
    KPluginInfo::List panelPlugins = Plasma::Containment::listContainmentsOfType("panel");

    if (!panelPlugins.isEmpty()) {
        addPanel(panelPlugins.first().pluginName());
    }
}

void DesktopCorona::addPanel(QAction *action)
{
    const QString plugin = action->data().toString();
    if (plugin.startsWith("plasma-desktop-template:")) {
        evaluateScripts(QStringList() << plugin.right(plugin.length() - qstrlen("plasma-desktop-template:")));
    } else if (!plugin.isEmpty()) {
        addPanel(plugin);
    }
}

void DesktopCorona::addPanel(const QString &plugin)
{
    Plasma::Containment* panel = addContainment(plugin);
    panel->showConfigurationInterface();

    //Fall back to the cursor position since we don't know what is the originating containment
    const int screen = Kephal::ScreenUtils::screenId(QCursor::pos());

    panel->setScreen(screen);

    QList<Plasma::Location> freeEdges = DesktopCorona::freeEdges(screen);
    //kDebug() << freeEdges;
    Plasma::Location destination;
    if (freeEdges.contains(Plasma::TopEdge)) {
        destination = Plasma::TopEdge;
    } else if (freeEdges.contains(Plasma::BottomEdge)) {
        destination = Plasma::BottomEdge;
    } else if (freeEdges.contains(Plasma::LeftEdge)) {
        destination = Plasma::LeftEdge;
    } else if (freeEdges.contains(Plasma::RightEdge)) {
        destination = Plasma::RightEdge;
    } else destination = Plasma::TopEdge;

    panel->setLocation(destination);

    const QRect screenGeom = screenGeometry(screen);
    const QRegion availGeom = availableScreenRegion(screen);
    int minH = 10;
    int minW = 10;
    int w = 35;
    int h = 35;

    //FIXME: this should really step through the rects on the relevant screen edge to find
    //appropriate space
    if (destination == Plasma::LeftEdge) {
        QRect r = availGeom.intersected(QRect(screenGeom.x(), screenGeom.y(), w, screenGeom.height())).boundingRect();
        h = r.height();
        minW = 35;
        minH = h;
    } else if (destination == Plasma::RightEdge) {
        QRect r = availGeom.intersected(QRect(screenGeom.right() - w, screenGeom.y(), w, screenGeom.height())).boundingRect();
        h = r.height();
        minW = 35;
        minH = h;
    } else if (destination == Plasma::TopEdge) {
        QRect r = availGeom.intersected(QRect(screenGeom.x(), screenGeom.y(), screenGeom.width(), h)).boundingRect();
        w = r.width();
        minH = 35;
        minW = w;
    } else if (destination == Plasma::BottomEdge) {
        QRect r = availGeom.intersected(QRect(screenGeom.x(), screenGeom.bottom() - h, screenGeom.width(), h)).boundingRect();
        w = r.width();
        minH = 35;
        minW = w;
    }

    panel->setMinimumSize(minW, minH);
    panel->setMaximumSize(w, h);
    panel->resize(w, h);
}

#include "desktopcorona.moc"

