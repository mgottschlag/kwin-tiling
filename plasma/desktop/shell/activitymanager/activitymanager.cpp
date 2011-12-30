/*
 *   Copyright (C) 2007 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "activitymanager.h"

#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>

#include <kaction.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <kstandardaction.h>
#include <klineedit.h>
#include <KStandardDirs>
#include <KServiceTypeTrader>

#include <plasma/applet.h>
#include <plasma/corona.h>
#include <plasma/containment.h>
#include <plasma/package.h>
#include <plasma/widgets/toolbutton.h>
#include <plasma/widgets/lineedit.h>
#include <plasma/widgets/declarativewidget.h>

#include "activitylist.h"
#include "filterbar.h"
#include "kidenticongenerator.h"
#include "plasmaapp.h"

#include <scripting/layouttemplatepackagestructure.h>
#include "scripting/desktopscriptengine.h"

class ActivityManagerPrivate
{

public:
    ActivityManagerPrivate(ActivityManager *w)
        : q(w),
          containment(0),
          iconSize(16) //FIXME bad!
    {
    }

    void init(Plasma::Location location);
    void containmentDestroyed();
    void setLocation(Plasma::Location location);

    Qt::Orientation orientation;
    Plasma::Location location;
    ActivityManager *q;
    Plasma::ToolButton *close;
    Plasma::Containment *containment;
    Plasma::DeclarativeWidget *declarativeWidget;
    Plasma::Package *package;

    /**
     * Widget that lists the applets
     */
    ActivityList *activityList;

    /**
     * Widget that contains the search and categories filters
     */
    FilterBar *filteringWidget;
    QGraphicsLinearLayout *filteringLayout;
    QGraphicsLinearLayout *mainLayout;
    int iconSize;
};

void ActivityManagerPrivate::init(Plasma::Location loc)
{
    location = loc;
    //init widgets
    if (loc == Plasma::LeftEdge || loc == Plasma::RightEdge) {
        orientation = Qt::Vertical;
    } else {
        orientation = Qt::Horizontal;
    }

    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    filteringLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    filteringWidget = new FilterBar(orientation, q);

    activityList = new ActivityList(loc);
    close = new Plasma::ToolButton;
    close->setIcon(KIcon("dialog-close"));

    Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
    package = new Plasma::Package(QString(), "org.kde.desktop.activitymanager", structure);

    declarativeWidget = new Plasma::DeclarativeWidget(q);
    declarativeWidget->setInitializationDelayed(true);
    declarativeWidget->setQmlPath(package->filePath("mainscript"));
    mainLayout->addItem(declarativeWidget);

    //the activitymanager class will be directly accessible from qml
    if (declarativeWidget->engine()) {
        QDeclarativeContext *ctxt = declarativeWidget->engine()->rootContext();
        if (ctxt) {
            ctxt->setContextProperty("activityManager", q);
            //QObject::connect(declarativeWidget, SIGNAL(finished()), q, SLOT(finished()));
        }
    }

    //connect
    QObject::connect(filteringWidget, SIGNAL(searchTermChanged(QString)), activityList, SLOT(searchTermChanged(QString)));
    QObject::connect(filteringWidget, SIGNAL(addWidgetsRequested()), q, SIGNAL(addWidgetsRequested()));
    QObject::connect(close, SIGNAL(clicked()), q, SIGNAL(closeClicked()));

    //adding to layout
    if (orientation == Qt::Horizontal) {
        filteringLayout->addItem(filteringWidget);
    } else {
        mainLayout->addItem(filteringWidget);
    }

    mainLayout->addItem(filteringLayout);
    mainLayout->addItem(activityList);
    activityList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->setAlignment(activityList, Qt::AlignTop | Qt::AlignHCenter);

    if (orientation == Qt::Horizontal) {
        filteringLayout->addItem(close);
        filteringLayout->setAlignment(close, Qt::AlignVCenter | Qt::AlignHCenter);
    } else {
        mainLayout->setAlignment(filteringWidget, Qt::AlignTop | Qt::AlignHCenter);
        mainLayout->setStretchFactor(activityList, 10);
        mainLayout->addItem(close);
    }

    q->setLayout(mainLayout);
}

