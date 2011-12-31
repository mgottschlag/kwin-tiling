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

#include <QGraphicsLinearLayout>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>

#include <KStandardDirs>
#include <KServiceTypeTrader>
#include <knewstuff3/downloaddialog.h>
#include <KIconDialog>
#include <KWindowSystem>

#include <plasma/containment.h>
#include <plasma/package.h>
#include <plasma/widgets/declarativewidget.h>

#include "kidenticongenerator.h"
#include "plasmaapp.h"

#include <scripting/layouttemplatepackagestructure.h>
#include "scripting/desktopscriptengine.h"

class ActivityManagerPrivate
{

public:
    ActivityManagerPrivate(ActivityManager *w)
        : q(w),
          containment(0)
    {
    }

    void init(Plasma::Location location);
    void containmentDestroyed();
    void setLocation(Plasma::Location location);

    Qt::Orientation orientation;
    Plasma::Location location;
    ActivityManager *q;
    Plasma::Containment *containment;
    Plasma::DeclarativeWidget *declarativeWidget;
    Plasma::Package *package;

    QGraphicsLinearLayout *mainLayout;
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
        }
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
    emit q->orientationChanged();
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

ActivityManager::Location ActivityManager::location()
{
    return (ActivityManager::Location)d->location;
}

Qt::Orientation ActivityManager::orientation() const
{
    return d->orientation;
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
    PlasmaApp::self()->createActivity(pluginName);
}

void ActivityManager::createActivityFromScript(const QString &script, const QString &name, const QString &icon, const QStringList &startupApps)
{
    PlasmaApp::self()->createActivityFromScript(script, name, icon, startupApps);
}

void ActivityManager::downloadActivityScripts()
{
    KNS3::DownloadDialog *dialog = new KNS3::DownloadDialog( "activities.knsrc", 0 );
    connect(dialog, SIGNAL(accepted()), this, SIGNAL(activityTypeActionsChanged()));
    connect(dialog, SIGNAL(accepted()), dialog, SLOT(deleteLater()));
    dialog->show();
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
                actionDescription["startupApps"] = startupApps;

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

QString ActivityManager::chooseIcon() const
{
    KIconDialog *dialog = new KIconDialog;
    dialog->setup(KIconLoader::Desktop);
    dialog->setProperty("DoNotCloseController", true);
    KWindowSystem::setOnDesktop(dialog->winId(), KWindowSystem::currentDesktop());
    dialog->showDialog();
    KWindowSystem::forceActiveWindow(dialog->winId());
    QString icon = dialog->openDialog();
    dialog->deleteLater();
    return icon;
}

#include "activitymanager.moc"
