/*
 *   Copyright (C) 2007 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
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

#include "widgetexplorer.h"

#include <kaction.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <kservicetypetrader.h>
#include <kstandardaction.h>
#include <kaboutdata.h>
#include <kaboutapplicationdialog.h>
#include <kcomponentdata.h>
#include <kpluginloader.h>
#include <klineedit.h>

#include <plasma/applet.h>
#include <plasma/corona.h>
#include <plasma/containment.h>

#include "kcategorizeditemsviewmodels_p.h"
#include "plasmaappletitemmodel_p.h"
#include "openwidgetassistant_p.h"
#include "appletslist.h"
#include "managewidgets.h"
#include "appletsfiltering.h"

//getting the user local
//KGlobal::dirs()->localkdedir();
//Compare it to the entryPath of the KPluginInfo
//and see if it can be uninstalled

using namespace KCategorizedItemsViewModels;

namespace Plasma
{

class WidgetExplorerPrivate
{

public:
    WidgetExplorerPrivate(WidgetExplorer *w)
        : q(w),
          containment(0),
          config("plasmarc"),
          configGroup(&config, "Applet Browser"),
          itemModel(configGroup, w),
          filterModel(w)
    {
    }

    void initFilters();
    void init(Qt::Orientation orientation);
    void initPushButtonWidgetMenu();
    void initRunningApplets();
    void containmentDestroyed();
    void setOrientation(Qt::Orientation orientation);
    void adjustContentsSize();

    /**
     * Tracks a new running applet
     */
    void appletAdded(Plasma::Applet *applet);

    /**
     * A running applet is no more
     */
    void appletRemoved(Plasma::Applet *applet);

    Qt::Orientation orientation;
    WidgetExplorer *q;
    QString application;
    Plasma::Containment *containment;

    QHash<QString, int> runningApplets; // applet name => count
    //extra hash so we can look up the names of deleted applets
    QHash<Plasma::Applet *,QString> appletNames;

    KConfig config;
    KConfigGroup configGroup;

    PlasmaAppletItemModel itemModel;
    KCategorizedItemsViewModels::DefaultFilterModel filterModel;

    /**
     * Button to install new widgets and its menu
     */
    ManageWidgetsPushButton *pushButtonWidget;
    KMenu *pushButtonWidgetMenu;
    /**
     * Widget that lists the applets
     */
    AppletsListWidget *appletsListWidget;

    /**
     * Widget that contains the search and categories filters
     */
    FilteringWidget *filteringWidget;
    QGraphicsLinearLayout *mainLayout;

};

void WidgetExplorerPrivate::initFilters()
{
    filterModel.clear();

    filterModel.addFilter(i18n("All Widgets"),
                          KCategorizedItemsViewModels::Filter(), KIcon("plasma"));

    // Filters: Special
    filterModel.addFilter(i18n("Used Before"),
                          KCategorizedItemsViewModels::Filter("used", true),
                          KIcon("view-history"));
    filterModel.addFilter(i18n("Running"),
                          KCategorizedItemsViewModels::Filter("running", true),
                          KIcon("dialog-ok"));

    filterModel.addSeparator(i18n("Categories:"));

    typedef QPair<QString, QString> catPair;
    QMap<QString, catPair > categories;
    foreach (const QString &category, Plasma::Applet::listCategories(application)) {
        if (!Plasma::Applet::listAppletInfo(category).isEmpty()) {
            QString trans = i18n(category.toLocal8Bit());
            categories.insert(trans.toLower(), qMakePair(trans, category.toLower()));
        }
    }

    foreach (const catPair &category, categories) {
        filterModel.addFilter(category.first,
                              KCategorizedItemsViewModels::Filter("category", category.second));
    }
}

void WidgetExplorerPrivate::init(Qt::Orientation orient)
{
    //init widgets
    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setSpacing(0);
    orientation = orient;
    filteringWidget = new FilteringWidget(orientation);
    appletsListWidget = new AppletsListWidget(orientation);

    //connect
    QObject::connect(appletsListWidget, SIGNAL(appletDoubleClicked(PlasmaAppletItem*)), q, SLOT(addApplet(PlasmaAppletItem*)));
    QObject::connect(filteringWidget->textSearch()->nativeWidget(), SIGNAL(textChanged(QString)), appletsListWidget, SLOT(searchTermChanged(QString)));
    QObject::connect(filteringWidget, SIGNAL(filterChanged(int)), appletsListWidget, SLOT(filterChanged(int)));

    //adding to layout
    mainLayout->addItem(filteringWidget);
    mainLayout->addItem(appletsListWidget);
    mainLayout->setAlignment(appletsListWidget, Qt::AlignVCenter | Qt::AlignHCenter);

    //filters & models
    initFilters();
    filteringWidget->setModel(&filterModel);
    appletsListWidget->setFilterModel(&filterModel);
    appletsListWidget->setItemModel(&itemModel);
    initRunningApplets();

    q->setLayout(mainLayout);
}