void ActivityManagerPrivate::setLocation(Plasma::Location loc)
{
    Qt::Orientation orient;
    if (loc == Plasma::LeftEdge || loc == Plasma::RightEdge) {
        orient = Qt::Vertical;
    } else {
        orient = Qt::Horizontal;
    }

    if (orientation == orient) {
        return;
    }

    location = loc;
//FIXME bet I could make this more efficient
    orientation = orient;
    filteringWidget->setOrientation(orientation);
    activityList->setLocation(containment->location());
    if (orientation == Qt::Horizontal) {
        mainLayout->removeItem(filteringWidget);
        mainLayout->removeItem(close);
        filteringLayout->addItem(filteringWidget);
        filteringLayout->addItem(close);
        filteringLayout->setAlignment(close, Qt::AlignVCenter | Qt::AlignHCenter);
    } else {
        filteringLayout->removeItem(filteringWidget);
        filteringLayout->removeItem(close);
        mainLayout->insertItem(0, filteringWidget);
        mainLayout->addItem(close);
        mainLayout->setAlignment(filteringWidget, Qt::AlignTop | Qt::AlignHCenter);
        mainLayout->setStretchFactor(activityList, 10);
    }
}

void ActivityManagerPrivate::containmentDestroyed()
{
    containment = 0;
}

//ActivityBar

ActivityManager::ActivityManager(Plasma::Location loc, QGraphicsItem *parent)
        :QGraphicsWidget(parent),
        d(new ActivityManagerPrivate(this))
{
    d->init(loc);
}

ActivityManager::ActivityManager(QGraphicsItem *parent)
        :QGraphicsWidget(parent),
        d(new ActivityManagerPrivate(this))
{
    d->init(Plasma::BottomEdge);
}

ActivityManager::~ActivityManager()
{
     delete d;
}

void ActivityManager::setLocation(Plasma::Location loc)
{
    d->setLocation(loc);
    emit(locationChanged(loc));
}

Plasma::Location ActivityManager::location()
{
    return d->location;
}

void ActivityManager::setIconSize(int size)
{
    d->activityList->setIconSize(size);
    adjustSize();
}

int ActivityManager::iconSize() const
{
    return d->activityList->iconSize();
}

QPixmap ActivityManager::pixmapForActivity(const QString &activityId)
{
    return KIdenticonGenerator::self()->generatePixmap(KIconLoader::SizeHuge, activityId);
}

void ActivityManager::cloneCurrentActivity()
{
    PlasmaApp::self()->cloneCurrentActivity();
}

void ActivityManager::createActivity(const QString &pluginName)
{
    
}

void ActivityManager::setContainment(Plasma::Containment *containment)
{
    kDebug() << "Setting containment to" << containment;
    if (d->containment != containment) {
        if (d->containment) {
            d->containment->disconnect(this);
        }

        d->containment = containment;

        if (d->containment) {
            connect(d->containment, SIGNAL(destroyed(QObject*)), this, SLOT(containmentDestroyed()));
        }
    }
}

QList<QVariant> ActivityManager::activityTypeActions()
{
    QList<QVariant> actions;

    QMap<QString, QVariantHash> sorted; //qmap sorts alphabetically

    //regular plugins
    KPluginInfo::List plugins = Plasma::Containment::listContainmentsOfType("desktop");
    foreach (const KPluginInfo& info, plugins) {
        if (info.property("NoDisplay").toBool()) {
            continue;
        }
        QVariantHash actionDescription;
        //skip desktop, it's in the top level menu
        if (info.pluginName() != "desktop") {
            actionDescription["icon"] = info.icon();
            actionDescription["text"] = info.name();
            actionDescription["separator"] = false;
            actionDescription["pluginName"] = info.pluginName();
            sorted.insert(info.name(), actionDescription);
        }
    }

    //templates
    const QString constraint = QString("[X-Plasma-Shell] == '%1' and 'desktop' ~in [X-Plasma-ContainmentCategories]")
                                      .arg(KGlobal::mainComponent().componentName());
    KService::List templates = KServiceTypeTrader::self()->query("Plasma/LayoutTemplate", constraint);
    foreach (const KService::Ptr &service, templates) {
        KPluginInfo info(service);
        Plasma::PackageStructure::Ptr structure(new WorkspaceScripting::LayoutTemplatePackageStructure);
        const QString path = KStandardDirs::locate("data", structure->defaultPackageRoot() + '/' + info.pluginName() + '/');
        if (!path.isEmpty()) {
            Plasma::Package package(path, structure);
            const QString scriptFile = package.filePath("mainscript");
            const QStringList startupApps = service->property("X-Plasma-ContainmentLayout-ExecuteOnCreation", QVariant::StringList).toStringList();

            if (!scriptFile.isEmpty() || !startupApps.isEmpty()) {
                QVariantHash actionDescription;

                actionDescription["icon"] = info.icon();
                actionDescription["text"] = info.name();
                actionDescription["separator"] = false;
                actionDescription["pluginName"] = QString();
                actionDescription["scriptFile"] = scriptFile;
                actionDescription["startupapps"] = startupApps;

                sorted.insert(info.name(), actionDescription);
            }
        }
    }

    //set up sorted menu
    foreach (QVariantHash actionDescription, sorted) {
        actions << actionDescription;
    }

    //separator
    {
        QVariantHash actionDescription;
        actionDescription["separator"] = true;
        actions << actionDescription;
    }

    //ghns
    //FIXME
    {
        QVariantHash actionDescription;
        actionDescription["icon"] = "get-hot-new-stuff";
        actionDescription["text"] = i18n("Get New Templates...");
        actionDescription["separator"] = false;
        actions << actionDescription;
    }

    return actions;
}

