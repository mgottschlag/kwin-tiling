#include "widgetexplorer.h"

#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <KAction>
#include <KConfig>
#include <KConfigGroup>
#include <KMenu>
#include <KPageWidgetItem>
#include <KPushButton>
#include <KServiceTypeTrader>
#include <KStandardAction>
#include <KAboutData>
#include <KAboutApplicationDialog>
#include <KComponentData>
#include <KPluginLoader>

#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include "kcategorizeditemsviewmodels_p.h"
#include "plasmaappletitemmodel_p.h"
#include "openwidgetassistant_p.h"
#include "customwidgets.h"

using namespace KCategorizedItemsViewModels;

namespace Plasma
{

class WidgetExplorerMainWidgetPrivate
{

public:
    WidgetExplorerMainWidgetPrivate(WidgetExplorerMainWidget *w)
        : q(w),
          containment(0),
          config("plasmarc"),
          configGroup(&config, "Applet Browser"),
          itemModel(configGroup, w),
          filterModel(w)
    {
    }

    void initFilters();
    void init();
    void initPushButtonWidgetMenu();
    void initRunningApplets();
    void containmentDestroyed();

    /**
     * Tracks a new running applet
     */
    void appletAdded(Plasma::Applet *applet);

    /**
     * A running applet is no more
     */
    void appletRemoved(Plasma::Applet *applet);

    WidgetExplorerMainWidget *q;
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
    AppletsList *appletsListWidget;

