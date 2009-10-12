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
#include "../common/linearappletoverlay.h"
#include "../common/appletmovespacer.h"
#include "../common/nettoolbox.h"

#include <QAction>
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsAnchorLayout>
#include <QApplication>

#include <KAction>
#include <KDebug>
#include <KIcon>
#include <KIconLoader>
#include <KLineEdit>
#include <KStandardDirs>

#include <Plasma/Theme>
#include <Plasma/Frame>
#include <Plasma/Corona>
#include <Plasma/LineEdit>
#include <Plasma/IconWidget>
#include <Plasma/RunnerManager>
#include <Plasma/QueryMatch>
#include <Plasma/ScrollWidget>


SearchLaunch::SearchLaunch(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_homeButton(0),
      m_queryCounter(0),
      m_maxColumnWidth(0),
      m_searchField(0),
      m_resultsView(0),
      m_appletsLayout(0),
      m_appletOverlay(0),
      m_stripUninitialized(true)
{
    setContainmentType(Containment::CustomContainment);
    setFocusPolicy(Qt::StrongFocus);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
}

SearchLaunch::~SearchLaunch()
{
    KConfigGroup cg = config();
    m_stripWidget->save(cg);

    delete m_runnermg;
}

void SearchLaunch::init()
{
    Containment::init();
    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));

    connect(this, SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));

    m_runnermg = new Plasma::RunnerManager(this);
    connect(m_runnermg, SIGNAL(matchesChanged(const QList<Plasma::QueryMatch>&)),
            this, SLOT(setQueryMatches(const QList<Plasma::QueryMatch>&)));

    m_toolBox = new NetToolBox(this);
    connect(m_toolBox, SIGNAL(toggled()), this, SIGNAL(toolBoxToggled()));
    connect(m_toolBox, SIGNAL(visibilityChanged(bool)), this, SIGNAL(toolBoxVisibilityChanged(bool)));
    m_toolBox->show();

    QAction *a = action("add widgets");
    if (a) {
        m_toolBox->addTool(a);
    }

    a = action("configure");
    if (a) {
        m_toolBox->addTool(a);
        a->setText(i18n("Configure Search and Launch"));
    }

    KAction *lockAction = new KAction(this);
    addAction("lock page", lockAction);
    lockAction->setText(i18n("Lock Page"));
    lockAction->setIcon(KIcon("object-locked"));
    QObject::connect(lockAction, SIGNAL(triggered(bool)), this, SLOT(toggleImmutability()));
    m_toolBox->addTool(lockAction);

    QString defaultMatchesConfig = KStandardDirs::locate("appdata", "defaultmatchesrc");
    KSharedPtr<KSharedConfig> config = KSharedConfig::openConfig(defaultMatchesConfig);
    KConfigGroup iconsGroup(config, "Matches");

    foreach (const QString &group, iconsGroup.groupList()) {
        KConfigGroup iconConfig(&iconsGroup, group);

        Plasma::QueryMatch match(0);
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setRelevance(1.0/(qreal)group.toInt());

        match.setId(iconConfig.readEntry("Id"));
        match.setIcon(KIcon(iconConfig.readEntry("Icon")));
        match.setText(iconConfig.readEntry("Name"));
        match.setData(iconConfig.readEntry("system"));

        m_defaultMatches.append(match);
    }
}

void SearchLaunch::toggleImmutability()
{
    if (immutability() == Plasma::UserImmutable) {
        setImmutability(Plasma::Mutable);
    } else if (immutability() == Plasma::Mutable) {
        setImmutability(Plasma::UserImmutable);
    }
}

void SearchLaunch::doSearch(const QString query)
{
    m_queryCounter = 0;
    m_resultsView->clear();

    const bool stillEmpty = query.isEmpty() && m_runnermg->query().isEmpty();
    if (!stillEmpty) {
        m_resultsView->clear();
    }

    m_maxColumnWidth = 0;
    m_matches.clear();
    m_runnermg->launchQuery(query);

    if (m_resultsView && query.isEmpty()) {
        if (stillEmpty && m_resultsView->count()) {
            m_resultsView->clear();
            return;
        }

        setQueryMatches(m_defaultMatches);
        m_homeButton->hide();
    } else if (m_homeButton) {
        m_homeButton->show();
    }
}

void SearchLaunch::reset()
{
    doSearch(QString());
}

void SearchLaunch::setQueryMatches(const QList<Plasma::QueryMatch> &m)
{
    if (m.isEmpty()) {
        return;
    }

    // just add new QueryMatch
    int i;
    for (i = m_queryCounter; i < m.size(); i++) {
        Plasma::QueryMatch match = m[i];

        // create new IconWidget with information from the match
        Plasma::IconWidget *icon = new Plasma::IconWidget(m_resultsView);
        icon->hide();
        icon->setText(match.text());
        icon->setIcon(match.icon());

        m_resultsView->insertItem(icon, 1/match.relevance());

        connect(icon, SIGNAL(activated()), this, SLOT(launch()));

        if (!m_runnermg->searchContext()->query().isEmpty()) {
            // create action to add to favourites strip
            QAction *action = new QAction(icon);
            action->setIcon(KIcon("favorites"));
            icon->addIconAction(action);
            connect(action, SIGNAL(triggered()), this, SLOT(addFavourite()));
        }

        // add to data structure
        m_matches.insert(icon, match);
    }
    m_queryCounter = i;
}



