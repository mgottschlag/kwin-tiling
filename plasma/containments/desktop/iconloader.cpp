/*
*   Copyright 2007 by Christopher Blauvelt <cblauvelt@gmail.com>
*   Copyright (C) 2007 Matt Broadstone <mbroadst@kde.org>
*   Copyright (C) 2007 Matias Costa <m.costacano@gmail.com>
*   Copyright (C) 2007 Montel Laurent <montel@kde.org>
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

#include "iconloader.h"
#include "desktop.h"

#include <QGraphicsScene>

#include <KUrl>
#include <KGlobalSettings>
#include <KDebug>
#include <KAuthorized>
#include <KToggleAction>
#include <KConfigGroup>

IconLoader::IconLoader(DefaultDesktop *parent)
    : QObject(parent),
      m_desktop(parent),
      m_verticalOrientation(true),
      m_iconShow(true),
      m_gridAlign(true),
      m_enableMedia(false)
{
    QTimer::singleShot(0, this, SLOT(init()));
}

IconLoader::~IconLoader()
{
}

void IconLoader::init()
{
    m_iconMap.clear();

    //load stored settings
    KConfigGroup cg(m_desktop->globalConfig());
    cg = KConfigGroup(&cg, "DesktopIcons");
    m_iconShow = cg.readEntry("showIcons", m_iconShow);
    m_gridAlign = cg.readEntry(i18n("alignToGrid"),m_gridAlign);
    m_enableMedia = cg.readEntry(i18n("enableMedia"),m_enableMedia);

    setGridSize(QSizeF(150, 150));

    connect(m_desktop, SIGNAL(appletRemoved(Plasma::Applet*)), this, SLOT(appletDeleted(Plasma::Applet*)));
    //build list of current icons
    foreach (Plasma::Applet* applet, m_desktop->applets()) {
        if (applet->name() == i18n("Icon")) {
            addIcon(applet);
        }
    }

    //list ~/Desktop and add new applets
    m_desktopDir.setAutoUpdate(true);
    m_desktopDir.setAutoErrorHandlingEnabled(false, 0);

    connect(&m_desktopDir, SIGNAL(newItems(KFileItemList)), this, SLOT(newItems(KFileItemList)) );
    connect(&m_desktopDir, SIGNAL(deleteItem(KFileItem)), this, SLOT(deleteItem(KFileItem)));
    
    setShowIcons(m_iconShow);
}

void IconLoader::createMenu()
{
    QAction* alignHorizontal = new QAction(i18n("Align Icons Horizontally"), this);
    connect(alignHorizontal, SIGNAL(triggered(bool)), this , SLOT(slotAlignHorizontal()));
    actions.append(alignHorizontal);

    QAction* alignVertical = new QAction(i18n("Align Icons Vertically"), this);
    connect(alignVertical, SIGNAL(triggered(bool)), this , SLOT(slotAlignVertical()));
    actions.append(alignVertical);
}

QList<QAction*> IconLoader::contextActions()
{
    if (!m_iconShow) {
        return QList<QAction*>();
    }

    if (actions.isEmpty()) {
        createMenu();
    }

    return actions;
}

void IconLoader::slotAlignHorizontal()
{
    m_verticalOrientation = false;
    alignHorizontal(m_iconMap.values());
}

void IconLoader::slotAlignVertical()
{
    m_verticalOrientation = true;
    alignVertical(m_iconMap.values());
}

void IconLoader::changeAlignment(bool horizontal)
{
    m_verticalOrientation = !horizontal;
    if (m_verticalOrientation) {
        alignVertical(m_iconMap.values());
    } else {
        alignHorizontal(m_iconMap.values());
    }
}

void IconLoader::configureMedia()
{
    if (m_enableMedia) {
       if (!m_solidEngine) {
           //m_solidEngine = m_desktop->dataEngine("solidnotifierengine");
           //connect(m_solidEngine, SIGNAL(newSource(const QString&)),
           //          this, SLOT(sourceAdded(const QString&)));
           //TODO update it.
       }
    } else if (m_solidEngine) {
        m_solidEngine=0;
        // remove from the lists and delete
        foreach (Plasma::Applet *icon, m_solidDevices.values()) {
            icon->destroy();
        }
        m_solidDevices.clear();
    }
}

void IconLoader::addIcon(const KUrl& url)
{
    QVariantList args;
    args << url.path();
    Plasma::Applet *newApplet = m_desktop->addApplet(QString("icon"),args,0);
    if (newApplet) {
        m_iconMap[url.path()] =  newApplet;
    }
}

void IconLoader::addIcon(Plasma::Applet *applet)
{
    KConfigGroup cg = applet->config();
    KUrl url = cg.readEntry(i18n("Url"), KUrl());
    if (url != KUrl()) {
        m_iconMap[url.path()] = applet;
    }
}

void IconLoader::deleteIcon(const KUrl& url)
{
    Plasma::Applet *applet = m_iconMap.value(url.path());
    if (applet) {
        m_iconMap.remove(url.path());
        applet->destroy();
    }
}

void IconLoader::deleteIcon(Plasma::Applet *applet)
{
    //we must be careful because this will be entered when the applet is already
    //in the qobject dtor so we can't invoke any applet methods
    m_iconMap.remove(m_iconMap.key(applet));
}

void IconLoader::newItems(const KFileItemList& items)
{
    if (!m_desktop) {
        return;
    }


    foreach (KFileItem item, items) {
        if (!m_iconMap.contains(item.url().path())) {
            addIcon(item.url());
        }
    }
}

void IconLoader::deleteItem(const KFileItem item)
{
    QString path = item.url().path();
    if (!m_iconMap.contains(path)) {
        kDebug() << "Icon " << path << " not found." << endl;
        return;
    }
    deleteIcon(item.url());
}

void IconLoader::appletDeleted(Plasma::Applet *applet)
{
    deleteIcon(applet);
}

void IconLoader::alignHorizontal(const QList<Plasma::Applet*> &items)
{
    Plasma::Applet *icon;
    int index = 0;
    int itemCount = items.count();
    if (itemCount == 0) {
        return;
    } else {
        icon = items[0];
    }

    qreal desktopWidth = m_desktop->contentSizeHint().width();
    qreal desktopHeight = m_desktop->contentSizeHint().height();
    qreal gridWidth = gridSize().width();
    qreal gridHeight = gridSize().height();

    QPointF pos;
    for(pos.ry() = gridHeight/2; pos.y() < desktopHeight; pos.ry() += gridHeight) {
        for(pos.rx() = gridWidth/2; pos.x() < desktopWidth; pos.rx() += gridWidth) {
            Plasma::Applet *existing = dynamic_cast<Plasma::Applet*>(m_desktop->scene()->itemAt(pos));
            // If the existing index is in the items list with an index
            // greater than icon index current we will have to move it.
            // So ignore it
            // If its equal then existing == icon
            int existingIndex = items.indexOf(existing);
            if (!existing || existing ==m_desktop || existingIndex >= index) {
                alignToGrid(icon, mapToGrid(pos));
                // get next
                if (++index < itemCount) {
                    icon = items[index];
                } else {
                    /* all items located */
                    return;
                }
            }
        }
    }
    // TODO What to do if we run out of space? make grid smaller?
}

