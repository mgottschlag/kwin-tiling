/***************************************************************************
 *   Copyright (C) 2008 by Lukas Appelhans                                 *
 *   l.appelhans@gmx.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "quicklaunchApplet.h"

#include <KConfigDialog>
#include <KDesktopFile>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsWidget>
#include <QDrag>
#include <QHash>
#include <QMouseEvent>
#include <QMimeData>
#include <QToolButton>

#include <KDialog>
#include <KMimeType>
#include <KStandardDirs>
#include <KWindowSystem>
#include <KIconLoader>
#include <KUrl>

#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/corona.h>

#include "math.h"

//TODO: Make Dialog resizeable (which also makes the number of rows in it configurable :)

static const int s_defaultIconSize = 16;
static const int s_defaultSpacing = 2;

QuicklaunchApplet::QuicklaunchApplet(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent, args),
    m_layout(0),
    m_innerLayout(0),
    m_timer(new QTimer(this)),
    m_visibleIcons(6),
    m_preferredIconSize(s_defaultIconSize),
    m_iconSize(s_defaultIconSize),
    m_dialogIconSize(s_defaultIconSize),
    m_dialog(0),
    m_dialogWidget(0),
    m_dialogLayout(0),
    m_addDialog(0),
    m_rightClickedIcon(0),
    m_addAction(0),
    m_removeAction(0),
    m_sortappAscending(0),
    m_sortappDescending(0)
{
    setHasConfigurationInterface(true);
    setAcceptDrops(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(performUiRefactor()));
    m_timer->setSingleShot(true);

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    resize(s_defaultIconSize * 3 + s_defaultSpacing * 4 + left + right,
           s_defaultIconSize + s_defaultSpacing * 2 + top + bottom);
}

QuicklaunchApplet::~QuicklaunchApplet()
{
    if (m_dialog) {
        m_dialog->close();
        delete m_dialog;
    }

    delete m_dialogWidget;
}

void QuicklaunchApplet::saveState(KConfigGroup &config) const
{
    QStringList iconUrls;
    foreach (QuicklaunchIcon * container, m_icons) {
        iconUrls.append(container->url().prettyUrl());
    }

    config.writeEntry("iconUrls", iconUrls);
}

void QuicklaunchApplet::init()
{
    KConfigGroup cg = config();
    m_preferredIconSize = m_iconSize = qMax(s_defaultIconSize, (int)cg.readEntry("iconSize", contentsRect().height() / 2));
    m_visibleIcons = qMax(-1, cg.readEntry("visibleIcons", m_visibleIcons));
    m_dialogIconSize = qMax(s_defaultIconSize, (int)cg.readEntry("dialogIconSize", contentsRect().height() / 2));

    // Initialize outer layout
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    // Initialize inner layout
    m_innerLayout = new QuicklaunchLayout(1, this, 0);
    m_innerLayout->setContentsMargins(0, 0, 0, 0);
    m_innerLayout->setSpacing(0);
    m_layout->addItem(m_innerLayout);

    // Initial "more icons" arrow
    m_arrow = new Plasma::IconWidget(this);
    m_arrow->setIcon(KIcon("arrow-right"));
    connect(m_arrow, SIGNAL(clicked()), SLOT(showDialog()));

    QStringList desktopFiles = cg.readEntry("iconUrls", QStringList());

    bool firstStart = desktopFiles.isEmpty();
    if (firstStart) {
        QStringList defaultApps;
        defaultApps << "konqbrowser" << "dolphin" << "kopete";

        foreach (const QString &defaultApp, defaultApps) {
            KService::Ptr service = KService::serviceByStorageId(defaultApp);
            if (service && service->isValid()) {
                QString path = service->entryPath();
                //kDebug() << path;
                if (!path.isEmpty() && QDir::isAbsolutePath(path)) {
                    desktopFiles << path;
                }
            }
        }
        //kDebug() << desktopFiles;
    }

    foreach (const QString &desktopFile, desktopFiles) {
        addProgram(-1, desktopFile);
    }

    performUiRefactor();

    if (firstStart) {
        resize(sizeHint(Qt::PreferredSize));
    }
}

QSizeF QuicklaunchApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (which == Qt::PreferredSize) {
        QSizeF sizeHint = size();
        if (!m_innerLayout) {
            return sizeHint;
        }
        qreal newWidth = m_innerLayout->columnCount() * sizeHint.height() / qMax(1, m_innerLayout->preferredRowCount());
        if (m_icons.size() > m_visibleIcons) {
            sizeHint.setWidth(newWidth + sizeHint.height());
        } else {
            sizeHint.setWidth(newWidth);
        }
        return sizeHint;
    }
    return QGraphicsWidget::sizeHint(which, constraint);
}

void QuicklaunchApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::SizeConstraint) {
        refactorUi();
    }
}

void QuicklaunchApplet::refactorUi()
{
     //static int my_counter = 0;
     //kDebug() << "refactorUi count = " << my_counter++;
     m_timer->start(100);
}

void QuicklaunchApplet::performUiRefactor()
{
    //static int my_perforcount = 0;
    //kDebug() << "performUiRefactor count = " << my_perforcount++;
    clearLayout(m_innerLayout);

    //Don't accept values under 16 nor anything over applet's height
    m_iconSize = qMin(qMax(m_preferredIconSize, s_defaultIconSize), (int)(contentsRect().height() - 2 * s_defaultSpacing));
    m_dialogIconSize = qMax(m_dialogIconSize, s_defaultIconSize);

    if (m_dialogLayout) {
        clearLayout(m_dialogLayout);
        m_dialogLayout->setPreferredRowCount((int)(size().height() / qMin(m_dialogIconSize, m_dialog->size().height())));
    }

    int rowCount;
    if (formFactor() == Plasma::Vertical) {
        int numIcons = qMin(m_icons.count(), m_visibleIcons);
        int colCount = qMax(qreal(1), contentsRect().width() / m_iconSize);
        rowCount = numIcons / colCount + numIcons % colCount;
        // prevent possible division by zero if size().width() is 0
    } else {
        rowCount = contentsRect().height() / m_iconSize;
        // prevent possible division by zero if size().height() is 0
    }

    rowCount = qMax(1, rowCount);
    m_innerLayout->setPreferredRowCount(rowCount);
    int count = 0;
    //kDebug() << m_icons.count() << "pixel icons in" << rowCount
    //         << "rows, with a max of" << m_visibleIcons << "visible";
    foreach (QuicklaunchIcon *icon, m_icons) {
        //icon->setMinimumSize(minSize);
        //icon->setMaximumSize(maxSize);
        if (count < m_visibleIcons || m_visibleIcons == -1) {
            icon->resize(QSize(m_iconSize, m_iconSize));
            icon->setIconSize(m_iconSize);
            icon->show();
            m_innerLayout->addItem(icon);
        } else if (m_dialogLayout) {
            icon->setMinimumSize(QSize(m_dialogIconSize, m_dialogIconSize));//TODO: Remove maximum/minimum sizes
            icon->setMaximumSize(QSize(m_dialogIconSize, m_dialogIconSize));
            icon->setIconSize(m_dialogIconSize);
            icon->show();
            m_dialogLayout->addItem(icon);
        } else {
            icon->hide();
        }

        ++count;
    }

    m_layout->removeItem(m_arrow);
    if (count > m_visibleIcons && m_visibleIcons != -1) {
        //m_arrow->setMinimumSize(minSize);
        //m_arrow->setMaximumSize(maxSize);
        m_arrow->resize(QSize(size().height(), size().height()));
        m_layout->addItem(m_arrow);
        m_arrow->show();
    } else {
        m_arrow->hide();
    }

    int cols = qMax(1,m_innerLayout->columnCount());
    int icons = qMax(1, qMin(m_icons.size(), m_visibleIcons));

    setPreferredSize(QSize((m_iconSize + 6) * cols,
                           (m_iconSize + 6) * ceil(icons / (double)cols)));
    
    if (m_dialog) {
        m_dialog->close();
        m_dialogLayout->updateGeometry();
        m_dialog->adjustSize();
    }

    update();
}

void QuicklaunchApplet::showDialog()
{
    if (!m_dialog) {
        m_dialogWidget = new QGraphicsWidget(this);
        m_dialogWidget->setAcceptDrops(true);
        m_dialogWidget->installEventFilter(this);
        qobject_cast<Plasma::Corona*>(m_dialogWidget->scene())->addOffscreenWidget(m_dialogWidget);

        // Initialize "more icons" dialog
        m_dialog = new Plasma::Dialog(0, Qt::X11BypassWindowManagerHint);
        m_dialog->setAcceptDrops(true);
        //m_dialog->installEventFilter(this);
        m_dialog->setContextMenuPolicy(Qt::ActionsContextMenu);
        m_dialogLayout = new QuicklaunchLayout(1, m_dialogWidget, m_dialogWidget);
        m_dialogWidget->setLayout(m_dialogLayout);
        performUiRefactor();
        m_dialog->setGraphicsWidget(m_dialogWidget);
    }

    if (m_dialog->isVisible()) {
        m_dialog->hide();
    } else {
        m_dialog->resize(m_dialogLayout->preferredSize().toSize());
        //m_dialog->updateGeometry();
        if (containment() && containment()->corona()) {
            //kDebug() << "position:" << containment()->corona()->popupPosition(m_arrow, m_dialog->size()) << "dialog size:" << m_dialog->size() << "layout preferred-size:" << m_dialogLayout->preferredSize().toSize();
            m_dialog->move(containment()->corona()->popupPosition(m_arrow, m_dialog->size()));
        }
        KWindowSystem::setState(m_dialog->winId(), NET::SkipTaskbar);
        m_dialog->show();
    }
}

void QuicklaunchApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    uiConfig.setupUi(widget);
    connect(parent, SIGNAL(accepted()), SLOT(configAccepted()));

    int dialogHeight = (int)KIconLoader::SizeEnormous;
    int height = dialogHeight;

    if (formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical) {
        height = contentsRect().height() - 2 * s_defaultSpacing;
    }

    uiConfig.iconSizeSpin->setMaximum(height);
    uiConfig.iconSizeSlider->setMaximum(height);
    uiConfig.dialogIconSizeSpin->setMaximum(dialogHeight);
    uiConfig.dialogIconSizeSlider->setMaximum(dialogHeight);

    uiConfig.iconSizeSpin->setMinimum(s_defaultIconSize);
    uiConfig.iconSizeSlider->setMinimum(s_defaultIconSize);
    uiConfig.dialogIconSizeSpin->setMinimum(s_defaultIconSize);
    uiConfig.dialogIconSizeSlider->setMinimum(s_defaultIconSize);

    uiConfig.iconSizeSpin->setValue(m_preferredIconSize);
    uiConfig.iconSizeSlider->setValue(m_preferredIconSize);
    uiConfig.dialogIconSizeSpin->setValue(m_dialogIconSize);
    uiConfig.dialogIconSizeSlider->setValue(m_dialogIconSize);

    uiConfig.icons->setValue(m_visibleIcons);
    parent->addPage(widget, i18n("General"), icon());
}

void QuicklaunchApplet::configAccepted()
{
    KConfigGroup cg = config();

    bool changed = false;
    int temp;

    temp = uiConfig.icons->value();
    if (temp != m_visibleIcons) {
        m_visibleIcons = temp;
        cg.writeEntry("visibleIcons", m_visibleIcons);
        changed = true;
    }

    temp = uiConfig.iconSizeSpin->value();
    if (temp != m_preferredIconSize) {
        m_preferredIconSize = temp;
        cg.writeEntry("iconSize", m_preferredIconSize);
        changed = true;
    }

    temp = uiConfig.dialogIconSizeSpin->value();
    if (temp != m_dialogIconSize) {
        m_dialogIconSize = temp;
        cg.writeEntry("dialogIconSize", m_dialogIconSize);
        changed = true;
    }

    if (changed) {
        emit configNeedsSaving();
        resize(sizeHint(Qt::PreferredSize));
        performUiRefactor();
    }
}

QList<QAction*> QuicklaunchApplet::contextActions(QuicklaunchIcon *icon)
{
    QList<QAction*> tempActions;
    if (!m_addAction) {
        m_addAction = new QAction(KIcon("list-add"), i18n("Add Icon..."), this);
        connect(m_addAction, SIGNAL(triggered(bool)), this, SLOT(showAddInterface()));
    }

    tempActions << m_addAction;

    if (icon) {
        m_rightClickedIcon = icon;
        if (!m_removeAction) {
            m_removeAction = new QAction(KIcon("list-remove"), i18n("Remove Icon"), this);
            connect(m_removeAction, SIGNAL(triggered(bool)), this, SLOT(removeCurrentIcon()));
        }
        tempActions << m_removeAction;
    }
    
    if (!m_sortappAscending) {
        m_sortappAscending = new QAction(KIcon("view-sort-ascending"), i18n("Sort Alphabetically (A to Z)"), this);
        connect(m_sortappAscending, SIGNAL(triggered(bool)), this, SLOT(ascendingSort()));
    }
    tempActions << m_sortappAscending;

    if (!m_sortappDescending) {
        m_sortappDescending = new QAction(KIcon("view-sort-descending"), i18n("Sort Alphabetically (Z to A)"), this);
        connect(m_sortappDescending, SIGNAL(triggered(bool)), this, SLOT(descendingSort()));
    }
    tempActions << m_sortappDescending;

    return tempActions;
}

void QuicklaunchApplet::ascendingSort() 
{
    sortQuicklaunch(AscendingSort);
}

void QuicklaunchApplet::descendingSort()
{
    sortQuicklaunch(DescendingSort);
}

void QuicklaunchApplet::sortQuicklaunch(SortingOrder sortingorder)
{
    QHash <QString,QString> quicklaunchHash;
    KUrl::List urls;
    QList<QString> sortedList;
    QList<QString> saveSortedUrlList;

    foreach (QuicklaunchIcon *icon, m_icons) {
        quicklaunchHash.insert(icon->appName(),icon->url().prettyUrl());
    }
    sortedList = quicklaunchHash.keys();

    qSort(sortedList);

    for (int i = 0; i < quicklaunchHash.size(); i++) {
        saveSortedUrlList.append(quicklaunchHash.value(sortedList.value(i)));
    }

    if (sortingorder == DescendingSort) {
        QList<QString> tempUrl;

        for (int i = saveSortedUrlList.size(); i > 0; i--) {
            tempUrl.append(saveSortedUrlList.takeLast());
        }
        saveSortedUrlList = tempUrl;
    }

    foreach (QuicklaunchIcon *icon, m_icons) {
        m_icons.removeAll(icon);
        icon->hide();
        icon->deleteLater();
    }

    foreach (const QString &desktopFile, saveSortedUrlList) {
        addProgram(-1, desktopFile);
    }
    performUiRefactor();

    KConfigGroup cg = config();
    cg.writeEntry("iconUrls", saveSortedUrlList);
}

void QuicklaunchApplet::dropApp(QGraphicsSceneDragDropEvent *event, bool droppedOnDialog)
{
    int pos = 0;
    if (droppedOnDialog) {
        QPointF point = event->pos();
        for(int i = 0; i < m_dialogLayout->count(); i++) {
           QGraphicsLayoutItem *item = m_dialogLayout->itemAt(i);
           if (item->geometry().contains(point)) {
               //m_dialogLayout->insertItem(dropedItem, i + 1);
               pos = i + 1 + m_visibleIcons;
               kDebug() << "The position of Droped Item in dialog is = " << pos;
               break;
           }
        }
    } else if (!m_icons.isEmpty()) {
        //qreal left, top, bottom, right;
        //getContentsMargins(&left, &top, &bottom, &right);
        QPointF point = mapFromScene(event->scenePos());//) + QPointF(left, top);
        int rowCount = m_innerLayout->rowCount();
        //kDebug() << "RowCount = " << rowCount;
        int colCount = m_innerLayout->columnCount();
        int col = 0;
        while (col < colCount) {

            if (col == colCount || col * rowCount >= m_icons.count()) {
                break;
            }

            //kDebug() << col << m_icons.at(col * rowCount)->geometry().left() << point.x();
            if (m_icons.at(col * rowCount)->geometry().left() > point.x()) {
                //kDebug() << "broke col at" << col;
                break;
            }

            ++col;
        }

        //int col = static_cast<int>((round(point.x()) * colCount / m_innerLayout->geometry().width()));
        //col = (col >= colCount) ? colCount - 1 : col;
        //int row = static_cast<int>(floor(point.y() * rowCount / m_innerLayout->geometry().height()));
        //row = (row >= m_innerLayout->rowCount()) ? m_innerLayout->rowCount() - 1 : row;
        //kDebug() << "doing rows";
        int row = -1;
        while (row < rowCount) {
            ++row;
            if (row == rowCount || row == m_icons.count()) {
                //kDebug() << "row made rowCount" << row;
                break;
            }

            //kDebug() << "row: " << row << m_icons.count();
            if (m_icons.at(row)->geometry().center().y() > point.y()) {
                //kDebug() << "broke row at" << row;
                break;
            }
        }

        //kDebug() << "row = " << row << "rows = " << rowCount << "cols = " << colCount << "col = " << col;
        if (rowCount > 1) {
            pos = row + rowCount * (col - 1);
        } else {
            pos = col;
        }

        //kDebug() << "position is " << pos;
    }

    if (dropHandler(pos, event->mimeData())) {
       event->setDropAction(Qt::MoveAction);
       event->accept();
       saveConfig();
       performUiRefactor();
    }
}

bool QuicklaunchApplet::eventFilter(QObject * object, QEvent * event)
{
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        m_mousePressPos = static_cast<QGraphicsSceneMouseEvent*>(event)->pos();
    }

    if (event->type() == QEvent::GraphicsSceneMouseMove) {
        QuicklaunchIcon * icon = qobject_cast<QuicklaunchIcon*>(object);
        if (m_icons.contains(icon)) {
            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
            int dragDistance = (mouseEvent->pos() - m_mousePressPos).toPoint().manhattanLength();
            if (dragDistance >= QApplication::startDragDistance()) {
                int numberOfPreviousIcons = m_icons.count();
                QMimeData *mimeData = new QMimeData();
                mimeData->setData("text/uri-list", icon->url().url().toAscii());
                mimeData->setText(mimeData->text());

                QDrag *drag = new QDrag(mouseEvent->widget());
                drag->setMimeData(mimeData);
                drag->setPixmap(icon->icon().pixmap(m_iconSize, m_iconSize));

                Qt::DropAction dropAction = drag->exec();

                // If the current number of icons is more than the previous amount then
                // it means that the icon was re-arranged and not dragged outside the applet
                // Therefore we remove the previous icon
                if (dropAction == Qt::MoveAction && m_icons.count() > numberOfPreviousIcons) {
                    m_icons.removeAll(icon);
                    icon->hide();
                    icon->deleteLater();

                    KConfigGroup cg = config();
                    saveState(cg);
                    emit configNeedsSaving();

                    performUiRefactor();
                }
            }
        }
    }

    if (event->type() == QEvent::GraphicsSceneDrop) {
        dropApp(static_cast<QGraphicsSceneDragDropEvent*>(event), object == m_dialogWidget);
        return true;
    }

    return QObject::eventFilter(object, event);
}

void QuicklaunchApplet::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setDropAction(Qt::MoveAction);
    event->setAccepted(event->mimeData()->hasUrls());
}

void QuicklaunchApplet::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setDropAction(Qt::MoveAction);
    event->setAccepted(event->mimeData()->hasUrls());
}

void QuicklaunchApplet::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    dropApp(static_cast<QGraphicsSceneDragDropEvent*>(event),false);
}

void QuicklaunchApplet::addProgram(int index, const QString &url, bool isNewIcon)
{
    if (index < 0 || index > m_icons.size()) {
        index = m_icons.size();
    }

    if (url.isEmpty()) {
        return;
    }

    KUrl appUrl = KUrl(url);
    KIcon icon;
    QString text;
    QString genericName;

    if (appUrl.isLocalFile() && KDesktopFile::isDesktopFile(appUrl.toLocalFile())) {
        KDesktopFile *f = new KDesktopFile(appUrl.toLocalFile());

        text = f->readName();
        icon = KIcon(f->readIcon());
        genericName = f->readGenericName();
        delete f;
    } else {
        icon = KIcon(KMimeType::iconNameForUrl(appUrl));
    }

    if (text.isNull()) {
        text = appUrl.fileName();
    }

    if (icon.isNull()) {
        icon = KIcon("unknown");
    }

    QuicklaunchIcon *container = new QuicklaunchIcon(appUrl, text, icon, genericName, this);
    container->installEventFilter(this);

    m_icons.insert(index, container);

    if (isNewIcon) {
        KConfigGroup cg = config();
        saveState(cg);
        emit configNeedsSaving();
    }
}

void QuicklaunchApplet::clearLayout(QGraphicsLayout *layout)
{
    while (layout->count() > 0) {
        layout->removeAt(0);
    }
}

void QuicklaunchApplet::removeCurrentIcon()
{
    m_icons.removeAll(m_rightClickedIcon);
    m_rightClickedIcon->hide();
    m_rightClickedIcon->deleteLater();
    performUiRefactor();

    KConfigGroup cg = config();
    saveState(cg);
    emit configNeedsSaving();
}

bool QuicklaunchApplet::dropHandler(const int pos, const QMimeData *mimedata)
{
    if (!KUrl::List::canDecode(mimedata)) {
        return false;
    }

    KUrl::List urls = KUrl::List::fromMimeData(mimedata);

    if (urls.isEmpty()) {
        return false;
    }

    //if there are more than one the last is junk
    if (urls.count() > 1) {
        urls.removeLast();
    }

    foreach (const KUrl &url, urls) {
        if (KDesktopFile::isDesktopFile(url.toLocalFile())) {
            addProgram(pos, url.toLocalFile(), true);
        }
    }
    return true;
}


void QuicklaunchApplet::showAddInterface()
{
    if (!m_addDialog) {
        m_addDialog = new KDialog;
        m_addDialog->setCaption(i18n("Add Shortcut"));

        QWidget *widget = new QWidget;
        addUi.setupUi(widget);
        m_addDialog->setMainWidget(widget);
        connect(m_addDialog, SIGNAL(okClicked()), this, SLOT(addAccepted()));
    }
    
    m_addDialog->resize(460,0); //the prefered size seems to be ignored
    
    m_addDialog->show();
}

void QuicklaunchApplet::addAccepted()
{
    int insertplace = m_rightClickedIcon ? m_icons.indexOf(m_rightClickedIcon) : m_icons.size();
    addProgram(insertplace, addUi.urlIcon->url().url(), true);
    performUiRefactor();
}

K_EXPORT_PLASMA_APPLET(quicklaunch, QuicklaunchApplet)

#include "quicklaunchApplet.moc"
