/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
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

// Own
#include "tasks.h"
#include "windowtaskitem.h"
#include "taskgroupitem.h"
#include "ui_tasksConfig.h"

//Taskmanager
#include <taskmanager/taskgroup.h>
#include <taskmanager/taskitem.h>

// KDE
#include <KConfigDialog>
#include <KWindowSystem>

// Qt
#include <QTimeLine>
#include <QGraphicsScene>
#include <QGraphicsLinearLayout>
#include <QVariant>

// Plasma
#include <Plasma/Containment>
#include <Plasma/FrameSvg>
#include <Plasma/Theme>

Tasks::Tasks(QObject* parent, const QVariantList &arguments)
     : Plasma::Applet(parent, arguments),
       m_taskItemBackground(0),
       m_colorScheme(0),
       m_leftMargin(0),
       m_topMargin(0),
       m_rightMargin(0),
       m_bottomMargin(0),
       m_rootGroupItem(0),
       m_groupManager(0),
       m_groupModifierKey(Qt::AltModifier)
{
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    m_screenTimer.setSingleShot(true);
    m_screenTimer.setInterval(300);
    resize(500, 58);

    setAcceptDrops(true);

}

Tasks::~Tasks()
{
    delete m_colorScheme;
    delete m_groupManager;
}

void Tasks::init()
{
    //kDebug();

    m_groupManager = new TaskManager::GroupManager(this);
    Plasma::Containment* appletContainment = containment();
    if (appletContainment) {
        m_groupManager->setScreen(appletContainment->screen());
    }

    //FIXME: the order of creation and setting of items in this method is both fragile (from
    // personal experience tinking with it) and convoluted. It should be possible to
    // set up the GroupManager firt, and *then* create the root TaskGroupItem.

   // connect(m_groupManager, SIGNAL(reload()), this, SLOT(reload()));
    connect(this, SIGNAL(settingsChanged()), m_groupManager, SLOT(reconnect()));

    m_rootGroupItem = new TaskGroupItem(this, this, false);
    m_rootGroupItem->expand();
    m_rootGroupItem->setGroup(m_groupManager->rootGroup());

    /*
    foreach (TaskManager::AbstractGroupableItem *item, m_groupManager->rootGroup()->members()) {
        kDebug() << item->name();
    }
    */

    connect(m_rootGroupItem, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(changeSizeHint(Qt::SizeHint)));

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    //like in Qt's designer
    //TODO : Qt's bug??
    setMaximumSize(INT_MAX,INT_MAX);

    layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    //TODO : Qt's bug??
    layout->setMaximumSize(INT_MAX,INT_MAX);
    layout->setOrientation(Qt::Vertical);
    layout->addItem(m_rootGroupItem);


    setLayout(layout);

    KConfigGroup cg = config();

    m_groupManager->setShowOnlyCurrentDesktop( cg.readEntry("showOnlyCurrentDesktop", false));
    m_groupManager->setShowOnlyCurrentScreen( cg.readEntry("showOnlyCurrentScreen", false));
    m_groupManager->setShowOnlyMinimized( cg.readEntry("showOnlyMinimized", false));
    m_groupManager->setOnlyGroupWhenFull(cg.readEntry("groupWhenFull", true));
    m_showTooltip = cg.readEntry("showTooltip", true);

    m_groupManager->setGroupingStrategy( static_cast<TaskManager::GroupManager::TaskGroupingStrategy>(cg.readEntry("groupingStrategy", static_cast<int>(TaskManager::GroupManager::ProgramGrouping))));

    m_groupManager->setSortingStrategy( static_cast<TaskManager::GroupManager::TaskSortingStrategy>(cg.readEntry("sortingStrategy", static_cast<int>(TaskManager::GroupManager::AlphaSorting))));
    m_rootGroupItem->setMaxRows( cg.readEntry("maxRows", 2));
    m_rootGroupItem->setForceRows( cg.readEntry("forceRows", false));

    emit settingsChanged();
}

void Tasks::reload()
{
    m_rootGroupItem->reload();
}

TaskManager::GroupManager &Tasks::groupManager() const
{
    return *m_groupManager;
}

Qt::KeyboardModifiers Tasks::groupModifierKey() const
{
    return m_groupModifierKey;
}