void SearchLaunch::launch(Plasma::IconWidget *icon)
{
    Plasma::QueryMatch match = m_matches.value(icon, Plasma::QueryMatch(0));
    if (m_runnermg->searchContext()->query().isEmpty()) {
        doSearch(match.data().toString());
    } else {
        m_runnermg->run(match);
    }
}

void SearchLaunch::launch()
{
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(sender());
    launch(icon);
}

void SearchLaunch::addFavourite()
{
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(sender()->parent());
    Plasma::QueryMatch match = m_matches.value(icon, Plasma::QueryMatch(0));
    m_stripWidget->add(match, m_runnermg->searchContext()->query());
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
    connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
}

void SearchLaunch::appletRemoved(Plasma::Applet* applet)
{
    Q_UNUSED(applet)

    if (!m_appletOverlay && m_appletsLayout->count() == 3) {
        m_mainLayout->removeItem(m_appletsLayout);
    }
}

void SearchLaunch::updateSize()
{
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(sender());

    if (applet) {
        if (formFactor() == Plasma::Horizontal) {
            const int delta = applet->preferredWidth() - applet->size().width();
            //setting the preferred width when delta = 0 and preferredWidth() < minimumWidth()
            // leads to the same thing as setPreferredWidth(minimumWidth())
            if (delta != 0) {
                setPreferredWidth(preferredWidth() + delta);
            }
        } else if (formFactor() == Plasma::Vertical) {
            const int delta = applet->preferredHeight() - applet->size().height();
            if (delta != 0) {
                setPreferredHeight(preferredHeight() + delta);
            }
        }

        resize(preferredSize());
    }
}


