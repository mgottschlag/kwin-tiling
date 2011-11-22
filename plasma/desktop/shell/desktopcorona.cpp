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

#include <KAction>
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
#include <Plasma/Context>
#include <Plasma/DataEngineManager>
#include <Plasma/Package>

#include <kephal/screens.h>

#include <scripting/layouttemplatepackagestructure.h>

#include "activity.h"
#include "kworkspace/kactivitycontroller.h"
#include "kworkspace/kactivityinfo.h"
#include "panelview.h"
#include "plasmaapp.h"
#include "plasma-shell-desktop.h"
#include "scripting/desktopscriptengine.h"

static const QString s_panelTemplatesPath("plasma-layout-templates/panels/*");

DesktopCorona::DesktopCorona(QObject *parent)
    : Plasma::Corona(parent),
      m_addPanelAction(0),
      m_addPanelsMenu(0),
      m_activityController(new KActivityController(this))
{
    init();
}

DesktopCorona::~DesktopCorona()
{
    delete m_addPanelsMenu;
}

void DesktopCorona::init()
{
    setPreferredToolBoxPlugin(Plasma::Containment::DesktopContainment, "org.kde.desktoptoolbox");
    setPreferredToolBoxPlugin(Plasma::Containment::CustomContainment, "org.kde.desktoptoolbox");
    setPreferredToolBoxPlugin(Plasma::Containment::PanelContainment, "org.kde.paneltoolbox");
    setPreferredToolBoxPlugin(Plasma::Containment::CustomPanelContainment, "org.kde.paneltoolbox");

    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "DesktopCorona init start" << "(line:" << __LINE__ << ")";
    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenAdded(Kephal::Screen*)), SLOT(screenAdded(Kephal::Screen*)));
    connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SIGNAL(availableScreenRegionChanged()));

    Plasma::ContainmentActionsPluginsConfig desktopPlugins;
    desktopPlugins.addPlugin(Qt::NoModifier, Qt::Vertical, "switchdesktop");
    desktopPlugins.addPlugin(Qt::NoModifier, Qt::MidButton, "paste");
    desktopPlugins.addPlugin(Qt::NoModifier, Qt::RightButton, "contextmenu");
    Plasma::ContainmentActionsPluginsConfig panelPlugins;
    panelPlugins.addPlugin(Qt::NoModifier, Qt::RightButton, "contextmenu");

    setContainmentActionsDefaults(Plasma::Containment::DesktopContainment, desktopPlugins);
    setContainmentActionsDefaults(Plasma::Containment::CustomContainment, desktopPlugins);
    setContainmentActionsDefaults(Plasma::Containment::PanelContainment, panelPlugins);
    setContainmentActionsDefaults(Plasma::Containment::CustomPanelContainment, panelPlugins);

    checkAddPanelAction();

    //why do these actions belong to plasmaapp?
    //because it makes the keyboard shortcuts work.
    KAction *action = new KAction(PlasmaApp::self());
    action->setText(i18n("Next Activity"));
    action->setObjectName( QLatin1String("Next Activity" )); // NO I18N
    action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Tab));
    connect(action, SIGNAL(triggered()), this, SLOT(activateNextActivity()));

    action = new KAction(PlasmaApp::self());
    action->setText(i18n("Previous Activity"));
    action->setObjectName( QLatin1String("Previous Activity" )); // NO I18N
    action->setGlobalShortcut(KShortcut(Qt::META + Qt::SHIFT + Qt::Key_Tab));
    connect(action, SIGNAL(triggered()), this, SLOT(activatePreviousActivity()));

    connect(this, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
            this, SLOT(updateImmutability(Plasma::ImmutabilityType)));
    connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), this, SLOT(checkAddPanelAction(QStringList)));

    connect(m_activityController, SIGNAL(currentActivityChanged(QString)), this, SLOT(currentActivityChanged(QString)));
    connect(m_activityController, SIGNAL(activityAdded(QString)), this, SLOT(activityAdded(QString)));
    connect(m_activityController, SIGNAL(activityRemoved(QString)), this, SLOT(activityRemoved(QString)));

    mapAnimation(Plasma::Animator::AppearAnimation, Plasma::Animator::ZoomAnimation);
    mapAnimation(Plasma::Animator::DisappearAnimation, Plasma::Animator::ZoomAnimation);
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "DesktopCorona init end" << "(line:" << __LINE__ << ")";
}

