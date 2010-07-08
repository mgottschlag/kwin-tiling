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
#include <plasma/widgets/toolbutton.h>
#include <plasma/widgets/lineedit.h>

#include "kcategorizeditemsviewmodels_p.h"
#include "plasmaappletitemmodel_p.h"
#include "appletslist.h"
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
          itemModel(w),
          filterModel(w),
          iconSize(16)
    {
    }

    void initFilters();
    void init(Qt::Orientation orientation);
    void initRunningApplets();
    void containmentDestroyed();
    void setOrientation(Qt::Orientation orientation);

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
    Plasma::ToolButton *close;
    QString application;
    Plasma::Containment *containment;

    QHash<QString, int> runningApplets; // applet name => count
    //extra hash so we can look up the names of deleted applets
    QHash<Plasma::Applet *,QString> appletNames;

    PlasmaAppletItemModel itemModel;
    KCategorizedItemsViewModels::DefaultFilterModel filterModel;

    /**
     * Widget that lists the applets
     */
    AppletsListWidget *appletsListWidget;

    /**
     * Widget that contains the search and categories filters
     */
    FilteringWidget *filteringWidget;
    QGraphicsLinearLayout *filteringLayout;
    QGraphicsLinearLayout *mainLayout;
    int iconSize;
};

void WidgetExplorerPrivate::initFilters()
{
    filterModel.addFilter(i18n("All Widgets"),
                          KCategorizedItemsViewModels::Filter(), KIcon("plasma"));

    // Filters: Special
    filterModel.addFilter(i18n("Running"),
                          KCategorizedItemsViewModels::Filter("running", true),
                          KIcon("dialog-ok"));

    filterModel.addSeparator(i18n("Categories:"));

    typedef QPair<QString, QString> catPair;
    QMap<QString, catPair > categories;
    QSet<QString> existingCategories = itemModel.categories();
    foreach (const QString &category, Plasma::Applet::listCategories(application)) {
        const QString lowerCaseCat = category.toLower();
        if (existingCategories.contains(lowerCaseCat)) {
            const QString trans = i18n(category.toLocal8Bit());
            categories.insert(trans.toLower(), qMakePair(trans, lowerCaseCat));
        }
    }

    foreach (const catPair &category, categories) {
        filterModel.addFilter(category.first,
                              KCategorizedItemsViewModels::Filter("category", category.second));
    }

    filteringWidget->setModel(&filterModel);
    appletsListWidget->setFilterModel(&filterModel);
}

void WidgetExplorerPrivate::init(Qt::Orientation orient)
{
    //init widgets
    orientation = orient;
    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setSpacing(0);
    filteringLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    filteringWidget = new FilteringWidget(orientation, q);
    appletsListWidget = new AppletsListWidget(orientation);
    close = new Plasma::ToolButton;
    close->setIcon(KIcon("dialog-close"));

    //connect
    QObject::connect(appletsListWidget, SIGNAL(appletDoubleClicked(PlasmaAppletItem*)), q, SLOT(addApplet(PlasmaAppletItem*)));
    QObject::connect(filteringWidget->textSearch()->nativeWidget(), SIGNAL(textChanged(QString)), appletsListWidget, SLOT(searchTermChanged(QString)));
    QObject::connect(filteringWidget, SIGNAL(filterChanged(int)), appletsListWidget, SLOT(filterChanged(int)));
    QObject::connect(close, SIGNAL(clicked()), q, SIGNAL(closeClicked()));

    //adding to layout
    if (orientation == Qt::Horizontal) {
        filteringLayout->addItem(filteringWidget);
        mainLayout->addItem(filteringLayout);
    } else {
        mainLayout->addItem(filteringWidget);
    }

    mainLayout->addItem(appletsListWidget);
    appletsListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->setAlignment(appletsListWidget, Qt::AlignTop | Qt::AlignHCenter);

    if (orientation == Qt::Horizontal) {
        filteringLayout->addItem(close);
        filteringLayout->setAlignment(close, Qt::AlignVCenter | Qt::AlignHCenter);
    } else {
        mainLayout->setAlignment(filteringWidget, Qt::AlignTop | Qt::AlignHCenter);
        mainLayout->setStretchFactor(appletsListWidget, 10);
        mainLayout->addItem(close);
    }

    //filters & models
    appletsListWidget->setItemModel(&itemModel);
    initRunningApplets();

    q->setLayout(mainLayout);
}

void WidgetExplorerPrivate::setOrientation(Qt::Orientation orient)
{
    if (orientation == orient) {
        return;
    }

    orientation = orient;
    filteringWidget->setListOrientation(orientation);
    appletsListWidget->setOrientation(orientation);
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
        mainLayout->setStretchFactor(appletsListWidget, 10);
    }
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

WidgetExplorer::WidgetExplorer(Qt::Orientation orientation, QGraphicsItem *parent)
        :QGraphicsWidget(parent),
        d(new WidgetExplorerPrivate(this))
{
    d->init(orientation);
}

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

void WidgetExplorer::setOrientation(Qt::Orientation orientation)
{
    d->setOrientation(orientation);
    emit(orientationChanged(orientation));
}

Qt::Orientation WidgetExplorer::orientation()
{
    return d->orientation;
}

void WidgetExplorer::setIconSize(int size)
{
    d->appletsListWidget->setIconSize(size);
    adjustSize();
}

int WidgetExplorer::iconSize() const
{
    return d->appletsListWidget->iconSize();
}

void WidgetExplorer::populateWidgetList(const QString &app)
{
    d->application = app;
    d->itemModel.setApplication(app);
    d->initFilters();
    //d->appletsListWidget->setFilterModel(&d->filterModel);

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
            connect(d->containment, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)), this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));
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
    if (!d->containment || !appletItem) {
        return;
    }

    kDebug() << appletItem->pluginName() << appletItem->arguments();
    d->containment->addApplet(appletItem->pluginName(), appletItem->arguments());
}

void WidgetExplorer::showEvent(QShowEvent *e)
{
    d->filteringWidget->setFocus();
    QGraphicsWidget::showEvent(e);
}

void WidgetExplorer::immutabilityChanged(Plasma::ImmutabilityType type)
{
    if (type != Plasma::Mutable) {
        emit closeClicked();
    }
}

} // namespace Plasma

#include "widgetexplorer.moc"