void SearchLaunch::constraintsEvent(Plasma::Constraints constraints)
{
    kDebug() << "constraints updated with" << constraints << "!!!!!!";

    if (constraints & Plasma::FormFactorConstraint ||
        constraints & Plasma::StartupCompletedConstraint) {

        Plasma::FormFactor form = formFactor();
        Qt::Orientation layoutDirection = form == Plasma::Vertical ? \
            Qt::Vertical : Qt::Horizontal;
        Qt::Orientation layoutOtherDirection = form == Plasma::Vertical ? \
            Qt::Horizontal : Qt::Vertical;

        // create our layout!
        if (!layout()) {
            // create main layout
            m_mainLayout = new QGraphicsLinearLayout();
            m_mainLayout->setOrientation(layoutOtherDirection);
            m_mainLayout->setContentsMargins(0, 0, 0, 0);
            m_mainLayout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                                    QSizePolicy::Expanding));
            setLayout(m_mainLayout);

            // create launch grid and make it centered
            QGraphicsLinearLayout *gridLayout = new QGraphicsLinearLayout(Qt::Vertical);


            m_resultsView = new ItemView(this);
            m_resultsView->setOrientation(Qt::Vertical);
            m_resultsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            m_resultsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            gridLayout->addItem(m_resultsView);

            connect(m_resultsView, SIGNAL(itemSelected(Plasma::IconWidget *)), this, SLOT(selectItem(Plasma::IconWidget *)));
            connect(m_resultsView, SIGNAL(itemActivated(Plasma::IconWidget *)), this, SLOT(launch(Plasma::IconWidget *)));

            m_stripWidget = new StripWidget(m_runnermg, this);
            m_appletsLayout = new QGraphicsLinearLayout();
            m_appletsLayout->setPreferredHeight(KIconLoader::SizeMedium);
            m_appletsLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            QGraphicsWidget *leftSpacer = new QGraphicsWidget(this);
            QGraphicsWidget *rightSpacer = new QGraphicsWidget(this);
            m_appletsLayout->addItem(leftSpacer);
            m_appletsLayout->addItem(rightSpacer);

            m_homeButton = new Plasma::IconWidget(this);
            m_homeButton->setIcon(KIcon("go-home"));
            m_homeButton->setText(i18n("Home"));
            m_homeButton->setOrientation(Qt::Horizontal);
            m_homeButton->setPreferredSize(m_homeButton->sizeFromIconSize(KIconLoader::SizeSmall));
            connect(m_homeButton, SIGNAL(activated()), this, SLOT(reset()));
            connect(m_resultsView, SIGNAL(resetRequested()), this, SLOT(reset()));

            QGraphicsAnchorLayout *searchLayout = new QGraphicsAnchorLayout();
            searchLayout->setSpacing(5);

            m_searchField = new Plasma::LineEdit(this);
            m_searchField->setPreferredWidth(200);
            m_searchField->nativeWidget()->setClearButtonShown(true);
            m_searchField->nativeWidget()->setClickMessage(i18n("Enter your query here"));
            connect(m_searchField, SIGNAL(returnPressed()), this, SLOT(query()));
            connect(m_searchField->nativeWidget(), SIGNAL(textEdited(const QString &)), this, SLOT(delayedQuery()));
            m_searchTimer = new QTimer(this);
            m_searchTimer->setSingleShot(true);
            connect(m_searchTimer, SIGNAL(timeout()), this, SLOT(query()));
            searchLayout->addAnchor(m_searchField, Qt::AnchorHorizontalCenter, searchLayout, Qt::AnchorHorizontalCenter);
            searchLayout->addAnchors(m_searchField, searchLayout, Qt::Vertical);
            searchLayout->addAnchors(m_homeButton, searchLayout, Qt::Vertical);
            searchLayout->addAnchor(m_homeButton, Qt::AnchorRight, m_searchField, Qt::AnchorLeft);


            // add our layouts to main vertical layout
            m_mainLayout->addItem(m_stripWidget);
            m_mainLayout->addItem(searchLayout);
            m_mainLayout->addItem(gridLayout);


            // correctly set margins
            m_mainLayout->activate();
            m_mainLayout->updateGeometry();

            setTabOrder(m_stripWidget, m_searchField);
            setTabOrder(m_searchField, m_resultsView);
        }
    }

    if (constraints & Plasma::LocationConstraint) {
        setFormFactorFromLocation(location());
    }

    if (constraints & Plasma::SizeConstraint) {
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

        if (immutability() == Plasma::Mutable && !m_appletOverlay && m_toolBox->isShowing()) {
            m_appletOverlay = new LinearAppletOverlay(this, m_appletsLayout);
            m_appletOverlay->resize(size());
        } else if (immutability() != Plasma::Mutable && m_appletOverlay && m_toolBox->isShowing()) {
            m_appletOverlay->deleteLater();
            m_appletOverlay = 0;
        }
    }
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
    qreal extraLeft = 0;
    qreal extraTop = 0;
    qreal extraRight = 0;
    qreal extraBottom = 0;

    switch (m_toolBox->location()) {
        case Plasma::LeftEdge:
        extraLeft= m_toolBox->expandedGeometry().width();
        break;
    case Plasma::RightEdge:
        extraRight = m_toolBox->expandedGeometry().width();
        break;
    case Plasma::TopEdge:
        extraTop = m_toolBox->expandedGeometry().height();
        break;
    case Plasma::BottomEdge:
    default:
        extraBottom = m_toolBox->expandedGeometry().height();
    }

    if (config && !m_appletOverlay && immutability() == Plasma::Mutable) {
        if (m_appletsLayout->count() == 2) {
            m_mainLayout->addItem(m_appletsLayout);
        }
        m_appletOverlay = new LinearAppletOverlay(this, m_appletsLayout);
        m_appletOverlay->resize(size());
        connect (m_appletOverlay, SIGNAL(dropRequested(QGraphicsSceneDragDropEvent *)),
                 this, SLOT(overlayRequestedDrop(QGraphicsSceneDragDropEvent *)));


        qreal left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        setContentsMargins(left + extraLeft, top + extraTop, right + extraRight, bottom + extraBottom);
    } else if (!config) {
        delete m_appletOverlay;
        m_appletOverlay = 0;
        if (m_appletsLayout->count() == 2) {
            m_mainLayout->removeItem(m_appletsLayout);
        }


        qreal left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        setContentsMargins(left - extraLeft, top - extraTop, right - extraRight, bottom - extraBottom);
    }
}

void SearchLaunch::overlayRequestedDrop(QGraphicsSceneDragDropEvent *event)
{
    dropEvent(event);
}

void SearchLaunch::paintInterface(QPainter *, const QStyleOptionGraphicsItem *, const QRect &)
{
    if (m_stripUninitialized) {
        m_stripUninitialized = false;
        QTimer::singleShot(100, this, SLOT(restoreStrip()));
    }
}

void SearchLaunch::delayedQuery()
{
    m_searchTimer->start(500);
}

void SearchLaunch::query()
{
    doSearch(m_searchField->text());
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
    Containment::focusInEvent(event);
}

void SearchLaunch::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ContentsRectChange && !m_toolBox->isShowing()) {
        qreal left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);

        //left preferred over right
        if (left > top && left > right && left > bottom) {
            m_toolBox->setLocation(Plasma::RightEdge);
        } else if (right > top && right >= left && right > bottom) {
            m_toolBox->setLocation(Plasma::LeftEdge);
        } else if (bottom > top && bottom > left && bottom > right) {
            m_toolBox->setLocation(Plasma::TopEdge);
        //bottom is the default
        } else {
            m_toolBox->setLocation(Plasma::BottomEdge);
        }
    }
}


K_EXPORT_PLASMA_APPLET(sal, SearchLaunch)

#include "sal.moc"