void IconLoader::alignVertical(const QList<Plasma::Applet*> &items)
{
    Plasma::Applet *icon;
    int index = 0;
    int itemCount = items.count();
    if (itemCount == 0) {
        return;
    } else {
        icon = items[0];
    }

    qreal desktopWidth = m_desktop->contentSizeHint().width();
    qreal desktopHeight = m_desktop->contentSizeHint().height();
    qreal gridWidth = gridSize().width();
    qreal gridHeight = gridSize().height();

    QPointF pos;
    for(pos.rx() = gridWidth/2; pos.x() < desktopWidth; pos.rx() += gridWidth) {
        for(pos.ry() = gridHeight/2; pos.y() < desktopHeight; pos.ry() += gridHeight) {
            Plasma::Applet *existing = dynamic_cast<Plasma::Applet*>(m_desktop->scene()->itemAt(pos));
            // If *existing is in items list with an index
            // greater than current icon index, then we will have to move
            // existing after.
            // So ignore it
            // If its equal then existing == icon then... we ignore it too
            int existingIndex = items.indexOf(existing);
            if (!existing || existing == m_desktop || existingIndex >= index) {
                alignToGrid(icon, mapToGrid(pos));
                // get next
                if (++index < itemCount) {
                    icon = items[index];
                } else {
                    /* all items located */
                    return;
                }
            }
        }
    }
    // TODO What to do if we run out of space? make grid smaller?
}