void Tasks::constraintsEvent(Plasma::Constraints constraints)
{
    //kDebug();
    if (m_groupManager && constraints & Plasma::ScreenConstraint) {
        Plasma::Containment* appletContainment = containment();
        if (appletContainment) {
            m_groupManager->setScreen(appletContainment->screen());
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        adjustGroupingStrategy();
    }

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    emit constraintsChanged(constraints);
}

Plasma::FrameSvg* Tasks::itemBackground()
{
    if (!m_taskItemBackground) {
        m_taskItemBackground = new Plasma::FrameSvg(this);
        m_taskItemBackground->setImagePath("widgets/tasks");
        m_taskItemBackground->setCacheAllRenderedFrames(true);
    }

    return m_taskItemBackground;
}

void Tasks::resizeItemBackground(const QSizeF &size)
{
  //kDebug();
    if (!m_taskItemBackground) {
        itemBackground();

        if (!m_taskItemBackground) {
            //kDebug() << "Error1";
            return;
        }
    }

    if (m_taskItemBackground->frameSize() == size) {
        //kDebug() << "Error2";
        return;
    }

    m_taskItemBackground->resizeFrame(size);

    QString oldPrefix = m_taskItemBackground->prefix();
    m_taskItemBackground->setElementPrefix("normal");
    //get the margins now
    m_taskItemBackground->getMargins(m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin);
    //if the task height is too little shrink the top and bottom margins
    if (size.height() - m_topMargin - m_bottomMargin < KIconLoader::SizeSmall) {
        m_topMargin = m_bottomMargin = qMax(1, int((size.height() - KIconLoader::SizeSmall)/2));
    }
    m_taskItemBackground->setElementPrefix(oldPrefix);
}

KColorScheme *Tasks::colorScheme()
{
    if (!m_colorScheme) {
        m_colorScheme = new KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme());
    }

    return m_colorScheme;
}


QSizeF Tasks::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (m_rootGroupItem && which == Qt::PreferredSize) {
        return m_rootGroupItem->preferredSize();
    } else {
        return Plasma::Applet::sizeHint(which, constraint);
    }
}

void Tasks::adjustGroupingStrategy()
{
    //FIXME: should use AbstractTaskItem::basicPreferredSize() but it seems to cause crashes
    //QSize itemSize = QSize(300, 30);
    //m_groupManager->setFullLimit(((size().width()*size().height()) / (itemSize.width()*itemSize.height())));
    //kDebug() << ((size().width()*size().height()) / (itemSize.width()*itemSize.height()));

    m_groupManager->setFullLimit(rootGroupItem()->optimumCapacity());
}

void Tasks::changeSizeHint(Qt::SizeHint which)
{
    emit sizeHintChanged(which);
    adjustGroupingStrategy();
}

void Tasks::createConfigurationInterface(KConfigDialog *parent)
{
     QWidget *widget = new QWidget;
     m_ui.setupUi(widget);
     connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
     connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
     parent->addPage(widget, i18n("General"), icon());

    m_ui.showTooltip->setChecked(m_showTooltip);
    m_ui.showOnlyCurrentDesktop->setChecked(m_groupManager->showOnlyCurrentDesktop());
    m_ui.showOnlyCurrentScreen->setChecked(m_groupManager->showOnlyCurrentScreen());
    m_ui.showOnlyMinimized->setChecked(m_groupManager->showOnlyMinimized());
    m_ui.fillRows->setChecked(m_rootGroupItem->forceRows());

    m_ui.groupingStrategy->addItem(i18n("Do Not Group"),QVariant(TaskManager::GroupManager::NoGrouping));
    m_ui.groupingStrategy->addItem(i18n("Manually"),QVariant(TaskManager::GroupManager::ManualGrouping));
    m_ui.groupingStrategy->addItem(i18n("By Program Name"),QVariant(TaskManager::GroupManager::ProgramGrouping));

    connect(m_ui.groupingStrategy, SIGNAL(currentIndexChanged(int)), this, SLOT(dialogGroupingChanged(int)));

    switch (m_groupManager->groupingStrategy()) {
        case TaskManager::GroupManager::NoGrouping:
            m_ui.groupingStrategy->setCurrentIndex(0);
            break;
        case TaskManager::GroupManager::ManualGrouping:
            m_ui.groupingStrategy->setCurrentIndex(1);
            break;
        case TaskManager::GroupManager::ProgramGrouping:
            m_ui.groupingStrategy->setCurrentIndex(2);
            break;
        default:
             m_ui.groupingStrategy->setCurrentIndex(-1);
    }
    kDebug() << m_groupManager->groupingStrategy();

    m_ui.groupWhenFull->setChecked(m_groupManager->onlyGroupWhenFull());


    m_ui.sortingStrategy->addItem(i18n("Do Not Sort"),QVariant(TaskManager::GroupManager::NoSorting));
    m_ui.sortingStrategy->addItem(i18n("Manually"),QVariant(TaskManager::GroupManager::ManualSorting));
    m_ui.sortingStrategy->addItem(i18n("Alphabetically"),QVariant(TaskManager::GroupManager::AlphaSorting));
    m_ui.sortingStrategy->addItem(i18n("By Desktop"),QVariant(TaskManager::GroupManager::DesktopSorting));


    switch (m_groupManager->sortingStrategy()) {
        case TaskManager::GroupManager::NoSorting:
            m_ui.sortingStrategy->setCurrentIndex(0);
            break;
        case TaskManager::GroupManager::ManualSorting:
            m_ui.sortingStrategy->setCurrentIndex(1);
            break;
        case TaskManager::GroupManager::AlphaSorting:
            m_ui.sortingStrategy->setCurrentIndex(2);
            break;
        case TaskManager::GroupManager::DesktopSorting:
            m_ui.sortingStrategy->setCurrentIndex(3);
            break;
        default:
             m_ui.sortingStrategy->setCurrentIndex(-1);
    }
 //   kDebug() << m_groupManager->sortingStrategy();
    m_ui.maxRows->setValue(m_rootGroupItem->maxRows());
}