void ActivityManager::populateActivityMenu()
{
/*    QTimer::singleShot(0, this, SLOT(setMenuPos()));
    if (!m_newActivityMenu->actions().isEmpty()) {
        // already populated.
        return;
    }

    QMenu *templatesMenu = m_newActivityMenu->addMenu(i18n("Templates"));
    QMap<QString, QAction*> sorted; //qmap sorts alphabetically

    //regular plugins
    KPluginInfo::List plugins = Plasma::Containment::listContainmentsOfType("desktop");
    foreach (const KPluginInfo& info, plugins) {
        if (info.property("NoDisplay").toBool()) {
            continue;
        }
        QAction *action;
        if (info.pluginName() == "desktop") { //suggest this one for newbies
            action = m_newActivityMenu->addAction(KIcon(info.icon()), i18n("Empty Desktop"));
        } else {
            action = new QAction(KIcon(info.icon()), info.name(), templatesMenu);
            sorted.insert(info.name(), action);
        }
        action->setData(info.pluginName());
    }

    //templates
    const QString constraint = QString("[X-Plasma-Shell] == '%1' and 'desktop' ~in [X-Plasma-ContainmentCategories]")
                                      .arg(KGlobal::mainComponent().componentName());
    KService::List templates = KServiceTypeTrader::self()->query("Plasma/LayoutTemplate", constraint);
    foreach (const KService::Ptr &service, templates) {
        KPluginInfo info(service);
        Plasma::PackageStructure::Ptr structure(new WorkspaceScripting::LayoutTemplatePackageStructure);
        const QString path = KStandardDirs::locate("data", structure->defaultPackageRoot() + '/' + info.pluginName() + '/');
        if (!path.isEmpty()) {
            Plasma::Package package(path, structure);
            const QString scriptFile = package.filePath("mainscript");
            const QStringList startupApps = service->property("X-Plasma-ContainmentLayout-ExecuteOnCreation", QVariant::StringList).toStringList();

            if (!scriptFile.isEmpty() || !startupApps.isEmpty()) {
                QAction *action = new QAction(KIcon(info.icon()), info.name(), templatesMenu);
                QVariantList data;
                data << scriptFile << info.name() << info.icon() << startupApps;
                action->setData(data);
                sorted.insert(info.name(), action);
            }
        }
    }

    //set up sorted menu
    foreach (QAction *action, sorted) {
        templatesMenu->addAction(action);
    }

    //clone
    QAction *action = m_newActivityMenu->addAction(KIcon("edit-copy"), i18n("Clone current activity"));
    action->setData(0);

    //ghns
    templatesMenu->addSeparator();
    action = templatesMenu->addAction(KIcon("get-hot-new-stuff"), i18n("Get New Templates..."));
    action->setData(1);*/
}

void ActivityManager::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event);
    qDebug() << "ActivityManager::focusInEvent()";
    QTimer::singleShot(300, d->filteringWidget, SLOT(setFocus())); 
}

#include "activitymanager.moc"
