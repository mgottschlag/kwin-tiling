/*
 *   Copyright 2009 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 by Artur Duque de Souza <asouza@kde.org>
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2,
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

#include "sal.h"
#include "stripwidget.h"
#include "itemview.h"
#include "runnersconfig.h"
#include "../common/linearappletoverlay.h"
#include "../common/appletmovespacer.h"
#include "iconactioncollection.h"
#include "models/commonmodel.h"
#include "models/kservicemodel.h"

#include <QAction>
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsAnchorLayout>
#include <QGraphicsView>
#include <QListView>
#include <QVBoxLayout>

#include <KAction>
#include <KDebug>
#include <KIcon>
#include <KIconLoader>
#include <KService>
#include <KConfigDialog>
#include <KRun>
#include <KLineEdit>
#include <KKeySequenceWidget>

#include <Plasma/AbstractToolBox>
#include <Plasma/Theme>
#include <Plasma/Frame>
#include <Plasma/Corona>
#include <Plasma/LineEdit>
#include <Plasma/IconWidget>
#include <Plasma/RunnerManager>
#include <Plasma/ScrollWidget>
#include <Plasma/ToolButton>


SearchLaunch::SearchLaunch(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_runnerModel(0),
      m_backButton(0),
      m_queryCounter(0),
      m_maxColumnWidth(0),
      m_searchField(0),
      m_resultsView(0),
      m_orientation(Qt::Vertical),
      m_firstItem(0),
      m_appletsLayout(0),
      m_appletOverlay(0),
      m_iconActionCollection(0),
      m_stripUninitialized(true)
{
    setContainmentType(Containment::CustomContainment);
    m_iconActionCollection = new IconActionCollection(this, this);
    setHasConfigurationInterface(true);
    setFocusPolicy(Qt::StrongFocus);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/frame");
    m_background->setElementPrefix("raised");
    m_background->setEnabledBorders(Plasma::FrameSvg::BottomBorder);
}

SearchLaunch::~SearchLaunch()
{
    KConfigGroup cg = config();
    m_stripWidget->save(cg);
    config().writeEntry("orientation", (int)m_orientation);
}

void SearchLaunch::init()
{
    Containment::init();
    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));

    connect(this, SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));


    setToolBox(Plasma::AbstractToolBox::load(corona()->preferredToolBoxPlugin(Plasma::Containment::DesktopContainment), QVariantList(), this));

    QAction *a = action("add widgets");
    if (a) {
        addToolBoxAction(a);
    }


    if (toolBox()) {
        connect(toolBox(), SIGNAL(toggled()), this, SIGNAL(toolBoxToggled()));
        connect(toolBox(), SIGNAL(visibilityChanged(bool)), this, SIGNAL(toolBoxVisibilityChanged(bool)));
        toolBox()->show();
    }

    a = action("configure");
    if (a) {
        addToolBoxAction(a);
        a->setText(i18n("Configure Search and Launch"));
    }


    QAction *lockAction = 0;
    if (corona()) {
        lockAction = corona()->action("lock widgets");
    }

    if (!lockAction || !lockAction->isEnabled()) {
        lockAction = new QAction(this);
        addAction("lock page", lockAction);
        lockAction->setText(i18n("Lock Page"));
        lockAction->setIcon(KIcon("object-locked"));
        QObject::connect(lockAction, SIGNAL(triggered(bool)), this, SLOT(toggleImmutability()));
    }

    addToolBoxAction(lockAction);

    //FIXME: two different use cases for the desktop and the newspaper, another reason to move the toolbox management out of here
    QAction *activityAction = 0;
    if (corona()) {
        activityAction = corona()->action("manage activities");
    }
    if (activityAction) {
        addToolBoxAction(activityAction);
    }

    a = new QAction(i18n("Next activity"), this);
    addAction("next containment", a);
    a = new QAction(i18n("Previous activity"), this);
    addAction("previous containment", a);

    if (corona()) {
        connect(corona(), SIGNAL(availableScreenRegionChanged()), this, SLOT(availableScreenRegionChanged()));
        availableScreenRegionChanged();
    }

    Plasma::FormFactor form = formFactor();
    Qt::Orientation layoutOtherDirection = form == Plasma::Vertical ? \
            Qt::Horizontal : Qt::Vertical;

    // create main layout
    m_mainLayout = new QGraphicsLinearLayout();
    m_mainLayout->setOrientation(layoutOtherDirection);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                            QSizePolicy::Expanding));
    setLayout(m_mainLayout);

    // create launch grid and make it centered
    m_resultsLayout = new QGraphicsLinearLayout;


    m_resultsView = new ItemView(this);

    m_resultsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_resultsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_resultsLayout->addItem(m_resultsView);

    connect(m_resultsView, SIGNAL(dragStartRequested(QModelIndex)), this, SLOT(resultsViewRequestedDrag(QModelIndex)));
    connect(m_resultsView, SIGNAL(itemActivated(QModelIndex)), this, SLOT(launch(QModelIndex)));
    connect(m_resultsView, SIGNAL(addActionTriggered(QModelIndex)), this, SLOT(addFavourite(QModelIndex)));


    //TODO how to do the strip widget?
    m_stripWidget = new StripWidget(this);
    m_stripWidget->setImmutability(immutability());
    connect(m_stripWidget, SIGNAL(saveNeeded()), this, SLOT(saveFavourites()));

    //load all config, only at this point we are sure it won't crash
    configChanged();

    m_appletsLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    m_appletsLayout->setPreferredHeight(KIconLoader::SizeMedium);
    m_appletsLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QGraphicsWidget *leftSpacer = new QGraphicsWidget(this);
    QGraphicsWidget *rightSpacer = new QGraphicsWidget(this);
    m_appletsLayout->addItem(leftSpacer);
    m_appletsLayout->addItem(rightSpacer);

    m_backButton = new Plasma::IconWidget(this);
    m_backButton->setIcon(KIcon("go-previous"));
    m_backButton->setText(i18n("Back"));
    m_backButton->setOrientation(Qt::Horizontal);
    m_backButton->setPreferredSize(m_backButton->sizeFromIconSize(KIconLoader::SizeSmallMedium));
    connect(m_backButton, SIGNAL(clicked()), this, SLOT(reset()));
    connect(m_resultsView, SIGNAL(resetRequested()), this, SLOT(reset()));

    QGraphicsAnchorLayout *searchLayout = new QGraphicsAnchorLayout();
    searchLayout->setSpacing(10);

    m_searchField = new Plasma::LineEdit(this);
    m_searchField->setPreferredWidth(200);
    m_searchField->nativeWidget()->setClearButtonShown(true);
    m_searchField->nativeWidget()->setClickMessage(i18n("Search..."));
    connect(m_searchField, SIGNAL(returnPressed()), this, SLOT(searchReturnPressed()));
    connect(m_searchField->nativeWidget(), SIGNAL(textEdited(QString)), this, SLOT(delayedQuery()));
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    connect(m_searchTimer, SIGNAL(timeout()), this, SLOT(query()));
    searchLayout->addAnchor(m_searchField, Qt::AnchorHorizontalCenter, searchLayout, Qt::AnchorHorizontalCenter);
    searchLayout->addAnchors(m_searchField, searchLayout, Qt::Vertical);
    searchLayout->addAnchors(m_backButton, searchLayout, Qt::Vertical);
    searchLayout->addAnchor(m_backButton, Qt::AnchorRight, m_searchField, Qt::AnchorLeft);


    // add our layouts to main vertical layout
    m_mainLayout->addItem(m_stripWidget);
    m_mainLayout->addItem(searchLayout);
    m_mainLayout->addItem(m_resultsLayout);


    // correctly set margins
    m_mainLayout->activate();
    m_mainLayout->updateGeometry();

    setTabOrder(m_stripWidget, m_searchField);
    setTabOrder(m_searchField, m_resultsView);
    setFormFactorFromLocation(location());

    if (action("remove")) {
        addToolBoxAction(action("remove"));
    }
}

void SearchLaunch::launchPackageManager()
{
    if (toolBox()) {
        toolBox()->setShowing(false);
    }
    KRun::run(*m_packageManagerService.data(), KUrl::List(), 0);
}

void SearchLaunch::configChanged()
{
    setOrientation((Qt::Orientation)config().readEntry("Orientation", (int)Qt::Vertical));

    m_stripWidget->setIconSize(config().readEntry("FavouritesIconSize", (int)KIconLoader::SizeLarge));

    m_resultsView->setIconSize(config().readEntry("ResultsIconSize", (int)KIconLoader::SizeHuge));

    QString packageManagerName = config().readEntry("PackageManager", "kpackagekit");

    if (!packageManagerName.isEmpty()) {
        m_packageManagerService = KService::serviceByDesktopName(packageManagerName);

        if (!action("add applications") && m_packageManagerService && !m_packageManagerService->exec().isEmpty()) {
            KAction *addApplicationsAction = new KAction(this);
            addAction("add applications", addApplicationsAction);
            addApplicationsAction->setText(i18n("Add applications"));
            addApplicationsAction->setIcon(KIcon("applications-other"));
            addToolBoxAction(addApplicationsAction);

            connect(addApplicationsAction, SIGNAL(triggered()), this, SLOT(launchPackageManager()));
        }
    }
}

void SearchLaunch::availableScreenRegionChanged()
{
    if (!corona()) {
        return;
    }

    QRect maxRect;
    int maxArea = 0;
    //we don't want the bounding rect (that could include panels too), but the maximumone representing the desktop
    foreach (const QRect &rect, corona()->availableScreenRegion(screen()).rects()) {
        int area = rect.width() * rect.height();
        if (area > maxArea) {
            maxRect = rect;
            maxArea = area;
        }
    }

    QGraphicsView *ownView = view();

    //FIXME: the second check is a workaround to a qt bug: when a qwidget has just been created, maptoglobal and mapfromglobal aren't symmetryc. remove as soon as the bug is fixed
    if (ownView && (ownView->mapFromGlobal(QPoint(0,0)) == -ownView->mapToGlobal(QPoint(0,0)))) {
        maxRect.moveTopLeft(ownView->mapFromGlobal(maxRect.topLeft()));
    }

    maxRect.moveTopLeft(QPoint(qMax(0, maxRect.left()), qMax(0, maxRect.top())));

    setContentsMargins(maxRect.left(), maxRect.top(), qMax((qreal)0.0, size().width() - maxRect.right()), qMax((qreal)0.0, size().height() - maxRect.bottom()));
}

void SearchLaunch::toggleImmutability()
{
    if (immutability() == Plasma::UserImmutable) {
        setImmutability(Plasma::Mutable);
    } else if (immutability() == Plasma::Mutable) {
        setImmutability(Plasma::UserImmutable);
    }
}

void SearchLaunch::doSearch(const QString &query, const QString &runner)
{
    if (!query.isEmpty()) {
        m_resultsView->setModel(m_runnerModel);
    } else {
        m_resultsView->setModel(m_serviceModel);
        m_serviceModel->setPath("/");
    }

    m_runnerModel->setQuery(query, runner);
    m_lastQuery = query;
    //enable or disable drag and drop
    if (immutability() == Plasma::Mutable && (m_resultsView->model() != m_serviceModel || m_serviceModel->path() != "/")) {
        m_resultsView->setDragAndDropMode(ItemContainer::CopyDragAndDrop);
    } else {
        m_resultsView->setDragAndDropMode(ItemContainer::NoDragAndDrop);
    }
}

void SearchLaunch::reset()
{
    if (m_resultsView->model() != m_serviceModel || m_serviceModel->path() != "/") {
        m_searchField->setText(QString());
        doSearch(QString());
        m_serviceModel->setPath("/");
        m_resultsView->setModel(m_serviceModel);
    }
}

void SearchLaunch::launch(QModelIndex index)
{
    KUrl url(index.data(CommonModel::Url).value<QString>());

    if (m_resultsView->model() == m_runnerModel) {
        KRunnerItemHandler::openUrl(url);
        reset();
        emit releaseVisualFocus();
    } else {
        QString id = url.path();
        if (id.startsWith(QLatin1Char('/'))) {
            id = id.remove(0, 1);
        }
        if (url.protocol() == "kservicegroup") {
            m_serviceModel->setPath(id);
        } else if (url.protocol() == "krunner") {
            m_resultsView->setModel(m_runnerModel);
            m_runnerModel->setQuery(id, url.host());
        } else {
            KServiceItemHandler::openUrl(url);
            reset();
            emit releaseVisualFocus();
        }
    }

    //enable or disable drag and drop
    if (immutability() == Plasma::Mutable && (m_resultsView->model() != m_serviceModel || m_serviceModel->path() != "/")) {
        m_resultsView->setDragAndDropMode(ItemContainer::CopyDragAndDrop);
    } else {
        m_resultsView->setDragAndDropMode(ItemContainer::NoDragAndDrop);
    }
}

void SearchLaunch::addFavourite(const QModelIndex &index)
{
    QMimeData *mimeData = m_resultsView->model()->mimeData(QModelIndexList()<<index);
    if (mimeData && !mimeData->urls().isEmpty()) {
        m_stripWidget->add(mimeData->urls().first());
    }
}

void SearchLaunch::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    Q_UNUSED(pos);

    if (!m_appletsLayout) {
        return;
    }

    if (m_appletsLayout->count() == 2) {
        m_mainLayout->removeItem(m_appletsLayout);
        m_mainLayout->addItem(m_appletsLayout);
    }

    Plasma::FormFactor f = formFactor();
    int insertIndex = -1;

    //if pos is (-1,-1) insert at the end of the panel
    if (pos != QPoint(-1, -1)) {
        for (int i = 1; i < m_appletsLayout->count()-1; ++i) {
            if (!dynamic_cast<Plasma::Applet *>(m_appletsLayout->itemAt(i)) &&
                !dynamic_cast<AppletMoveSpacer *>(m_appletsLayout->itemAt(i))) {
                continue;
            }

            QRectF siblingGeometry = m_appletsLayout->itemAt(i)->geometry();
            if (f == Plasma::Horizontal) {
                qreal middle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
                if (pos.x() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.x() <= siblingGeometry.right()) {
                    insertIndex = i + 1;
                    break;
                }
            } else { // Plasma::Vertical
                qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
                if (pos.y() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.y() <= siblingGeometry.bottom()) {
                    insertIndex = i + 1;
                    break;
                }
            }
        }
    }

    if (insertIndex != -1) {
        m_appletsLayout->insertItem(insertIndex, applet);
    } else {
        m_appletsLayout->insertItem(m_appletsLayout->count()-1, applet);
    }

    applet->setBackgroundHints(NoBackground);
}

void SearchLaunch::appletRemoved(Plasma::Applet* applet)
{
    Q_UNUSED(applet)

    if (!m_appletOverlay && m_appletsLayout->count() == 3) {
        m_mainLayout->removeItem(m_appletsLayout);
    }
}

void SearchLaunch::constraintsEvent(Plasma::Constraints constraints)
{
    //kDebug() << "constraints updated with" << constraints << "!!!!!!";

    if (constraints & Plasma::FormFactorConstraint ||
        constraints & Plasma::StartupCompletedConstraint) {

         // create the models
        if (!m_runnerModel) {
            m_runnerModel = new KRunnerModel(this);
            m_serviceModel = new KServiceModel(config(), this);
            m_resultsView->setModel(m_serviceModel);
        }

        resize(corona()->screenGeometry(screen()).size());
    }

    if (constraints & Plasma::LocationConstraint) {
        setFormFactorFromLocation(location());
    }

    if (constraints & Plasma::SizeConstraint) {
        availableScreenRegionChanged();
        if (m_appletsLayout) {
            m_appletsLayout->setMaximumHeight(size().height()/4);
        }
        if (m_appletOverlay) {
            m_appletOverlay->resize(size());
        }
    }

    if (constraints & Plasma::StartupCompletedConstraint) {
        Plasma::DataEngine *engine = dataEngine("searchlaunch");
        engine->connectSource("query", this);
    }

    if (constraints & Plasma::ScreenConstraint) {
        if (screen() != -1 && m_searchField) {
            m_searchField->setFocus();
        }
    }

    if (constraints & Plasma::ImmutableConstraint) {
        //update lock button
        QAction *a = action("lock page");
        if (a) {
            switch (immutability()) {
                case Plasma::SystemImmutable:
                    a->setEnabled(false);
                    a->setVisible(false);
                    break;

                case Plasma::UserImmutable:
                    a->setText(i18n("Unlock Page"));
                    a->setIcon(KIcon("object-unlocked"));
                    a->setEnabled(true);
                    a->setVisible(true);
                    break;

                case Plasma::Mutable:
                    a->setText(i18n("Lock Page"));
                    a->setIcon(KIcon("object-locked"));
                    a->setEnabled(true);
                    a->setVisible(true);
                    break;
            }
        }

        //kill or create the config overlay if needed
        if (immutability() == Plasma::Mutable && !m_appletOverlay && toolBox() && toolBox()->isShowing()) {
            m_appletOverlay = new LinearAppletOverlay(this, m_appletsLayout);
            m_appletOverlay->resize(size());
        } else if (immutability() != Plasma::Mutable && m_appletOverlay && toolBox() && toolBox()->isShowing()) {
            m_appletOverlay->deleteLater();
            m_appletOverlay = 0;
        }

        //enable or disable drag and drop
        if (immutability() == Plasma::Mutable && (m_resultsView->model() != m_serviceModel || m_serviceModel->path() != "/")) {
            m_resultsView->setDragAndDropMode(ItemContainer::CopyDragAndDrop);
        } else {
            m_resultsView->setDragAndDropMode(ItemContainer::NoDragAndDrop);
        }
        m_stripWidget->setImmutability(immutability());
    }
}

void SearchLaunch::setOrientation(Qt::Orientation orientation)
{
    if (m_orientation == orientation) {
        return;
    }

    m_orientation = orientation;
    m_resultsView->setOrientation(orientation);
}

void SearchLaunch::setFormFactorFromLocation(Plasma::Location loc)
{
    switch (loc) {
    case Plasma::BottomEdge:
    case Plasma::TopEdge:
        //kDebug() << "setting horizontal form factor";
        setFormFactor(Plasma::Horizontal);
        break;
    case Plasma::RightEdge:
    case Plasma::LeftEdge:
        //kDebug() << "setting vertical form factor";
        setFormFactor(Plasma::Vertical);
        break;
    default:
        setFormFactor(Plasma::Horizontal);
    }
}

void SearchLaunch::restoreStrip()
{
    KConfigGroup cg = config();
    m_stripWidget->restore(cg);
    reset();
}

void SearchLaunch::updateConfigurationMode(bool config)
{

    if (config && !m_appletOverlay && immutability() == Plasma::Mutable) {
        if (m_appletsLayout->count() == 2) {
            m_mainLayout->addItem(m_appletsLayout);
        }
        m_appletOverlay = new LinearAppletOverlay(this, m_appletsLayout);
        m_appletOverlay->resize(size());
        connect (m_appletOverlay, SIGNAL(dropRequested(QGraphicsSceneDragDropEvent*)),
                 this, SLOT(overlayRequestedDrop(QGraphicsSceneDragDropEvent*)));
    } else if (!config) {
        delete m_appletOverlay;
        m_appletOverlay = 0;
        if (m_appletsLayout->count() == 2) {
            m_mainLayout->removeItem(m_appletsLayout);
        }
    }
}

void SearchLaunch::overlayRequestedDrop(QGraphicsSceneDragDropEvent *event)
{
    dropEvent(event);
}


void SearchLaunch::resultsViewRequestedDrag(QModelIndex index)
{
    if (!m_resultsView->model()) {
        return;
    }


    QModelIndexList list;
    list.append(index);
    QMimeData *mimeData = m_resultsView->model()->mimeData(list);

    QDrag *drag = new QDrag(view());
    drag->setMimeData(mimeData);
    drag->setPixmap(index.data(Qt::DecorationRole).value<QIcon>().pixmap(KIconLoader::SizeHuge, KIconLoader::SizeHuge));

    drag->exec(Qt::CopyAction);
}

void SearchLaunch::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *, const QRect &)
{
    if (m_stripUninitialized) {
        m_stripUninitialized = false;
        QTimer::singleShot(100, this, SLOT(restoreStrip()));
    } else {
        m_background->resizeFrame(QSizeF(size().width(), m_stripWidget->geometry().bottom()));
        m_background->paintFrame(painter);
    }
}

void SearchLaunch::delayedQuery()
{
    m_searchTimer->start(500);
}

void SearchLaunch::query()
{
    QString query = m_searchField->text();
    doSearch(query);
    m_lastQuery = query;
}

void SearchLaunch::searchReturnPressed()
{
    QString query = m_searchField->text();
    //by pressing enter  do a query or
    if (query == m_lastQuery && !query.isEmpty()) {
        launch(m_resultsView->model()->index(0, 0, QModelIndex()));
        reset();
    } else {
        doSearch(query);
        m_lastQuery = query;
    }
}

void SearchLaunch::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(sourceName);

    const QString query(data["query"].toString());
    //Take ownership of the screen if we don't have one
    //FIXME: hardcoding 0 is bad: maybe pass the screen from the dataengine?
    if (!query.isEmpty()) {
        if (screen() < 0) {
            setScreen(0);
        }
        emit activate();
    }

    doSearch(query);
    if (m_searchField) {
        m_searchField->setText(query);
    }
}

void SearchLaunch::focusInEvent(QFocusEvent *event)
{
    if (m_searchField) {
        m_searchField->setFocus();
    }
    if (screen() < 0) {
        setScreen(0);
    }
    Containment::focusInEvent(event);
}

void SearchLaunch::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ContentsRectChange) {

        if (toolBox() && toolBox()->isShowing()) {
            updateConfigurationMode(true);
        }
    }
    Plasma::Containment::changeEvent(event);
}

void SearchLaunch::createConfigurationInterface(KConfigDialog *parent)
{
    RunnersConfig *runnersConfig = new RunnersConfig(m_runnerModel->runnerManager(), parent);
    parent->addPage(runnersConfig, i18nc("Title of the page that lets the user choose the loaded krunner plugins", "Search plugins"), "edit-find");

    connect(parent, SIGNAL(applyClicked()), runnersConfig, SLOT(accept()));
    connect(parent, SIGNAL(okClicked()), runnersConfig, SLOT(accept()));

    QListView *enabledEntries = new QListView(parent);
    enabledEntries->setModel(m_serviceModel->allRootEntriesModel());
    enabledEntries->setModelColumn(0);
    parent->addPage(enabledEntries, i18nc("Title of the page that lets the user choose what entries will be allowed in the main menu", "Main menu"), "view-list-icons");

    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    if (!m_shortcutEditor) {
        m_shortcutEditor = new KKeySequenceWidget(page);
        connect(parent, SIGNAL(applyClicked()), this, SLOT(configDialogFinished()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configDialogFinished()));
    }

    m_shortcutEditor.data()->setKeySequence(globalShortcut().primary());
    layout->addWidget(m_shortcutEditor.data());
    layout->addStretch();
    parent->addPage(page, i18n("Keyboard Shortcut"), "preferences-desktop-keyboard");

    connect(parent, SIGNAL(applyClicked()), m_serviceModel, SLOT(saveConfig()));
    connect(parent, SIGNAL(okClicked()), m_serviceModel, SLOT(saveConfig()));
}

void SearchLaunch::configDialogFinished()
{
    if (m_shortcutEditor) {
        QKeySequence sequence = m_shortcutEditor.data()->keySequence();
        if (sequence != globalShortcut().primary()) {
            setGlobalShortcut(KShortcut(sequence));
            emit configNeedsSaving();
        }
    }
}

void SearchLaunch::saveFavourites()
{
    KConfigGroup cg = config();
    m_stripWidget->save(cg);
}

K_EXPORT_PLASMA_APPLET(sal, SearchLaunch)

#include "sal.moc"