void DesktopCorona::checkAddPanelAction(const QStringList &sycocaChanges)
{
    if (!sycocaChanges.isEmpty() && !sycocaChanges.contains("services")) {
        return;
    }

    delete m_addPanelAction;
    m_addPanelAction = 0;

    delete m_addPanelsMenu;
    m_addPanelsMenu = 0;

    KPluginInfo::List panelContainmentPlugins = Plasma::Containment::listContainmentsOfType("panel");
    const QString constraint = QString("[X-Plasma-Shell] == '%1' and 'panel' ~in [X-Plasma-ContainmentCategories]")
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
    int num = numScreens();
    for (int i = 0; i < num; ++i) {
        checkScreen(i, signalWhenExists);
    }
}

void DesktopCorona::checkScreen(int screen, bool signalWhenExists)
{
    // signalWhenExists is there to allow PlasmaApp to know when to create views
    // it does this only on containment addition, but in the case of a screen being
    // added and the containment already existing for that screen, no signal is emitted
    // and so PlasmaApp does not know that it needs to create a view for it. to avoid
    // taking care of that case in PlasmaApp (which would duplicate some of the code below,
    // DesktopCorona will, when signalWhenExists is true, emit a containmentAdded signal
    // even if the containment actually existed prior to this method being called.
    //
    //note: hte signal actually triggers view creation only for panels, atm.
    //desktop views are created in response to containment's screenChanged signal instead, which is
    //buggy (sometimes the containment thinks it's already on the screen, so no view is created)

    Activity *currentActivity = activity(m_activityController->currentActivity());
    //ensure the desktop(s) have a containment and view
    if (AppSettings::perVirtualDesktopViews()) {
        int numDesktops = KWindowSystem::numberOfDesktops();

        for (int j = 0; j < numDesktops; ++j) {
            checkDesktop(currentActivity, signalWhenExists, screen, j);
        }
    } else {
        checkDesktop(currentActivity, signalWhenExists, screen);
    }

    //ensure the panels get views too
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

void DesktopCorona::checkDesktop(Activity *activity, bool signalWhenExists, int screen, int desktop)
{
    Plasma::Containment *c = activity->containmentForScreen(screen, desktop);

    if (!c) {
        return;
    }

    c->setScreen(screen, desktop);
    c->flushPendingConstraintsEvents();
    requestConfigSync();

    if (signalWhenExists) {
        emit containmentAdded(c);
    }
}

int DesktopCorona::numScreens() const
{
#ifdef Q_WS_X11
    if (KGlobalSettings::isMultiHead()) {
        // with multihead, we "lie" and say that there is only one screen
        return 1;
    }
#endif

    return Kephal::ScreenUtils::numScreens();
}

QRect DesktopCorona::screenGeometry(int id) const
{
#ifdef Q_WS_X11
    if (KGlobalSettings::isMultiHead()) {
        // with multihead, we "lie" and say that screen 0 is the default screen, in fact, we pretend
        // we have only one screen at all
        Display *dpy = XOpenDisplay(NULL);
        if (dpy) {
            id = DefaultScreen(dpy);
            XCloseDisplay(dpy);
        }
    }
#endif

    return Kephal::ScreenUtils::screenGeometry(id);
}

QRegion DesktopCorona::availableScreenRegion(int id) const
{
#ifdef Q_WS_X11
    if (KGlobalSettings::isMultiHead()) {
        // with multihead, we "lie" and say that screen 0 is the default screen, in fact, we pretend
        // we have only one screen at all
        Display *dpy = XOpenDisplay(NULL);
        if (dpy) {
            id = DefaultScreen(dpy);
            XCloseDisplay(dpy);
        }
    }
#endif

    if (id < 0) {
        id = Kephal::ScreenUtils::primaryScreenId();
    }

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
    if (id < 0) {
        id = Kephal::ScreenUtils::primaryScreenId();
    }

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

int DesktopCorona::screenId(const QPoint &pos) const
{
#ifdef Q_WS_X11
    if (KGlobalSettings::isMultiHead()) {
        // with multihead, we "lie" and say that there is only one screen
        return 0;
    }
#endif

    return Kephal::ScreenUtils::screenId(pos);
}

void DesktopCorona::processUpdateScripts()
{
    evaluateScripts(WorkspaceScripting::ScriptEngine::pendingUpdateScripts());
}

void DesktopCorona::evaluateScripts(const QStringList &scripts, bool isStartup)
{
    foreach (const QString &script, scripts) {
        WorkspaceScripting::DesktopScriptEngine scriptEngine(this, isStartup);
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
    evaluateScripts(WorkspaceScripting::ScriptEngine::defaultLayoutScripts());
    if (containments().isEmpty()) {
        QString defaultConfig = KStandardDirs::locate("appdata", "plasma-default-layoutrc");
        if (!defaultConfig.isEmpty()) {
            kDebug() << "attempting to load the default layout from:" << defaultConfig;
            loadLayout(defaultConfig);
            QTimer::singleShot(1000, this, SLOT(saveDefaultSetup()));
        }
    }

    QTimer::singleShot(1000, this, SLOT(saveDefaultSetup()));
}

void DesktopCorona::saveDefaultSetup()
{
    // a "null" KConfigGroup is used to force a save into the config file
    KConfigGroup invalidConfig;

    foreach (Plasma::Containment *containment, containments()) {
        containment->save(invalidConfig);
        foreach (Plasma::Applet* applet, containment->applets()) {
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
        //FIXME: a better way to filter out what is not wanted?
        if (!plugin.property("X-Plasma-ContainmentCategories").value<QStringList>().contains("netbook")) {
            sorted.insert(plugin.name(), qMakePair(plugin, KService::Ptr(0)));
        }
    }

    const QString constraint = QString("[X-Plasma-Shell] == '%1' and 'panel' in [X-Plasma-ContainmentCategories]")
                                      .arg(KGlobal::mainComponent().componentName());
    KService::List templates = KServiceTypeTrader::self()->query("Plasma/LayoutTemplate", constraint);
    foreach (const KService::Ptr &service, templates) {
        sorted.insert(service->name(), qMakePair(emptyInfo, service));
    }

    QMapIterator<QString, QPair<KPluginInfo, KService::Ptr> > it(sorted);
    Plasma::PackageStructure::Ptr templateStructure(new WorkspaceScripting::LayoutTemplatePackageStructure);
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
            const QString path = KStandardDirs::locate("data", templateStructure->defaultPackageRoot() + '/' + info.pluginName() + '/');
            if (!path.isEmpty()) {
                Plasma::Package package(path, templateStructure);
                const QString scriptFile = package.filePath("mainscript");
                if (!scriptFile.isEmpty()) {
                    QAction *action = m_addPanelsMenu->addAction(info.name());
                    action->setData(QString::fromLatin1("plasma-desktop-template:%1").arg(scriptFile));
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
        evaluateScripts(QStringList() << plugin.right(plugin.length() - qstrlen("plasma-desktop-template:")), false);
    } else if (!plugin.isEmpty()) {
        addPanel(plugin);
    }
}

void DesktopCorona::addPanel(const QString &plugin)
{
    Plasma::Containment *panel = addContainment(plugin);
    if (!panel) {
        return;
    }

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

void DesktopCorona::checkActivities()
{
    kDebug() << "containments to start with" << containments().count();

    KActivityConsumer::ServiceStatus status = m_activityController->serviceStatus();
    //kDebug() << "$%$%$#%$%$%Status:" << status;
    if (status == KActivityConsumer::NotRunning) {
        //panic and give up - better than causing a mess
        kDebug() << "No ActivityManager? Help, I've fallen and I can't get up!";
        return;
    }

    QStringList existingActivities = m_activityController->listActivities();
    foreach (const QString &id, existingActivities) {
        activityAdded(id);
    }

    QStringList newActivities;
    QString newCurrentActivity;
    //migration checks:
    //-containments with an invalid id are deleted.
    //-containments that claim they were on a screen are kept together, and are preferred if we
    //need to initialize the current activity.
    //-containments that don't know where they were or who they were with just get made into their
    //own activity.
    foreach (Plasma::Containment *cont, containments()) {
        if ((cont->containmentType() == Plasma::Containment::DesktopContainment ||
             cont->containmentType() == Plasma::Containment::CustomContainment) &&
            !offscreenWidgets().contains(cont)) {
            Plasma::Context *context = cont->context();
            QString oldId = context->currentActivityId();
            if (!oldId.isEmpty()) {
                if (existingActivities.contains(oldId)) {
                    continue; //it's already claimed
                }
                kDebug() << "invalid id" << oldId;
                //byebye
                cont->destroy(false);
                continue;
            }
            if (cont->screen() > -1) {
                //it belongs on the current activity
                if (!newCurrentActivity.isEmpty()) {
                    context->setCurrentActivityId(newCurrentActivity);
                    continue;
                }
            }
            //discourage blank names
            if (context->currentActivity().isEmpty()) {
                context->setCurrentActivity(i18nc("Default name for a new activity", "New Activity"));
            }
            //create a new activity for the containment
            QString id = m_activityController->addActivity(context->currentActivity());
            context->setCurrentActivityId(id);
            newActivities << id;
            if (cont->screen() > -1) {
                newCurrentActivity = id;
            }
            kDebug() << "migrated" << context->currentActivityId() << context->currentActivity();
        }
    }

    kDebug() << "migrated?" << !newActivities.isEmpty() << containments().count();
    if (!newActivities.isEmpty()) {
        requestConfigSync();
    }

    //init the newbies
    foreach (const QString &id, newActivities) {
        activityAdded(id);
    }

    //ensure the current activity is initialized
    if (m_activityController->currentActivity().isEmpty()) {
        kDebug() << "guessing at current activity";
        if (existingActivities.isEmpty()) {
            if (newCurrentActivity.isEmpty()) {
                if (newActivities.isEmpty()) {
                    kDebug() << "no activities!?! Bad activitymanager, no cookie!";
                    QString id = m_activityController->addActivity(i18nc("Default name for a new activity", "New Activity"));
                    activityAdded(id);
                    m_activityController->setCurrentActivity(id);
                    kDebug() << "created emergency activity" << id;
                } else {
                    m_activityController->setCurrentActivity(newActivities.first());
                }
            } else {
                m_activityController->setCurrentActivity(newCurrentActivity);
            }
        } else {
            m_activityController->setCurrentActivity(existingActivities.first());
        }
    }
}

void DesktopCorona::currentActivityChanged(const QString &newActivity)
{
    kDebug() << newActivity;
    Activity *act =activity(newActivity);
    if (act) {
        act->ensureActive();
    }
}

Activity* DesktopCorona::activity(const QString &id)
{
    if (!m_activities.contains(id)) {
        //the add signal comes late sometimes
        activityAdded(id);
    }
    return m_activities.value(id);
}

void DesktopCorona::activityAdded(const QString &id)
{
    //TODO more sanity checks
    if (m_activities.contains(id)) {
        kDebug() << "you're late." << id;
        return;
    }

    Activity *a = new Activity(id, this);
    if (a->isCurrent()) {
        a->ensureActive();
    }
    m_activities.insert(id, a);
}

void DesktopCorona::activityRemoved(const QString &id)
{
    Activity *a = m_activities.take(id);
    a->deleteLater();
}

void DesktopCorona::activateNextActivity()
{
    QStringList list = m_activityController->listActivities(KActivityInfo::Running);
    if (list.isEmpty()) {
        return;
    }

    //FIXME: if the current activity is in transition the "next" will be the first
    int start = list.indexOf(m_activityController->currentActivity());
    int i = (start + 1) % list.size();

    m_activityController->setCurrentActivity(list.at(i));
}

void DesktopCorona::activatePreviousActivity()
{
    QStringList list = m_activityController->listActivities(KActivityInfo::Running);
    if (list.isEmpty()) {
        return;
    }

    //FIXME: if the current activity is in transition the "previous" will be the last
    int start = list.indexOf(m_activityController->currentActivity());
    //fun fact: in c++, (-1 % foo) == -1
    int i = start - 1;
    if (i < 0) {
        i = list.size() - 1;
    }

    m_activityController->setCurrentActivity(list.at(i));
}


#include "desktopcorona.moc"

