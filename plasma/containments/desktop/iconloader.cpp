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
      m_orientation(Qt::Vertical),
      m_showIcons(true),
      m_gridAlign(true),
      m_enableMedia(false)
{
    init();
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
    m_showIcons = cg.readEntry("showIcons", m_showIcons);
    m_gridAlign = cg.readEntry("alignToGrid", m_gridAlign);
    m_orientation = (Qt::Orientation)cg.readEntry("orientation", (int)m_orientation);
    setShowDeviceIcons(cg.readEntry("enableMedia", m_enableMedia));

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

    setShowIcons(m_showIcons);
}

void IconLoader::setShowDeviceIcons(bool show)
{
    if (m_enableMedia != show) {
        m_enableMedia = show;
        configureMedia();
    }
}

bool IconLoader::showDeviceIcons() const
{
    return m_enableMedia;
}

void IconLoader::reloadConfig()
{
    KConfigGroup cg(m_desktop->globalConfig());
    cg = KConfigGroup(&cg, "DesktopIcons");
    setGridAligned(cg.readEntry("alignToGrid", m_gridAlign));
    setShowIcons(cg.readEntry("showIcons", m_showIcons));
    m_orientation = (Qt::Orientation)cg.readEntry("orientation", (int)m_orientation);
    setShowDeviceIcons(cg.readEntry("enableMedia", m_enableMedia));
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
    if (!m_showIcons) {
        return QList<QAction*>();
    }

    if (actions.isEmpty()) {
        createMenu();
    }

    return actions;
}

void IconLoader::slotAlignHorizontal()
{
    m_orientation = Qt::Horizontal;
    alignIcons();
}

void IconLoader::slotAlignVertical()
{
    m_orientation = Qt::Vertical;
    alignIcons();
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
        //kDebug() << "putting" << url.path() << "into the map";
        if (m_gridAlign) {
            alignToGrid(newApplet);
        }
        m_iconMap[url.path()] =  newApplet;
    }
}

void IconLoader::addIcon(Plasma::Applet *applet)
{
    KConfigGroup cg = applet->config();
    KUrl url = cg.readEntry("Url", KUrl());
    if (!url.isEmpty()) {
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
        //kDebug() << "adding item" << item.url();
        if (!m_iconMap.contains(item.url().path())) {
            addIcon(item.url());
        }
    }
}

void IconLoader::deleteItem(const KFileItem item)
{
    QString path = item.url().path();
    if (!m_iconMap.contains(path)) {
        //kDebug() << "Icon " << path << " not found." << endl;
        return;
    }
    deleteIcon(item.url());
}

void IconLoader::appletDeleted(Plasma::Applet *applet)
{
    deleteIcon(applet);
}

void IconLoader::alignIcons()
{
    QList<Plasma::Applet*> icons = m_iconMap.values();
    if (icons.count() == 0 || !m_showIcons) {
        return;
    }

    qreal gridWidth = gridSize().width();
    qreal gridHeight = gridSize().height();

    QRectF candidateRect(0, 0, gridWidth, gridHeight);
    QList<Plasma::Applet*> placedIcons;

    foreach (Plasma::Applet* icon, icons) {
        candidateRect = QRectF(QPointF(0.0,0.0),icon->boundingRect().size());
        //check if any placed icons intersect with the icon placed at various grid locations

        candidateRect = nextFreeRect(candidateRect, placedIcons);
        if (!candidateRect.isValid()) {
            return;
        }

        //mapToGrid on the center spot to eliminate conversion error
        setToGrid(icon, mapToGrid(candidateRect.topLeft()));
        placedIcons << icon;
    }
}

QRectF IconLoader::nextFreeRect(const QRectF itemRect)
{
    return nextFreeRect(itemRect, m_iconMap.values());
}

QRectF IconLoader::nextFreeRect(const QRectF itemRect, QList<Plasma::Applet*> placedItems)
{
    QRectF newRect = itemRect;
    while (intersectsWithItems(newRect, placedItems)) {
        newRect = advanceAlongGrid(newRect);
        if (!availableGeometry().contains(newRect)) {
            //we've moved completely off the desktop so we should stop
            return QRectF();
        }
    }
    return newRect;
}

bool IconLoader::intersectsWithItems(const QRectF item, const QList<Plasma::Applet*> &items) const
{
    foreach (Plasma::Applet* testItem, items) {
        if (item.intersects(testItem->sceneBoundingRect())) {
            return true;
        }
    }
    return false;
}

QRectF IconLoader::advanceAlongGrid(QRectF rect)
{
    QRectF newRect = rect;
    qreal gridWidth = m_gridSize.width();
    qreal gridHeight = m_gridSize.height();
    
    if (m_orientation == Qt::Horizontal) {
        newRect.translate(gridWidth,0);
    } else {
        newRect.translate(0, gridHeight);
    }
    if (!availableGeometry().contains(newRect)) {
        //we've moved off of the desktop and need to move back
        if (m_orientation == Qt::Horizontal) {
            newRect.moveTo(0,newRect.y()+gridHeight);
        } else {
            newRect.moveTo(newRect.x()+gridWidth,0);
        }
    }
    return newRect;
}

void IconLoader::setToGrid(Plasma::Applet* icon, const QPoint p)
{
    //place the center of the icon in the center of the grid if it's smaller than the grid square
    QPointF newPos = mapFromGrid(p);
    if (m_gridSize.width() > icon->boundingRect().width()) {
        newPos.setX(newPos.x() + (m_gridSize.width()/2 - icon->boundingRect().width()/2));
    }
    if (m_gridSize.height() > icon->boundingRect().height()) {
        newPos.setY(newPos.y() + (m_gridSize.height()/2 - icon->boundingRect().height()/2));
    }
    icon->setPos(newPos);
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


bool IconLoader::isFreePlace(const QPointF &p)
{
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
            alignToGrid(applet,mapToGrid(applet->scenePos()));
        }
    }
    m_gridAlign = align;
}

void IconLoader::alignToGrid(Plasma::Applet *item)
{
    QRectF freeRect = nextFreeRect(QRectF(QPointF(0.0,0.0),item->boundingRect().size()));
    QPoint currentGridPos = mapToGrid(freeRect.topLeft());
    alignToGrid(item, currentGridPos);
}

void IconLoader::alignToGrid(Plasma::Applet *item, const QPoint &pos, bool moveIntersectingItems)
{
    if (moveIntersectingItems) {
        return;
    } else {
        setToGrid(item, pos);
    }
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
    return m_showIcons;
}

void IconLoader::setShowIcons(bool iconsVisible)
{
    m_showIcons = iconsVisible;
    if (m_showIcons) {
        m_desktopDir.openUrl(KGlobalSettings::desktopPath());
    } else {
        m_desktopDir.stop();
        QString desktopDir = KGlobalSettings::desktopPath();
        foreach (Plasma::Applet *icon, m_iconMap.values()) {
            KConfigGroup cg = icon->config();
            KUrl url = cg.readEntry("Url", KUrl());

            //kDebug() << "checking" << url.path() << "against" << desktopDir;
            if (url.path().startsWith(desktopDir)) {
                icon->destroy();    //the icon will be taken out of m_iconMap by the appletDeleted Slot
            }
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