    /**
     * Widget that contains the search and categories filters
     */
    FilteringWidget *filteringWidget;

};

void WidgetExplorerMainWidgetPrivate::initFilters()
{
    filterModel.clear();

    //conectar o addFilter daqui com a lista de categorias
    filterModel.addFilter(i18n("All Widgets"),
                          KCategorizedItemsViewModels::Filter(), KIcon("plasma"));

    // Filters: Special
    filterModel.addFilter(i18n("Widgets I Have Used Before"),
                          KCategorizedItemsViewModels::Filter("used", true),
                          KIcon("view-history"));
    filterModel.addFilter(i18n("Currently Running Widgets"),
                          KCategorizedItemsViewModels::Filter("running", true),
                          KIcon("view-history"));

    //conectar com algo que adiciona separador na lista de categorias
    filterModel.addSeparator(i18n("Categories:"));

    foreach (const QString &category, Plasma::Applet::listCategories(application)) {
        filterModel.addFilter(i18n(category.toLocal8Bit()),
                              KCategorizedItemsViewModels::Filter("category", category.toLower()));
    }
}

void WidgetExplorerMainWidgetPrivate::init()
{
    // **** find a fancier way to use bla ****
    QDesktopWidget *bla = new QDesktopWidget();
    q->setMinimumWidth(bla->screenGeometry(-1).width());
    q->setMaximumWidth(bla->screenGeometry(-1).width());
    q->setMinimumHeight(170);
    q->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    q->setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsLinearLayout *mainLinearLayout = new QGraphicsLinearLayout(Qt::Horizontal, 0);

    filteringWidget = new FilteringWidget();
    filteringWidget->setMaximumWidth(q->contentsRect().width()/5);
    filteringWidget->setMinimumWidth(q->contentsRect().width()/5);

    appletsListWidget = new AppletsList();

    //QObject::connect(appletsListWidget, SIGNAL(doubleClicked(const QModelIndex &)), q, SLOT(addApplet()));
    QObject::connect(appletsListWidget, SIGNAL(appletDoubleClicked(PlasmaAppletItem*)), q, SLOT(addApplet(PlasmaAppletItem*)));
    appletsListWidget->setMaximumHeight(q->contentsRect().height());
    appletsListWidget->setMinimumHeight(q->contentsRect().height());

    QObject::connect(filteringWidget->textSearch()->nativeWidget(), SIGNAL(textChanged(QString)), appletsListWidget, SLOT(searchTermChanged(QString)));

    mainLinearLayout->addItem(appletsListWidget);
    mainLinearLayout->addItem(filteringWidget);

    mainLinearLayout->setAlignment(appletsListWidget, Qt::AlignVCenter);
    mainLinearLayout->setAlignment(filteringWidget, Qt::AlignRight);
    mainLinearLayout->setContentsMargins(5, 5, 5, 5);

    initFilters();
    filteringWidget->categoriesList()->setModel(&filterModel);
    appletsListWidget->setFilterModel(&filterModel);

    QObject::connect(filteringWidget->categoriesList(), SIGNAL(filterChanged(int)), appletsListWidget, SLOT(filterChanged(int)));

    // Other models
    appletsListWidget->setItemModel(&itemModel);
    initRunningApplets();

    q->setLayout(mainLinearLayout);

}

void WidgetExplorerMainWidgetPrivate::initPushButtonWidgetMenu()
{
    pushButtonWidgetMenu = new KMenu(i18n("Get New Widgets"));
    QObject::connect(pushButtonWidgetMenu, SIGNAL(aboutToShow()), q, SLOT(populateWidgetsMenu()));
    pushButtonWidget->button()->setMenu(pushButtonWidgetMenu);

}

void WidgetExplorerMainWidgetPrivate::initRunningApplets()
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

void WidgetExplorerMainWidgetPrivate::containmentDestroyed()
{
    containment = 0;
}

void WidgetExplorerMainWidgetPrivate::appletAdded(Plasma::Applet *applet)
{
    QString name = applet->pluginName();
    //kDebug() << name;

    runningApplets[name]++;
    appletNames.insert(applet, name);
    itemModel.setRunningApplets(name, runningApplets[name]);
}

void WidgetExplorerMainWidgetPrivate::appletRemoved(Plasma::Applet *applet)
{
    //kDebug() << (QObject*)applet;
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

/* This is just a wrapper around KAboutApplicationDialog that deletes
the KAboutData object that it is associated with, when it is deleted.
This is required to free memory correctly when KAboutApplicationDialog
is called with a temporary KAboutData that is allocated on the heap.
(see the code below, in AppletBrowserWidget::infoAboutApplet())
*/
class KAboutApplicationDialog2 : public KAboutApplicationDialog
{
public:
    KAboutApplicationDialog2(KAboutData *ab, QWidget *parent = 0)
    : KAboutApplicationDialog(ab, parent), m_ab(ab) {}

    ~KAboutApplicationDialog2()
    {
        delete m_ab;
    }

private:
    KAboutData *m_ab;
};


//WidgetExplorerMainWidget

WidgetExplorerMainWidget::WidgetExplorerMainWidget(QGraphicsItem *parent)
        :QGraphicsWidget(parent),
        d(new WidgetExplorerMainWidgetPrivate(this))
{

    d->init();

    m_backgroundSvg = new Plasma::FrameSvg(this);
    m_backgroundSvg->setImagePath("widgets/translucentbackground");
}

WidgetExplorerMainWidget::~WidgetExplorerMainWidget()
{
     delete d;
}

void WidgetExplorerMainWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
 {
     QGraphicsWidget::paint(painter, option, widget);
     m_backgroundSvg->resizeFrame(contentsRect().size());
     m_backgroundSvg->paintFrame(painter, contentsRect().topLeft());
 }

void WidgetExplorerMainWidget::setApplication(const QString &app)
{
    d->application = app;
    d->initFilters();
    d->itemModel.setApplication(app);

    //FIXME: AFAIK this shouldn't be necessary ... but here it is. need to find out what in that
    //       maze of models and views is screwing up
    d->appletsListWidget->setItemModel(&d->itemModel);

    d->itemModel.setRunningApplets(d->runningApplets);
}

QString WidgetExplorerMainWidget::application()
{
    return d->application;
}

void WidgetExplorerMainWidget::setContainment(Plasma::Containment *containment)
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

Containment *WidgetExplorerMainWidget::containment() const
{
    return d->containment;
}

void WidgetExplorerMainWidget::addApplet()
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

void WidgetExplorerMainWidget::addApplet(PlasmaAppletItem *appletItem)
{
    if (!d->containment) {
        return;
    }

    kDebug() << appletItem->pluginName() << appletItem->arguments();

    d->containment->addApplet(appletItem->pluginName(), appletItem->arguments());
}

void WidgetExplorerMainWidget::destroyApplets(const QString &name)
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

void WidgetExplorerMainWidget::infoAboutApplet(const QString &name)
{
    if (!d->containment) {
        return;
    }

    KPluginInfo::List applets = Plasma::Applet::listAppletInfo();
    foreach (const KPluginInfo &info, applets) {
        if (info.pluginName() == name) {
            KAboutData *aboutData = new KAboutData(info.name().toUtf8(),
                                              info.name().toUtf8(),
                                              ki18n(info.name().toUtf8()),
                                              info.version().toUtf8(), ki18n(info.comment().toUtf8()),
                                              info.fullLicense().key(), ki18n(QByteArray()), ki18n(QByteArray()), info.website().toLatin1(),
                                              info.email().toLatin1());

            aboutData->setProgramIconName(info.icon());

            aboutData->addAuthor(ki18n(info.author().toUtf8()), ki18n(QByteArray()), info.email().toLatin1());

            //TODO should recycle this dialog if it is called up twice
            //KAboutApplication nao recebe QGraphicsWidget como parent, entao coloquei 0
            //KAboutApplicationDialog *aboutDialog = new KAboutApplicationDialog2(aboutData, this);
            KAboutApplicationDialog *aboutDialog = new KAboutApplicationDialog2(aboutData, 0);
            aboutDialog->show();
            break;
        }
    }
}

void WidgetExplorerMainWidget::downloadWidgets(const QString &type)
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

void WidgetExplorerMainWidget::openWidgetFile()
{
    // TODO: if we already have one of these showing and the user clicks to
    // add it again, show the same window?
    OpenWidgetAssistant *assistant = new OpenWidgetAssistant(0);
    assistant->setAttribute(Qt::WA_DeleteOnClose, true);
    assistant->show();
}

void WidgetExplorerMainWidget::populateWidgetsMenu()
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

//WidgetExplorer

WidgetExplorer::WidgetExplorer(QWidget *parent, Qt::WindowFlags f)
{
    Q_UNUSED(parent)
    Q_UNUSED(f)
    init();
}

WidgetExplorer::~WidgetExplorer()
{
//    KConfigGroup cg(KGlobal::config(), "PlasmaAppletBrowserDialog");
//    saveDialogSize(cg);
}

void WidgetExplorer::init()
{
    m_widget = new WidgetExplorerMainWidget();
    addItem(m_widget);
}

void WidgetExplorer::setApplication(const QString &app)
{
    m_widget->setApplication(app);
}

QString WidgetExplorer::application()
{
    return m_widget->application();
}

void WidgetExplorer::setContainment(Plasma::Containment *containment)
{
    m_widget->setContainment(containment);
}

Containment *WidgetExplorer::containment() const
{
    return m_widget->containment();
}

} // namespace Plasma

#include "widgetexplorer.moc"