void WidgetExplorerPrivate::adjustContentsSize()
{
    QSizeF contentsSize = q->contentsRect().size();

    if (appletsListWidget != 0) {
        appletsListWidget->setPreferredSize(-1,-1);
    }

    if (orientation == Qt::Horizontal) {
        if (filteringWidget != 0) {
            filteringWidget->setPreferredSize(-1, -1);
        }

    } else {
        if (filteringWidget != 0) {
            filteringWidget->setPreferredSize(-1, -1);
            filteringWidget->setMinimumHeight(contentsSize.height()/5);
            filteringWidget->setMaximumHeight(contentsSize.height()/5);
        }
    }
}

void WidgetExplorerPrivate::setOrientation(Qt::Orientation orient)
{
    orientation = orient;
    filteringWidget->setListOrientation(orientation);
    appletsListWidget->setOrientation(orientation);
}

void WidgetExplorerPrivate::initPushButtonWidgetMenu()
{
    pushButtonWidgetMenu = new KMenu(i18n("Get New Widgets"));
    QObject::connect(pushButtonWidgetMenu, SIGNAL(aboutToShow()), q, SLOT(populateWidgetsMenu()));
    pushButtonWidget->button()->setMenu(pushButtonWidgetMenu);

}

void WidgetExplorerPrivate::initRunningApplets()
{
//get applets from corona, count them, send results to model
    if (!containment) {
        return;
    }

    Plasma::Corona *c = containment->corona();

    //we've tried our best to get a corona
    //we don't want just one containment, we want them all
    if (!c) {
        kDebug() << "can't happen";
        return;
    }

    appletNames.clear();
    runningApplets.clear();
    QList<Containment*> containments = c->containments();
    foreach (Containment *containment, containments) {
        QObject::connect(containment, SIGNAL(appletAdded(Plasma::Applet*,QPointF)), q, SLOT(appletAdded(Plasma::Applet*)));
        QObject::connect(containment, SIGNAL(appletRemoved(Plasma::Applet*)), q, SLOT(appletRemoved(Plasma::Applet*)));

        foreach (Applet *applet, containment->applets()) {
            runningApplets[applet->pluginName()]++;
        }
    }

    //kDebug() << runningApplets;
    itemModel.setRunningApplets(runningApplets);
}

void WidgetExplorerPrivate::containmentDestroyed()
{
    containment = 0;
}

void WidgetExplorerPrivate::appletAdded(Plasma::Applet *applet)
{
    QString name = applet->pluginName();

    runningApplets[name]++;
    appletNames.insert(applet, name);
    itemModel.setRunningApplets(name, runningApplets[name]);
}

void WidgetExplorerPrivate::appletRemoved(Plasma::Applet *applet)
{
    Plasma::Applet *a = (Plasma::Applet *)applet; //don't care if it's valid, just need the address

    QString name = appletNames.take(a);

    int count = 0;
    if (runningApplets.contains(name)) {
        count = runningApplets[name] - 1;

        if (count < 1) {
            runningApplets.remove(name);
        } else {
            runningApplets[name] = count;
        }
    }

    itemModel.setRunningApplets(name, count);
}

//WidgetExplorer

WidgetExplorer::WidgetExplorer(QGraphicsItem *parent)
        :QGraphicsWidget(parent),
        d(new WidgetExplorerPrivate(this))
{
    d->init(Qt::Horizontal);
}

WidgetExplorer::~WidgetExplorer()
{
     delete d;
}

void WidgetExplorer::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event);
    d->adjustContentsSize();
}

void WidgetExplorer::setOrientation(Qt::Orientation orientation)
{
    d->setOrientation(orientation);
    emit(orientationChanged(orientation));
}

Qt::Orientation WidgetExplorer::orientation()
{
    return d->orientation;
}

void WidgetExplorer::setApplication(const QString &app)
{
    d->application = app;
    d->initFilters();
    d->itemModel.setApplication(app);

    //FIXME: AFAIK this shouldn't be necessary ... but here it is. need to find out what in that
    //       maze of models and views is screwing up
    d->appletsListWidget->setItemModel(&d->itemModel);

    d->itemModel.setRunningApplets(d->runningApplets);
}

QString WidgetExplorer::application()
{
    return d->application;
}

void WidgetExplorer::setContainment(Plasma::Containment *containment)
{
    if (d->containment != containment) {
        if (d->containment) {
            d->containment->disconnect(this);
        }

        d->containment = containment;

        if (d->containment) {
            connect(d->containment, SIGNAL(destroyed(QObject*)), this, SLOT(containmentDestroyed()));
        }

        d->initRunningApplets();
    }
}

