/***************************************************************************
 *   applet.cpp                                                            *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Sebastian Sauer                                    *
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "applet.h"

#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QGraphicsLayout>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QPainter>
#include <QtGui/QX11Info>


#include <KConfigDialog>
#include <KComboBox>
#include <KWindowSystem>

#include <Solid/Device>

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/extendergroup.h>
#include <plasma/framesvg.h>
#include <plasma/widgets/label.h>
#include <plasma/theme.h>
#include <plasma/dataenginemanager.h>
#include <plasma/dataengine.h>
#include <Plasma/TabBar>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/IconWidget>
#include <Plasma/Dialog>
#include <Plasma/WindowEffects>

#include "config.h"

#include "../core/manager.h"
#include "taskarea.h"

namespace SystemTray
{


K_EXPORT_PLASMA_APPLET(systemtray, Applet)


Manager *Applet::s_manager = 0;
int Applet::s_managerUsage = 0;

Applet::Applet(QObject *parent, const QVariantList &arguments)
    : Plasma::PopupApplet(parent, arguments),
      m_taskArea(0),
      m_background(0)
{
    if (!s_manager) {
        s_manager = new SystemTray::Manager();
    }

    ++s_managerUsage;

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/systemtray");
    m_background->setCacheAllRenderedFrames(true);
    m_taskArea = new TaskArea(this);
    connect(m_taskArea, SIGNAL(toggleHiddenItems()), this, SLOT(togglePopup()));

    m_icons = new Plasma::Svg(this);
    m_icons->setImagePath("widgets/configuration-icons");

    setPopupIcon(QIcon());
    setPassivePopup(false);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setBackgroundHints(NoBackground);
    setHasConfigurationInterface(true);
}

Applet::~Applet()
{
    // stop listening to the manager
    disconnect(s_manager, 0, this, 0);

    // remove the taskArea so we can delete the widgets without it going nuts on us
    delete m_taskArea;

    foreach (Task *task, s_manager->tasks()) {
        // we don't care about the task updates anymore
        disconnect(task, 0, this, 0);

        // delete our widget (if any); some widgets (such as the extender info one)
        // may rely on the applet being around, so we need to delete them here and now
        // while we're still kicking
        delete task->widget(this, false);
    }

    --s_managerUsage;
    if (s_managerUsage < 1) {
        delete s_manager;
        s_manager = 0;
        s_managerUsage = 0;
    }
}

void Applet::init()
{
    connect(s_manager, SIGNAL(taskAdded(SystemTray::Task*)),
            m_taskArea, SLOT(addTask(SystemTray::Task*)));
    //TODO: we re-add the task when it changed: slightly silly!
    connect(s_manager, SIGNAL(taskChanged(SystemTray::Task*)),
            m_taskArea, SLOT(addTask(SystemTray::Task*)));
    connect(s_manager, SIGNAL(taskRemoved(SystemTray::Task*)),
            m_taskArea, SLOT(removeTask(SystemTray::Task*)));

    connect(m_taskArea, SIGNAL(sizeHintChanged(Qt::SizeHint)),
            this, SLOT(propogateSizeHintChange(Qt::SizeHint)));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(checkSizes()));

    QTimer::singleShot(0, this, SLOT(checkDefaultApplets()));
    configChanged();
}

QGraphicsWidget *Applet::graphicsWidget()
{
    return m_taskArea->hiddenTasksWidget();
}

void Applet::configChanged()
{
    KConfigGroup gcg = globalConfig();
    KConfigGroup cg = config();

    const QStringList hiddenTypes = cg.readEntry("hidden", QStringList());
    const QStringList alwaysShownTypes = cg.readEntry("alwaysShown", QStringList());
    m_taskArea->setHiddenTypes(hiddenTypes);
    m_taskArea->setAlwaysShownTypes(alwaysShownTypes);

    m_shownCategories.clear();

    if (cg.readEntry("ShowApplicationStatus", gcg.readEntry("ShowApplicationStatus", true))) {
        m_shownCategories.insert(Task::ApplicationStatus);
    }

    if (cg.readEntry("ShowCommunications", gcg.readEntry("ShowCommunications", true))) {
        m_shownCategories.insert(Task::Communications);
    }

    if (cg.readEntry("ShowSystemServices", gcg.readEntry("ShowSystemServices", true))) {
        m_shownCategories.insert(Task::SystemServices);
    }

    if (cg.readEntry("ShowHardware", gcg.readEntry("ShowHardware", true))) {
        m_shownCategories.insert(Task::Hardware);
    }

    m_shownCategories.insert(Task::UnknownCategory);

    s_manager->loadApplets(cg, this);
    m_taskArea->syncTasks(s_manager->tasks());
    checkSizes();
    setTaskAreaGeometry();
}

void Applet::popupEvent(bool show)
{
    m_taskArea->setShowHiddenItems(show);
}

void Applet::constraintsEvent(Plasma::Constraints constraints)
{
    setBackgroundHints(NoBackground);
    if (constraints & Plasma::FormFactorConstraint) {
        QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        policy.setHeightForWidth(true);
        bool vertical = formFactor() == Plasma::Vertical;

        if (!vertical) {
            policy.setVerticalPolicy(QSizePolicy::Expanding);
        } else {
            policy.setHorizontalPolicy(QSizePolicy::Expanding);
        }

        setSizePolicy(policy);
        m_taskArea->setSizePolicy(policy);
        m_taskArea->setOrientation(vertical ? Qt::Vertical : Qt::Horizontal);
    }

    if (constraints & Plasma::LocationConstraint) {
        m_taskArea->setLocation(location());
    }

    if (constraints & Plasma::SizeConstraint) {
        checkSizes();
    }

    if (constraints & Plasma::ImmutableConstraint) {
        if (m_plasmoidTasksInterface) {
            bool visible = (immutability() == Plasma::UserImmutable);
            m_plasmoidTasksUi.applets->setEnabled(immutability() == Plasma::Mutable);
            m_plasmoidTasksUi.unlockLabel->setVisible(visible);
            m_plasmoidTasksUi.unlockButton->setVisible(visible);
        }
    }

    s_manager->forwardConstraintsEvent(constraints);
}

SystemTray::Manager *Applet::manager() const
{
    return s_manager;
}

QSet<Task::Category> Applet::shownCategories() const
{
    return m_shownCategories;
}

void Applet::setGeometry(const QRectF &rect)
{
    Plasma::Applet::setGeometry(rect);

    if (m_taskArea) {
        setTaskAreaGeometry();
    }
}

void Applet::checkSizes()
{
    Plasma::FormFactor f = formFactor();
    qreal leftMargin, topMargin, rightMargin, bottomMargin;
    m_background->setElementPrefix(QString());
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    m_background->getMargins(leftMargin, topMargin, rightMargin, bottomMargin);

    QSizeF minSize = m_taskArea->effectiveSizeHint(Qt::MinimumSize);
    if (f == Plasma::Horizontal && minSize.height() >= size().height() - topMargin - bottomMargin) {
        m_background->setElementPrefix(QString());
        m_background->setEnabledBorders(Plasma::FrameSvg::LeftBorder | Plasma::FrameSvg::RightBorder);
        m_background->setElementPrefix("lastelements");
        m_background->setEnabledBorders(Plasma::FrameSvg::LeftBorder | Plasma::FrameSvg::RightBorder);
        setContentsMargins(leftMargin, 0, rightMargin, 0);
    } else if (f == Plasma::Vertical && minSize.width() >= size().width() - leftMargin - rightMargin) {
        m_background->setElementPrefix(QString());
        m_background->setEnabledBorders(Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::BottomBorder);
        m_background->setElementPrefix("lastelements");
        m_background->setEnabledBorders(Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::BottomBorder);
        setContentsMargins(0, topMargin, 0, bottomMargin);
    } else {
        m_background->setElementPrefix(QString());
        m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        m_background->setElementPrefix("lastelements");
        m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);
    }

    QSizeF preferredSize = m_taskArea->effectiveSizeHint(Qt::PreferredSize);
    preferredSize.setWidth(preferredSize.width() + leftMargin + rightMargin);
    preferredSize.setHeight(preferredSize.height() + topMargin + bottomMargin);
    setPreferredSize(preferredSize);

    QSizeF actualSize = size();

    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    if (f == Plasma::Horizontal) {
        setMinimumSize(preferredSize.width(), 0);
        setMaximumSize(preferredSize.width(), QWIDGETSIZE_MAX);
    } else if (f == Plasma::Vertical) {
        setMinimumSize(0, preferredSize.height());
        setMaximumSize(QWIDGETSIZE_MAX, preferredSize.height());
    } else if (actualSize.width() < preferredSize.width() ||
               actualSize.height() < preferredSize.height()) {
        setMinimumSize(22, 22);

        QSizeF constraint;
        if (actualSize.width() > actualSize.height()) {
            constraint = QSize(actualSize.width() - leftMargin - rightMargin, -1);
        } else {
            constraint = QSize(-1, actualSize.height() - topMargin - bottomMargin);
        }

        preferredSize = m_taskArea->effectiveSizeHint(Qt::PreferredSize, actualSize);
        preferredSize.setWidth(qMax(actualSize.width(), preferredSize.width()));
        preferredSize.setHeight(qMax(actualSize.height(), preferredSize.height()));

        resize(preferredSize);
    }
}


void Applet::setTaskAreaGeometry()
{
    qreal leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

    QRectF taskAreaRect(rect());
    taskAreaRect.moveLeft(leftMargin);
    taskAreaRect.moveTop(topMargin);
    taskAreaRect.setWidth(taskAreaRect.width() - leftMargin - rightMargin);
    taskAreaRect.setHeight(taskAreaRect.height() - topMargin - bottomMargin);

    m_taskArea->setGeometry(taskAreaRect);
}

void Applet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)

    if (m_taskArea->leftEasement() > 0) {
        Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::AllBorders;
        Plasma::FrameSvg::EnabledBorders oldBorders = m_background->enabledBorders();

        m_background->setElementPrefix(QString());

        QRectF iconRect;
        qreal left, top, right, bottom;
        //we need also the border we're going to disable
        m_background->getMargins(left, top, right, bottom);

        switch (formFactor()) {
        case Plasma::Vertical:
            borders &= ~Plasma::FrameSvg::BottomBorder;
            iconRect = QRect(left, 0, size().width() - left - right, m_taskArea->leftEasement() + top);
            break;
        case Plasma::Horizontal:
        default:
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                borders &= ~Plasma::FrameSvg::LeftBorder;
                iconRect = QRect(size().width() - m_taskArea->leftEasement() - right,
                                 top,
                                 m_taskArea->leftEasement(),
                                 size().height() - top - bottom);
            } else {
                borders &= ~Plasma::FrameSvg::RightBorder;
                iconRect = QRect(0, top, m_taskArea->leftEasement() + left, size().height() - top - bottom);
            }
            break;
        }

        m_background->setEnabledBorders(borders);

        m_background->resizeFrame(iconRect.size());
        m_background->paintFrame(painter, iconRect.topLeft());

        m_background->setEnabledBorders(oldBorders);
    }

    QRect normalRect = rect().toRect();
    QRect lastRect(normalRect);
    m_background->setElementPrefix("lastelements");

    if (formFactor() == Plasma::Vertical) {
        const int rightEasement = m_taskArea->rightEasement() + m_background->marginSize(Plasma::BottomMargin);
        normalRect.setY(m_taskArea->leftEasement());
        normalRect.setBottom(normalRect.bottom() - rightEasement);

        lastRect.setY(normalRect.bottom() + 1);
        lastRect.setHeight(rightEasement);
    } else if (QApplication::layoutDirection() == Qt::RightToLeft) {
        const int rightEasement = m_taskArea->rightEasement() + m_background->marginSize(Plasma::LeftMargin);
        normalRect.setWidth(normalRect.width() - m_taskArea->leftEasement());
        normalRect.setLeft(rightEasement);

        lastRect.setX(0);
        lastRect.setWidth(rightEasement);
    } else {
        const int rightEasement = m_taskArea->rightEasement() + m_background->marginSize(Plasma::RightMargin);
        normalRect.setX(m_taskArea->leftEasement());
        normalRect.setWidth(normalRect.width() - rightEasement);

        lastRect.setX(normalRect.right() + 1);
        lastRect.setWidth(rightEasement);
    }

    QRect r = normalRect.united(lastRect);

    painter->save();

    m_background->setElementPrefix(QString());
    m_background->resizeFrame(r.size());
    if (m_taskArea->rightEasement() > 0) {
        painter->setClipRect(normalRect);
    }
    m_background->paintFrame(painter, r, QRectF(QPointF(0, 0), r.size()));

    if (m_taskArea->rightEasement() > 0) {
        m_background->setElementPrefix("lastelements");
        m_background->resizeFrame(r.size());
        painter->setClipRect(lastRect);
        m_background->paintFrame(painter, r, QRectF(QPointF(0, 0), r.size()));

        if (formFactor() == Plasma::Vertical && m_background->hasElement("horizontal-separator")) {
            QSize s = m_background->elementRect("horizontal-separator").size().toSize();
            m_background->paint(painter, QRect(lastRect.topLeft() - QPoint(0, s.height() / 2),
                                                QSize(lastRect.width(), s.height())), "horizontal-separator");
        } else if (QApplication::layoutDirection() == Qt::RightToLeft && m_background->hasElement("vertical-separator")) {
            QSize s = m_background->elementRect("vertical-separator").size().toSize();
            m_background->paint(painter, QRect(lastRect.topRight() - QPoint(s.width() / 2, 0),
                                                QSize(s.width(), lastRect.height())), "vertical-separator");
        } else if (m_background->hasElement("vertical-separator")) {
            QSize s = m_background->elementRect("vertical-separator").size().toSize();
            m_background->paint(painter, QRect(lastRect.topLeft() - QPoint(s.width() / 2, 0),
                                                QSize(s.width(), lastRect.height())), "vertical-separator");
        }
    }

    painter->restore();
}


void Applet::propogateSizeHintChange(Qt::SizeHint which)
{
    checkSizes();
    emit sizeHintChanged(which);
}


void Applet::createConfigurationInterface(KConfigDialog *parent)
{
    if (!m_autoHideInterface) {
        KConfigGroup gcg = config();
        KConfigGroup cg = config();

        m_notificationInterface = new QWidget();
        m_autoHideInterface = new QWidget();
        m_plasmoidTasksInterface = new QWidget();

        m_notificationUi.setupUi(m_notificationInterface.data());

        m_notificationUi.showApplicationStatus->setChecked(cg.readEntry("ShowApplicationStatus",
                                                           gcg.readEntry("ShowApplicationStatus", true)));
        m_notificationUi.showCommunications->setChecked(cg.readEntry("ShowCommunications",
                                                        gcg.readEntry("ShowCommunications", true)));
        m_notificationUi.showSystemServices->setChecked(cg.readEntry("ShowSystemServices",
                                                        gcg.readEntry("ShowSystemServices", true)));
        m_notificationUi.showHardware->setChecked(cg.readEntry("ShowHardware",
                                                  gcg.readEntry("ShowHardware", true)));

        m_autoHideUi.setupUi(m_autoHideInterface.data());

        m_plasmoidTasksUi.setupUi(m_plasmoidTasksInterface.data());

        QAction *unlockAction = 0;
        if (containment() && containment()->corona()) {
            unlockAction = containment()->corona()->action("lock widgets");
        }
        if (unlockAction) {
            disconnect(m_plasmoidTasksUi.unlockButton, SIGNAL(clicked()), unlockAction, SLOT(trigger()));
            connect(m_plasmoidTasksUi.unlockButton, SIGNAL(clicked()), unlockAction, SLOT(trigger()));
        }


        connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        parent->addPage(m_notificationInterface.data(), i18n("Information"),
                        "preferences-desktop-notification",
                        i18n("Choose which information to show"));
        parent->addPage(m_autoHideInterface.data(), i18n("Auto Hide"), "window-suppressed");
        parent->addPage(m_plasmoidTasksInterface.data(), i18n("Plasma Widgets"), "plasma");

        bool visible = (immutability() == Plasma::UserImmutable);
        m_plasmoidTasksUi.applets->setEnabled(immutability() == Plasma::Mutable);
        m_plasmoidTasksUi.unlockLabel->setVisible(visible);
        m_plasmoidTasksUi.unlockButton->setVisible(visible);
    }

    m_autoHideUi.icons->clear();
    m_plasmoidTasksUi.applets->clear();

    QMultiMap<QString, const Task *> sortedTasks;
    foreach (const Task *task, s_manager->tasks()) {
        if (!m_shownCategories.contains(task->category())) {
             continue;
        }

        if (!task->isHideable()) {
            continue;
        }

        sortedTasks.insert(task->name(), task);
    }

    foreach (const Task *task, sortedTasks) {
        QTreeWidgetItem *listItem = new QTreeWidgetItem(m_autoHideUi.icons);
        KComboBox *itemCombo = new KComboBox(m_autoHideUi.icons);
        listItem->setText(0, task->name());
        listItem->setIcon(0, task->icon());
        listItem->setFlags(Qt::ItemIsEnabled);
        listItem->setData(0, Qt::UserRole, task->typeId());

        itemCombo->addItem(i18nc("Item will be automatically shown or hidden from the systray", "Auto"));
        itemCombo->addItem(i18nc("Item is never visible in the systray", "Hidden"));
        itemCombo->addItem(i18nc("Item is always visible in the systray", "Always Visible"));

        if (task->hidden() & Task::UserHidden) {
            itemCombo->setCurrentIndex(1);
        } else if (m_taskArea->alwaysShownTypes().contains(task->typeId())) {
            itemCombo->setCurrentIndex(2);
        } else {
            itemCombo->setCurrentIndex(0);
        }
        m_autoHideUi.icons->setItemWidget(listItem, 1, itemCombo);

        m_autoHideUi.icons->addTopLevelItem(listItem);
    }

    QStringList ownApplets = s_manager->applets(this);
    foreach (const KPluginInfo &info, Plasma::Applet::listAppletInfo()) {
        KService::Ptr service = info.service();
        if (service->property("X-Plasma-NotificationArea", QVariant::Bool).toBool()) {
            QListWidgetItem *listItem = new QListWidgetItem();
            listItem->setText(service->name());
            listItem->setIcon(KIcon(service->icon()));
            listItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            listItem->setData(Qt::UserRole, info.pluginName());
            listItem->setCheckState(ownApplets.contains(info.pluginName()) ? Qt::Checked : Qt::Unchecked);
            m_plasmoidTasksUi.applets->addItem(listItem);
        }
    }
}

void Applet::configAccepted()
{
    QStringList hiddenTypes;
    QStringList alwaysShownTypes;
    QTreeWidget *hiddenList = m_autoHideUi.icons;
    for (int i = 0; i < hiddenList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = hiddenList->topLevelItem(i);
        KComboBox *itemCombo = static_cast<KComboBox *>(hiddenList->itemWidget(item, 1));
        //kDebug() << (item->checkState() == Qt::Checked) << item->data(Qt::UserRole).toString();
        //Always hidden
        if (itemCombo->currentIndex() == 1) {
            hiddenTypes << item->data(0, Qt::UserRole).toString();
        //Always visible
        } else if (itemCombo->currentIndex() == 2) {
            alwaysShownTypes << item->data(0, Qt::UserRole).toString();
        }
    }

    KConfigGroup cg = config();
    cg.writeEntry("hidden", hiddenTypes);
    cg.writeEntry("alwaysShown", alwaysShownTypes);

    cg.writeEntry("ShowApplicationStatus", m_notificationUi.showApplicationStatus->isChecked());
    cg.writeEntry("ShowCommunications", m_notificationUi.showCommunications->isChecked());
    cg.writeEntry("ShowSystemServices", m_notificationUi.showSystemServices->isChecked());
    cg.writeEntry("ShowHardware", m_notificationUi.showHardware->isChecked());

    QStringList applets = s_manager->applets(this);
    for (int i = 0; i < m_plasmoidTasksUi.applets->count(); ++i) {
        QListWidgetItem * item = m_plasmoidTasksUi.applets->item(i);
        QString appletName = item->data(Qt::UserRole).toString();

        if (item->checkState() != Qt::Unchecked && !applets.contains(appletName)) {
            s_manager->addApplet(appletName, this);
        }

        if (item->checkState() == Qt::Checked) {
            applets.removeAll(appletName);
        }
    }

    foreach (const QString &appletName, applets) {
        s_manager->removeApplet(appletName, this);
    }

    emit configNeedsSaving();
}

void Applet::checkDefaultApplets()
{
    if (config().readEntry("DefaultAppletsAdded", false)) {
        return;
    }


    QStringList applets = s_manager->applets(this);
    if (!applets.contains("notifier")) {
        s_manager->addApplet("notifier", this);
    }

    if (!applets.contains("notifications")) {
        s_manager->addApplet("notifications", this);
    }

    if (!applets.contains("battery")) {
        Plasma::DataEngineManager *engines = Plasma::DataEngineManager::self();
        Plasma::DataEngine *power = engines->loadEngine("powermanagement");
        if (power) {
            const QStringList &batteries = power->query("Battery")["sources"].toStringList();
            if (!batteries.isEmpty()) {
                s_manager->addApplet("battery", this);
            }
        }
        engines->unloadEngine("powermanagement");
    }

    config().writeEntry("DefaultAppletsAdded", false);
}

}

#include "applet.moc"