QPointF IconLoader::findFreePlaceNear(QPointF point)
{
    // TODO make this faster
    QPointF p = mapFromGrid(mapToGrid(point));
    if (isFreePlace(p)) return p;
    
    int maxD = qMax(gridDimensions().width(),
                       gridDimensions().height());
    qreal gridWidth = gridSize().width();
    qreal gridHeight = gridSize().height();
    // d is the grid squares ahead to look from p
    // each iteration we try to look d grid squares from p
    // and store free places in freePlaces list
    QList<QPointF> freePlaces;
    for (int d=1; d<maxD && freePlaces.isEmpty(); ++d) {
        // These variables control we do not look out of screen
        qreal left = qMax(p.x() -d*gridWidth, gridWidth/2);
        qreal right = qMin(p.x() +d*gridWidth,
                           m_desktop->contentSizeHint().width() -gridWidth/2);
        qreal top = qMax(p.y() -d*gridHeight, gridHeight/2);
        qreal bottom = qMin(p.y() +d*gridHeight,
                            m_desktop->contentSizeHint().height() -gridHeight/2);
        QPointF pos;
        for (qreal x=left; x<=right; x += gridWidth) {
            pos = QPointF(x, top);
            if (isFreePlace(pos)) {
                freePlaces.append(pos);
            }
            pos = QPointF(x, bottom);
            if (isFreePlace(pos)) {
                freePlaces.append(pos);
            }

        }
        for (qreal y=top; y<=bottom; y += gridHeight) {
            pos = QPointF(left, y);
            if (isFreePlace(pos)) {
                freePlaces.append(pos);
            }
            pos = QPointF(right, y);
            if (isFreePlace(pos)) {
                freePlaces.append(pos);
            }
        }
    }
    double min = maxD * gridWidth * gridHeight;
    // Take from freePlaces the nearest
    // If freePlaces is empty, p is initialized in the first line as
    // mapFromGrid(mapToGrid(point));
    foreach (QPointF free, freePlaces) {
        double d = qAbs(point.x() - free.x()) + qAbs(point.y() -free.y());
        if (d < min) {
            min = d;
            p = free;
        }
    }
    return p;
}


bool IconLoader::isFreePlace(const QPointF &p) {
    QGraphicsItem *existing = m_desktop->scene()->itemAt(p);
    bool free = (!existing) || (existing == m_desktop);
    return free;
}

bool IconLoader::isGridAligned() const
{
    return m_gridAlign;
}

void IconLoader::setGridAligned(bool align)
{
    // if icon alignment is false now and we change
    // to true then align all icons
    if (!m_gridAlign && align) {
        foreach (Plasma::Applet *applet, m_iconMap.values()) {
            alignToGrid(applet);
        }
    }
    m_gridAlign = align;
}

void IconLoader::alignToGrid(QGraphicsItem *item)
{
    QPointF currentPos = item->pos();
    QPoint currentGridPos = mapToGrid(currentPos);
    alignToGrid(item, currentGridPos);
}

void IconLoader::alignToGrid(QGraphicsItem *item, const QPoint &pos)
{
    qreal width = item->boundingRect().width();
    qreal height = item->boundingRect().height();
    QPointF scenePos = mapFromGrid(pos);
    scenePos.rx() -= width/2;
    scenePos.ry() -= height/2;
    item->setPos(scenePos);
}

QSizeF IconLoader::gridSize() const
{
    return m_gridSize;
}

QSize IconLoader::gridDimensions() const
{
    QSizeF thisSize = m_desktop->contentSizeHint();
    return QSize( int(thisSize.width() / m_gridSize.width()),
                  int(thisSize.height() / m_gridSize.height()) );
}

void IconLoader::setGridSize(const QSizeF& gridSize)
{
    QSizeF desktopSize = m_desktop->contentSizeHint();
    qreal bestMatchWidth = desktopSize.width()
                        / qRound(desktopSize.width()/gridSize.width());
    qreal bestMatchHeight = desktopSize.height()
                        / qRound(desktopSize.height()/gridSize.height());
    m_gridSize = QSizeF(bestMatchWidth, bestMatchHeight);
    if (m_gridAlign) {
        foreach (Plasma::Applet *item, m_iconMap.values()) {
            alignToGrid(item);
        }
    }
}

bool IconLoader::showIcons() const
{
    return m_iconShow;
}

void IconLoader::setShowIcons(bool iconsVisible)
{
    m_iconShow = iconsVisible;
    if (m_iconShow) {
        m_desktopDir.openUrl(KGlobalSettings::desktopPath());
    } else {
        m_desktopDir.stop();
        foreach (Plasma::Applet *icon, m_iconMap.values()) {
            icon->destroy();    //the icon will be taken out of m_iconMap by the appletDeleted Slot
        }
    }
}

void IconLoader::sourceAdded(const QString &source)
{
    Q_UNUSED(source)
    kDebug() << "Not yet implemented.";
}

void IconLoader::sourceDeleted(const QString &source)
{
    Q_UNUSED(source)
    kDebug() << "Not yet implemented.";
}

#include "iconloader.moc"
