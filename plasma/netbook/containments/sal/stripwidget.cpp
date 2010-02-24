/*
 *   Copyright 2009 by Artur Duque de Souza <asouza@kde.org>
 *
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

#include "stripwidget.h"

#include <QGraphicsGridLayout>
#include <QGraphicsScene>
#include <QToolButton>
#include <QAction>
#include <QTimer>
#include <QFile>
#include <QDir>

#include <KIcon>
#include <KIconLoader>
#include <KRun>

#include <Plasma/Frame>
#include <Plasma/ToolButton>
#include <Plasma/IconWidget>
#include <Plasma/QueryMatch>
#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>
#include <Plasma/ScrollWidget>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

#include "itemview.h"
#include "iconactioncollection.h"

StripWidget::StripWidget(Plasma::RunnerManager *rm, QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_runnermg(rm),
      m_itemView(0),
      m_deleteTarget(0),
      m_iconActionCollection(0),
      m_offset(0),
      m_startupCompleted(false)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setAcceptDrops(true);

    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(parent);
    if (applet) {
        m_iconActionCollection = new IconActionCollection(applet, this);
    }

    m_arrowsLayout = new QGraphicsLinearLayout(this);
    m_arrowsLayout->setContentsMargins(0, 0, 0, 0);
    setFocusPolicy(Qt::StrongFocus);

    m_leftArrow = new Plasma::ToolButton(this);
    m_leftArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_leftArrow->setPreferredWidth(KIconLoader::SizeMedium);
    m_leftArrow->setImage("widgets/arrows", "left-arrow");
    connect(m_leftArrow, SIGNAL(clicked()), this, SLOT(goLeft()));
    connect(m_leftArrow, SIGNAL(pressed()), this, SLOT(scrollTimeout()));

    m_rightArrow = new Plasma::ToolButton(this);
    m_rightArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_rightArrow->setPreferredWidth(KIconLoader::SizeMedium);
    m_rightArrow->setImage("widgets/arrows", "right-arrow");
    connect(m_rightArrow, SIGNAL(clicked()), this, SLOT(goRight()));
    connect(m_rightArrow, SIGNAL(pressed()), this, SLOT(scrollTimeout()));

    m_leftArrow->setEnabled(false);
    m_rightArrow->setEnabled(false);
    m_leftArrow->hide();
    m_rightArrow->hide();

    m_itemView = new ItemView(this);
    m_itemView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_itemView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_itemView->installEventFilter(this);
    m_itemView->setOrientation(Qt::Horizontal);
    m_itemView->setIconSize(KIconLoader::SizeLarge);
    m_itemView->setDragAndDropMode(ItemContainer::MoveDragAndDrop);

    connect(m_itemView, SIGNAL(itemActivated(Plasma::IconWidget *)), this, SLOT(launchFavourite(Plasma::IconWidget *)));
    connect(m_itemView, SIGNAL(scrollBarsNeededChanged(ItemView::ScrollBarFlags)), this, SLOT(arrowsNeededChanged(ItemView::ScrollBarFlags)));
    connect(m_itemView, SIGNAL(itemReordered(Plasma::IconWidget *, int)), this, SLOT(itemReordered(Plasma::IconWidget *, int)));
    connect(m_itemView, SIGNAL(dragStartRequested(Plasma::IconWidget *)), this, SLOT(showDeleteTarget()));

    m_arrowsLayout->addItem(m_leftArrow);
    m_arrowsLayout->addItem(m_itemView);
    m_arrowsLayout->addItem(m_rightArrow);

    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setSingleShot(false);
    connect(m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimeout()));

    m_setCurrentTimer = new QTimer(this);
    m_setCurrentTimer->setSingleShot(false);
    connect(m_setCurrentTimer, SIGNAL(timeout()), this, SLOT(highlightCurrentItem()));
}

StripWidget::~StripWidget()
{
    foreach(Plasma::QueryMatch *match, m_favouritesMatches) {
        delete match;
    }
}

Plasma::IconWidget *StripWidget::createIcon(const QPointF &point)
{
    // create new IconWidget for favourite strip

    Plasma::IconWidget *fav = m_itemView->createItem();
    fav->hide();
    fav->setTextBackgroundColor(QColor());
    fav->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    fav->installEventFilter(this);


    Plasma::ToolTipContent toolTipData = Plasma::ToolTipContent();
    toolTipData.setAutohide(true);


    Plasma::ToolTipManager::self()->registerWidget(this);
    Plasma::ToolTipManager::self()->setContent(fav, toolTipData);

    connect(fav, SIGNAL(activated()), this, SLOT(launchFavourite()));

    // set an action to be able to remove from favourites
    QAction *action = new QAction(fav);
    action->setIcon(KIcon("list-remove"));
    fav->addIconAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(removeFavourite()));

    if (m_iconActionCollection) {
        m_iconActionCollection->addAction(action);
    }

    if (point != QPointF()) {
        m_itemView->insertItem(fav, m_itemView->positionToWeight(point));
    } else {
        m_itemView->addItem(fav);
    }

    if (m_startupCompleted) {
        m_itemView->setCurrentItem(fav);
        m_setCurrentTimer->start(300);
    }

    return fav;
}

void StripWidget::showDeleteTarget()
{
    if (!m_deleteTarget) {
        m_deleteTarget = new Plasma::IconWidget();
        scene()->addItem(m_deleteTarget);
        m_deleteTarget->setIcon("user-trash");
        m_deleteTarget->resize(KIconLoader::SizeHuge, KIconLoader::SizeHuge);
        m_deleteTarget->setZValue(99);
    }
    m_deleteTarget->setPos(mapToScene(boundingRect().bottomLeft()));
    m_deleteTarget->show();
}

void StripWidget::setImmutability(Plasma::ImmutabilityType immutability)
{
    if (immutability == Plasma::Mutable) {
        m_itemView->setDragAndDropMode(ItemContainer::MoveDragAndDrop);
    } else {
        m_itemView->setDragAndDropMode(ItemContainer::NoDragAndDrop);
    }
}

void StripWidget::highlightCurrentItem()
{
    m_itemView->setCurrentItem(m_itemView->currentItem());
}

void StripWidget::add(Plasma::QueryMatch match, const QString &query, const QPointF &point)
{
    // add to layout and data structures
    Plasma::QueryMatch *newMatch = new Plasma::QueryMatch(match);
    m_favouritesMatches.append(newMatch);
    m_favouritesQueries.insert(newMatch, query);

    Plasma::IconWidget *icon = createIcon(point);

    icon->setText(newMatch->text());
    icon->setIcon(newMatch->icon());

    icon->setMinimumSize(icon->sizeFromIconSize(m_itemView->iconSize()));
    icon->setMaximumSize(icon->sizeFromIconSize(m_itemView->iconSize()));

    Plasma::ToolTipContent toolTipData = Plasma::ToolTipContent();
    toolTipData.setAutohide(true);
    toolTipData.setMainText(newMatch->text());
    toolTipData.setSubText(newMatch->subtext());
    toolTipData.setImage(newMatch->icon());

    Plasma::ToolTipManager::self()->registerWidget(icon);
    Plasma::ToolTipManager::self()->setContent(icon, toolTipData);

    m_favouritesIcons.insert(icon, newMatch);
}

void StripWidget::add(const QString &fileName, const QPointF &point)
{
    KDesktopFile *file = new KDesktopFile(fileName);
    Plasma::IconWidget * icon = createIcon(point);

    icon->setIcon(file->readIcon());
    icon->setText(file->readName());

    icon->setMinimumSize(icon->sizeFromIconSize(m_itemView->iconSize()));
    icon->setMaximumSize(icon->sizeFromIconSize(m_itemView->iconSize()));

    Plasma::ToolTipContent toolTipData = Plasma::ToolTipContent();
    toolTipData.setAutohide(true);
    toolTipData.setMainText(file->readName());
    toolTipData.setSubText(file->readGenericName());
    toolTipData.setImage(KIcon(file->readIcon()));

    Plasma::ToolTipManager::self()->registerWidget(icon);
    Plasma::ToolTipManager::self()->setContent(icon, toolTipData);

    m_desktopFiles.insert(icon, file);
}

void StripWidget::remove(Plasma::IconWidget *favourite)
{
    if (!favourite) {
        return;
    }

    if (m_favouritesIcons.contains(favourite)) {
        Plasma::QueryMatch *match = m_favouritesIcons.value(favourite);
        m_favouritesMatches.removeOne(match);
        m_favouritesQueries.remove(match);
        m_favouritesIcons.remove(favourite);

        // must be deleteLater because the IconWidget will return from the action?
        favourite->deleteLater();
        delete match;
    } else if (m_desktopFiles.contains(favourite)) {
        delete m_desktopFiles.value(favourite);
        m_desktopFiles.remove(favourite);
        favourite->deleteLater();
    }
}

void StripWidget::removeFavourite()
{
    Plasma::IconWidget *icon = qobject_cast<Plasma::IconWidget*>(sender()->parent());

    if (icon) {
        remove(icon);
    }
}

void StripWidget::itemReordered(Plasma::IconWidget *icon, int index)
{
    if (m_deleteTarget && m_deleteTarget->geometry().intersects(icon->mapToItem(this, icon->boundingRect()).boundingRect())) {
        remove(icon);
        m_deleteTarget->hide();
    } else if (m_favouritesIcons.contains(icon)) {
        Plasma::QueryMatch *match = m_favouritesIcons.value(icon);
        m_favouritesMatches.removeAll(match);
        m_favouritesMatches.insert(index, match);
    }
}

void StripWidget::launchFavourite()
{
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(sender());
    
    if (m_favouritesIcons.contains(icon)) {
        Plasma::QueryMatch *match = m_favouritesIcons.value(icon);

        Plasma::RunnerContext context;
        context.setQuery(m_favouritesQueries.value(match));
        match->run(context);
    } else if (m_desktopFiles.contains(icon)) {
        KRun::run(KService(m_desktopFiles.value(icon)), KUrl::List(), 0, false);
    }
}

void StripWidget::launchFavourite(Plasma::IconWidget *icon)
{
    Plasma::QueryMatch *match = m_favouritesIcons.value(icon);

    Plasma::RunnerContext context;
    context.setQuery(m_favouritesQueries.value(match));
    match->run(context);
}

void StripWidget::goRight()
{
    QRectF rect(m_itemView->boundingRect());
    rect.moveLeft(rect.right() - m_itemView->widget()->pos().x());
    rect.setWidth(rect.width()/4);

    m_itemView->ensureRectVisible(rect);
}

void StripWidget::goLeft()
{
    QRectF rect(m_itemView->boundingRect());
    rect.setWidth(rect.width()/4);
    rect.moveRight(- m_itemView->widget()->pos().x());

    m_itemView->ensureRectVisible(rect);
}

void StripWidget::scrollTimeout()
{
    if (!m_scrollTimer->isActive()) {
        m_scrollTimer->start(250);
    } else if (m_leftArrow->isDown()) {
        goLeft();
    } else if (m_rightArrow->isDown()) {
        goRight();
    } else {
        m_scrollTimer->stop();
    }
}

void StripWidget::save(KConfigGroup &cg)
{
    kDebug() << "----------------> Saving Stuff...";

    // erase the old stuff before saving the new one
    KConfigGroup oldGroup(&cg, "stripwidget");
    oldGroup.deleteGroup();

    KConfigGroup stripGroup(&cg, "stripwidget");

    int id = 0;
    foreach(Plasma::IconWidget *icon, m_itemView->items()) {
        // Write now just saves one for tests. Later will save
        // all the strip
        KConfigGroup config(&stripGroup, QString("favourite-%1").arg(id));

        //config.writeEntry("icon", match->);
        config.writeEntry("text", icon->text());

        if (m_favouritesIcons.contains(icon)) {
            Plasma::QueryMatch *match = m_favouritesIcons.value(icon);
            config.writeEntry("runnerid", match->runner()->id());
            config.writeEntry("query", m_favouritesQueries.value(match));
            config.writeEntry("matchId", match->id());
            config.writeEntry("subText", match->subtext());
        } else if (m_desktopFiles.contains(icon)) {
            config.writeEntry("url", (m_desktopFiles[icon])->fileName());
            config.writeEntry("subText", m_desktopFiles[icon]->readGenericName());
        }

        ++id;
    }
}

void StripWidget::restore(KConfigGroup &cg)
{
    kDebug() << "----------------> Restoring Stuff...";

    KConfigGroup stripGroup(&cg, "stripwidget");

    // get all the favourites
    QStringList groupNames(stripGroup.groupList());
    qSort(groupNames);
    QMap<uint, KConfigGroup> favouritesConfigs;
    foreach (const QString &favouriteGroup, stripGroup.groupList()) {
        if (favouriteGroup.startsWith("favourite-")) {
            KConfigGroup favouriteConfig(&stripGroup, favouriteGroup);
            favouritesConfigs.insert(favouriteGroup.split("-").last().toUInt(), favouriteConfig);
        }
    }

    QVector<QString> runnerIds;
    QVector<QString> queries;
    QVector<QString> matchIds;
    QVector<QString> urls;

    if (favouritesConfigs.isEmpty()) {
        runnerIds.resize(4);
        queries.resize(4);
        matchIds.resize(4);
        QString homePlace = "places_file://"+QDir::homePath();
        matchIds << "services_kde4-konqbrowser.desktop" << "services_kde4-KMail.desktop" << "services_kde4-systemsettings.desktop" << homePlace;
        queries << "konqueror" << "kmail" << "systemsettings" << "home";
        runnerIds << "services" << "services" << "services" << "places";
    } else {
        runnerIds.resize(favouritesConfigs.size());
        queries.resize(favouritesConfigs.size());
        matchIds.resize(favouritesConfigs.size());
        urls.resize(favouritesConfigs.size());
        QMap<uint, KConfigGroup>::const_iterator it = favouritesConfigs.constBegin();
        int i = 0;
        while (it != favouritesConfigs.constEnd()) {
            KConfigGroup favouriteConfig = it.value();

            runnerIds[i] = favouriteConfig.readEntry("runnerid");
            queries[i] = favouriteConfig.readEntry("query");
            matchIds[i] = favouriteConfig.readEntry("matchId");
            urls[i] = favouriteConfig.readEntry("url");
            ++i;
            ++it;
        }
    }

    QString currentQuery;
    int numIcons = stripGroup.groupList().size();
    for (int i = 0; i < numIcons; ++i ) {
        if (!urls[i].isNull()) {
            add(urls[i]);
        } else {
            // perform the query
            m_runnermg->blockSignals(true);
            const bool found = m_runnermg->execQuery(queries[i], runnerIds[i]);
            m_runnermg->blockSignals(false);
            if (currentQuery == queries[i] || found) {
                currentQuery = queries[i];
                // find our match
                Plasma::QueryMatch match(m_runnermg->searchContext()->match(matchIds[i]));

                // we should verify some other saved information to avoid putting the
                // wrong item if the search result is different!
                if (match.isValid()) {
                    add(match, queries[i]);
                }
            }
        }
    }

    m_startupCompleted = true;
}

void StripWidget::setIconSize(int iconSize)
{
    m_itemView->setIconSize(iconSize);
}

int StripWidget::iconSize() const
{
    return m_itemView->iconSize();
}

void StripWidget::arrowsNeededChanged(ItemView::ScrollBarFlags flags)
{
    bool leftNeeded = false;
    bool rightNeeded = false;

    if (flags & ItemView::HorizontalScrollBar) {
        leftNeeded = m_itemView->scrollPosition().x() > 0;
        rightNeeded = m_itemView->contentsSize().width() - m_itemView->scrollPosition().x() > m_itemView->size().width();
    }

    m_leftArrow->setEnabled(leftNeeded);
    m_rightArrow->setEnabled(rightNeeded);
    m_leftArrow->setVisible(leftNeeded|rightNeeded);
    m_rightArrow->setVisible(leftNeeded|rightNeeded);
    m_arrowsLayout->invalidate();
}



void StripWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    m_itemView->setFocus();
}

void StripWidget::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
     event->setAccepted(event->mimeData()->hasFormat("application/x-plasma-salquerymatch") || event->mimeData()->hasFormat("text/uri-list"));
}

void StripWidget::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    m_itemView->setScrollPositionFromDragPosition(event->pos());
}

void StripWidget::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-plasma-salquerymatch")) {
         QByteArray itemData = event->mimeData()->data("application/x-plasma-salquerymatch");
         QDataStream dataStream(&itemData, QIODevice::ReadOnly);

         QString query;
         QString runnerId;
         QString matchId;

         dataStream >>query>>runnerId>>matchId;

         //FIXME: another inefficient async query
         m_runnermg->blockSignals(true);
         m_runnermg->execQuery(query, runnerId);
         m_runnermg->blockSignals(false);

         Plasma::QueryMatch match(m_runnermg->searchContext()->match(matchId));

         if (match.isValid()) {
             add(match, query, mapToScene(event->pos()));
         }

     } else if (event->mimeData()->urls().size() > 0) {
         add(event->mimeData()->urls().first().path(), mapToScene(event->pos()));
     } else {
         event->ignore();
     }
}
