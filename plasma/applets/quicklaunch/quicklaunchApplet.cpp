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
#include <QMouseEvent>
#include <QMimeData>
#include <QToolButton>

#include <KDialog>
#include <KMimeType>
#include <KStandardDirs>
#include <KWindowSystem>

#include <plasma/containment.h>
#include <plasma/dialog.h>
#include <plasma/corona.h>

#include "math.h"

static const int s_defaultIconSize = 16;
static const int s_defaultSpacing = 2;

QuicklaunchApplet::QuicklaunchApplet(QObject *parent, const QVariantList &args)
  : Plasma::Applet(parent, args),
    m_visibleIcons(6),
    m_rowCount(2),
    m_dialogRowCount(2),
    m_dialog(0),
    m_dialogWidget(0),
    m_dialogLayout(0),
    m_addDialog(0),
    m_rightClickedIcon(0),
    m_addAction(0),
    m_removeAction(0)
{
    setHasConfigurationInterface(true);
    setAcceptDrops(true);

    // set our default size here
    resize((m_visibleIcons / m_rowCount) * s_defaultIconSize +
            (s_defaultSpacing * (m_visibleIcons + 1)),
           m_rowCount * 22 + s_defaultSpacing * 3);
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
    m_rowCount = qMax(1, cg.readEntry("rowCount", m_rowCount));
    m_visibleIcons = qMax(1, cg.readEntry("visibleIcons", m_visibleIcons));
    m_dialogRowCount = qMax(1, cg.readEntry("dialogRowCount", m_dialogRowCount));

    // Initialize outer layout
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    // Initialize inner layout
    m_innerLayout = new QuicklaunchLayout(0, m_rowCount);
    m_innerLayout->setContentsMargins(0, 0, 0, 0);
    m_innerLayout->setSpacing(0);
    m_layout->addItem(m_innerLayout);

    // Initial "more icons" arrow
    m_arrow = new Plasma::IconWidget(this);
    m_arrow->setIcon(KIcon("arrow-right"));
    connect(m_arrow, SIGNAL(clicked()), SLOT(showDialog()));

    QStringList desktopFiles = cg.readEntry("iconUrls", QStringList());

    if (desktopFiles.isEmpty()) {
        QStringList defaultApps;
        defaultApps << "konqbrowser" << "dolphin" << "kopete";

        foreach (const QString &defaultApp, defaultApps) {
            KService::Ptr service = KService::serviceByStorageId(defaultApp);
            if (service && service->isValid()) {
                QString path = service->entryPath();
                kDebug() << path;
                if (!path.isEmpty() && QDir::isAbsolutePath(path)) {
                    desktopFiles << path;
                }
            }
        }
        kDebug() << desktopFiles;
    }

    loadPrograms(desktopFiles);
    refactorUi();
}

QSizeF QuicklaunchApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (which == Qt::PreferredSize) {
        QSizeF sizeHint = size();
        if (m_icons.size() > m_visibleIcons) {
            sizeHint.setWidth(sizeHint.height() / m_innerLayout->rowCount() * m_innerLayout->columnCount() + size().height());
        } else {
            sizeHint.setWidth(sizeHint.height() / m_innerLayout->rowCount() * m_innerLayout->columnCount());
        }
        return sizeHint;
    }
    return QGraphicsWidget::sizeHint(which, constraint);
}

void QuicklaunchApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::SizeConstraint) {
        //TODO: don't call so often
        refactorUi();
    }
}

void QuicklaunchApplet::refactorUi()
{
    clearLayout(m_innerLayout);

    if (m_dialogLayout) {
        clearLayout(m_dialogLayout);
        m_dialogLayout->setRowCount(m_dialogRowCount);
    }
    int rowCount;
    int iconWidth;
    if (formFactor() == Plasma::Vertical) {
        rowCount = qMin(m_rowCount, int(size().width()) / (s_defaultIconSize + s_defaultSpacing));
        iconWidth = size().width() / rowCount;
    } else {
        rowCount = qMin(m_rowCount, int(size().height()) / (s_defaultIconSize + s_defaultSpacing));
        iconWidth = qMax(s_defaultIconSize, int(size().height()) / rowCount);
    }

    m_innerLayout->setRowCount(rowCount);
    const QSizeF minSize = QSizeF(iconWidth, iconWidth);
    const QSizeF maxSize = QSizeF(iconWidth, iconWidth);
    int count = 0;
    kDebug() << m_icons.count() << iconWidth << "pixel icons in" << rowCount
             << "rows, with a max of" << m_visibleIcons << "visible";
    foreach (QuicklaunchIcon *icon, m_icons) {
        //icon->setMinimumSize(minSize);
        //icon->setMaximumSize(maxSize);
        icon->resize(minSize);

        if (count < m_visibleIcons || m_visibleIcons == -1) {
            icon->show();
            m_innerLayout->addItem(icon);
        } else if (m_dialogLayout) {
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
        m_arrow->resize(minSize);
        m_layout->addItem(m_arrow);
        m_arrow->show();
    } else {
        m_arrow->hide();
    }

    if (m_dialog) {
        m_dialog->close();
        m_dialogLayout->updateGeometry();
        m_dialog->adjustSize();
    }
    m_innerLayout->updateGeometry();
    m_layout->updateGeometry();
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
        m_dialogLayout = new QuicklaunchLayout(m_dialogWidget, m_dialogRowCount);
        m_dialogWidget->setLayout(m_dialogLayout);
        refactorUi();
        m_dialog->setGraphicsWidget(m_dialogWidget);
    }

    if (m_dialog->isVisible()) {
        m_dialog->hide();
    } else {
        m_dialog->resize(m_dialogLayout->preferredSize().toSize());
        //m_dialog->updateGeometry();
        if(containment() && containment()->corona()) {
            kDebug() << "position:" << containment()->corona()->popupPosition(m_arrow, m_dialog->size()) << "dialog size:" << m_dialog->size() << "layout preferred-size:" << m_dialogLayout->preferredSize().toSize();
            m_dialog->move(containment()->corona()->popupPosition(m_arrow, m_dialog->size()));
        }
        KWindowSystem::setState(m_dialog->winId(), NET::SkipTaskbar);
        //QPoint(popupPosition(m_dialog->sizeHint()).x() + (m_visibleIcons) * size().height() / m_rowCount, 
        //               popupPosition(m_dialog->sizeHint()).y()));
        m_dialog->show();
    }
}

void QuicklaunchApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    uiConfig.setupUi(widget);
    connect(parent, SIGNAL(accepted()), SLOT(configAccepted()));
    uiConfig.rowCount->setValue(m_rowCount);
    uiConfig.dialogRowCount->setValue(m_dialogRowCount);
    uiConfig.dialogRowCount->hide();
    uiConfig.dialogrowLabel->hide();
    uiConfig.icons->setValue(m_visibleIcons);
    parent->addPage(widget, i18n("General"), icon());
}

void QuicklaunchApplet::configAccepted()
{
    bool changed = false;
    int temp = uiConfig.rowCount->value();

    KConfigGroup cg = config();
    if (temp != m_rowCount) {
        m_rowCount = temp;
        cg.writeEntry("rowCount", m_rowCount);
        changed = true;
    }

    temp = uiConfig.icons->value();
    if (temp != m_visibleIcons) {
        m_visibleIcons = temp;
        cg.writeEntry("visibleIcons", m_visibleIcons);
        changed = true;
    }

    temp = uiConfig.dialogRowCount->value();
    if (temp != m_dialogRowCount) {
        m_dialogRowCount = temp;
        cg.writeEntry("dialogRowCount", m_dialogRowCount);
        changed = true;
    }

    if (changed) {
        emit configNeedsSaving();
        refactorUi();
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

    return tempActions;
}

bool QuicklaunchApplet::eventFilter(QObject * object, QEvent * event)
{
    Q_UNUSED(object)
    if (event->type() == QEvent::GraphicsSceneDrop) {
        dropEvent(static_cast<QGraphicsSceneDragDropEvent*>(event));
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
    // Calculate position
    QPointF point = mapFromScene(event->scenePos());
    int rowCount = m_innerLayout->rowCount();
    int cols = static_cast<int>(ceil(1.0 * qMin(m_icons.size(), m_visibleIcons) / rowCount));
    int col = static_cast<int>((round(point.x()) * cols / m_innerLayout->geometry().width()));
    col = (col >= cols) ? col - 1 : col;
    int row = static_cast<int>(floor(point.y() * rowCount / m_innerLayout->geometry().height()));
    row = (row >= m_rowCount) ? row - 1 : row;
    int pos = row * cols + col;

    if (pos >= m_icons.size()) {
        pos = m_icons.size() - 1;
    }

    if (dropHandler(pos, event->mimeData())) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
        saveConfig();
        refactorUi();
    }
}

void QuicklaunchApplet::addProgram(int index, const QString &url)
{
    if (index < 0 || index > m_icons.size()) {
        index = m_icons.size();
    }

    KUrl appUrl = KUrl(url);
    KIcon icon;

    if (appUrl.isLocalFile() && KDesktopFile::isDesktopFile(appUrl.toLocalFile())) {
        KDesktopFile *f = new KDesktopFile(appUrl.toLocalFile());
        icon = KIcon(f->readIcon());
        delete f;
    } else {
        icon = KIcon(KMimeType::iconNameForUrl(appUrl));
    }

    if (icon.isNull()) {
        icon = KIcon("unknown");
    }

    QuicklaunchIcon *container = new QuicklaunchIcon(appUrl, icon, this);
    container->installEventFilter(this);
    m_icons.insert(index, container);
}

void QuicklaunchApplet::loadPrograms(const QStringList &desktopFiles)
{
    foreach (const QString &desktopFile, desktopFiles) {
        addProgram(-1, desktopFile);
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
    refactorUi();
}

bool QuicklaunchApplet::dropHandler(const int pos, const QMimeData *mimedata)
{
    if (!KUrl::List::canDecode(mimedata)) {
        return false;
    }

    KUrl::List urls = KUrl::List::fromMimeData(mimedata);

    if (!urls.count()) {
        return false;
    }

    //if there are more than one the last is junk
    if (urls.count() > 1) {
        urls.removeLast();
    }

    foreach (const KUrl &url, urls) {
        if(KDesktopFile::isDesktopFile(url.toLocalFile())) {
            addProgram(pos, url.toLocalFile());
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
    m_addDialog->show();
}

void QuicklaunchApplet::addAccepted()
{
    int insertplace = m_rightClickedIcon ? m_icons.indexOf(m_rightClickedIcon) : m_icons.size();
    addProgram(insertplace, addUi.urlIcon->url().url());
    refactorUi();
}

K_EXPORT_PLASMA_APPLET(quicklaunch, QuicklaunchApplet)

#include "quicklaunchApplet.moc"