Containment *WidgetExplorer::containment() const
{
    return d->containment;
}

Plasma::Corona *WidgetExplorer::corona() const
{
    if (d->containment) {
        return d->containment->corona();
    }

    return 0;
}

void WidgetExplorer::addApplet()
{
    if (!d->containment) {
        return;
    }

    foreach (AbstractItem *item, d->appletsListWidget->selectedItems()) {
        PlasmaAppletItem *selectedItem = (PlasmaAppletItem *) item;
        kDebug() << "Adding applet " << selectedItem->name() << "to containment";
        d->containment->addApplet(selectedItem->pluginName(), selectedItem->arguments());
    }
}

void WidgetExplorer::addApplet(PlasmaAppletItem *appletItem)
{
    if (!d->containment) {
        return;
    }

    kDebug() << appletItem->pluginName() << appletItem->arguments();

    d->containment->addApplet(appletItem->pluginName(), appletItem->arguments());
}

void WidgetExplorer::destroyApplets(const QString &name)
{
    if (!d->containment) {
        return;
    }

    Plasma::Corona *c = d->containment->corona();

    //we've tried our best to get a corona
    //we don't want just one containment, we want them all
    if (!c) {
        kDebug() << "can't happen";
        return;
    }

    foreach (Containment *containment, c->containments()) {
        QList<Applet*> applets = containment->applets();
        foreach (Applet *applet, applets) {
            if (applet->pluginName() == name) {
                d->appletNames.remove(applet);
                applet->disconnect(this);
                applet->destroy();
            }
        }
    }

    d->runningApplets.remove(name);
    d->itemModel.setRunningApplets(name, 0);
}

void WidgetExplorer::downloadWidgets(const QString &type)
{
    PackageStructure *installer = 0;

    if (!type.isEmpty()) {
        QString constraint = QString("'%1' == [X-KDE-PluginInfo-Name]").arg(type);
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure",
                                                                  constraint);
        if (offers.isEmpty()) {
            kDebug() << "could not find requested PackageStructure plugin" << type;
        } else {
            KService::Ptr service = offers.first();
            QString error;
            installer = service->createInstance<Plasma::PackageStructure>(topLevelWidget(),
                                                                          QVariantList(), &error);
            if (installer) {
                connect(installer, SIGNAL(newWidgetBrowserFinished()),
                        installer, SLOT(deleteLater()));
            } else {
                kDebug() << "found, but could not load requested PackageStructure plugin" << type
                         << "; reported error was" << error;
            }
        }
    }

    if (installer) {
        installer->createNewWidgetBrowser();
    } else {
        // we don't need to delete the default Applet::packageStructure as that
        // belongs to the applet
       Applet::packageStructure()->createNewWidgetBrowser();
    }
}

void WidgetExplorer::openWidgetFile()
{
    // TODO: if we already have one of these showing and the user clicks to
    // add it again, show the same window?
    OpenWidgetAssistant *assistant = new OpenWidgetAssistant(0);
    assistant->setAttribute(Qt::WA_DeleteOnClose, true);
    assistant->show();
}

void WidgetExplorer::populateWidgetsMenu()
{

    if (!d->pushButtonWidgetMenu->actions().isEmpty()) {
        // already populated.
        return;
    }

    QSignalMapper *mapper = new QSignalMapper(this);
    QObject::connect(mapper, SIGNAL(mapped(QString)), this, SLOT(downloadWidgets(QString)));

    QAction *action = new QAction(KIcon("applications-internet"),
                                  i18n("Download New Plasma Widgets"), this);
    QObject::connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
    mapper->setMapping(action, QString());
    d->pushButtonWidgetMenu->addAction(action);

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure");
    foreach (const KService::Ptr &service, offers) {
        //kDebug() << service->property("X-Plasma-ProvidesWidgetBrowser");
        if (service->property("X-Plasma-ProvidesWidgetBrowser").toBool()) {
            QAction *action = new QAction(KIcon("applications-internet"),
                                          i18nc("%1 is a type of widgets, as defined by "
                                                "e.g. some plasma-packagestructure-*.desktop files",
                                                "Download New %1", service->name()), this);
            QObject::connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
            mapper->setMapping(action, service->property("X-KDE-PluginInfo-Name").toString());
            d->pushButtonWidgetMenu->addAction(action);
        }
    }

    d->pushButtonWidgetMenu->addSeparator();

    action = new QAction(KIcon("package-x-generic"),
                         i18n("Install Widget From Local File..."), this);
    QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT(openWidgetFile()));
    d->pushButtonWidgetMenu->addAction(action);
}

} // namespace Plasma

#include "widgetexplorer.moc"