void Tasks::dialogGroupingChanged(int index)
{
     m_ui.groupWhenFull->setEnabled(static_cast<TaskManager::GroupManager::TaskGroupingStrategy>(m_ui.groupingStrategy->itemData(index).toInt()) == TaskManager::GroupManager::ProgramGrouping);
}

void Tasks::configAccepted()
{
    kDebug();
    bool changed = false;

    if (m_groupManager->showOnlyCurrentDesktop() != (m_ui.showOnlyCurrentDesktop->isChecked())) {
        m_groupManager->setShowOnlyCurrentDesktop(!m_groupManager->showOnlyCurrentDesktop());
        KConfigGroup cg = config();
        cg.writeEntry("showOnlyCurrentDesktop", m_groupManager->showOnlyCurrentDesktop());
        changed = true;
    }
    if (m_groupManager->showOnlyCurrentScreen() != (m_ui.showOnlyCurrentScreen->isChecked())) {
        m_groupManager->setShowOnlyCurrentScreen(!m_groupManager->showOnlyCurrentScreen());
        KConfigGroup cg = config();
        cg.writeEntry("showOnlyCurrentScreen", m_groupManager->showOnlyCurrentScreen());
        changed = true;
    }
    if (m_groupManager->showOnlyMinimized() != (m_ui.showOnlyMinimized->isChecked())) {
        m_groupManager->setShowOnlyMinimized(!m_groupManager->showOnlyMinimized());
        KConfigGroup cg = config();
        cg.writeEntry("showOnlyMinimized", m_groupManager->showOnlyMinimized());
        changed = true;
    }

    if (m_groupManager->groupingStrategy() != (m_ui.groupingStrategy->currentIndex())) {
        m_groupManager->setGroupingStrategy(static_cast<TaskManager::GroupManager::TaskGroupingStrategy>(m_ui.groupingStrategy->itemData(m_ui.groupingStrategy->currentIndex()).toInt()));
        KConfigGroup cg = config();
        cg.writeEntry("groupingStrategy", static_cast<int>(m_groupManager->groupingStrategy()));
        changed = true;
    }

    if (m_groupManager->onlyGroupWhenFull() != m_ui.groupWhenFull->isChecked()) {
        adjustGroupingStrategy();
        m_groupManager->setOnlyGroupWhenFull(m_ui.groupWhenFull->isChecked());
        KConfigGroup cg = config();
        cg.writeEntry("groupWhenFull", m_groupManager->onlyGroupWhenFull());
        changed = true;
    }

    if (m_groupManager->sortingStrategy() != (m_ui.sortingStrategy->currentIndex())) {
        m_groupManager->setSortingStrategy(static_cast<TaskManager::GroupManager::TaskSortingStrategy>(m_ui.sortingStrategy->itemData(m_ui.sortingStrategy->currentIndex()).toInt()));
        KConfigGroup cg = config();
        cg.writeEntry("sortingStrategy", static_cast<int>(m_groupManager->sortingStrategy()));
        changed = true;
    }

    if (m_rootGroupItem->maxRows() != (m_ui.maxRows->value())) {
        m_rootGroupItem->setMaxRows(m_ui.maxRows->value());
        KConfigGroup cg = config();
        cg.writeEntry("maxRows", m_rootGroupItem->maxRows());
        changed = true;
    }

    if (m_rootGroupItem->forceRows() != m_ui.fillRows->isChecked()) {
        m_rootGroupItem->setForceRows(m_ui.fillRows->isChecked());
        KConfigGroup cg = config();
        cg.writeEntry("forceRows", m_rootGroupItem->forceRows());
        changed = true;
    }

    if (m_showTooltip != (m_ui.showTooltip->checkState() == Qt::Checked)) {
        m_showTooltip = !m_showTooltip;
        KConfigGroup cg = config();
        cg.writeEntry("showTooltip", m_showTooltip);
        changed = true;
    }

    if (changed) {
        emit settingsChanged();
        emit configNeedsSaving();
        update();
    }
}

bool Tasks::showTooltip() const
{
    return m_showTooltip;
}

void Tasks::needsVisualFocus()
{
    emit activate();
}

void Tasks::themeRefresh()
{
    delete m_taskItemBackground;
    m_taskItemBackground = 0;

    delete m_colorScheme;
    m_colorScheme = 0;
}



TaskGroupItem* Tasks::rootGroupItem()
{
    return m_rootGroupItem;
}


K_EXPORT_PLASMA_APPLET(tasks, Tasks)

#include "tasks.moc"
