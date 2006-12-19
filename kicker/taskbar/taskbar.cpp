/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2004 Sebastian Wolff
Copyright (c) 2005 Aaron Seigo <aseigo@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <math.h>

#include <QApplication>
#include <QBitmap>
#include <qdesktopwidget.h>
#include <QLayout>
#include <QPainter>
#include <QStringList>
#include <q3tl.h>
#include <QWheelEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QMouseEvent>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kglobalaccel.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kwindowlistmenu.h>
#include <QDBusInterface>
#include "kshadowengine.h"
#include "kshadowsettings.h"
#include "kickerSettings.h"
#include "taskbarsettings.h"
#include "taskcontainer.h"
#include "taskmanager.h"
#include "simplebutton.h"

#include "taskbar.h"
#include "taskbar.moc"


TaskBar::TaskBar( QWidget *parent )
    : QWidget( parent ),
      blocklayout(true),
      m_showAllWindows(false),
      m_currentScreen(-1),
      m_showOnlyCurrentScreen(false),
      m_sortByDesktop(false),
      m_showIcon(false),
      arrowType(Qt::LeftArrow),
      m_textShadowEngine(0),
      m_direction(Plasma::Up),
      m_showWindowListButton(true),
      m_windowListButton(0),
      m_windowListMenu(0)
{
    // init
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    // setup animation frames
    for (int i = 1; i < 11; i++)
    {
        frames.append(new QPixmap(KStandardDirs::locate("data",
                                                        "kicker/pics/disk" + QString::number(i) + ".png")));
    }

    m_layout = new QBoxLayout( QApplication::isRightToLeft() ?
                                QBoxLayout::RightToLeft :
                                QBoxLayout::LeftToRight,
                                this );
    m_layout->setMargin( 0 );

    m_container = new Panner(this);
    m_container->setFrameStyle( QFrame::NoFrame );

    m_layout->addWidget(m_container);

    // configure
    configure();

    // connect manager
    connect(TaskManager::self(), SIGNAL(taskAdded(Task::TaskPtr)),
            this, SLOT(add(Task::TaskPtr)));
    connect(TaskManager::self(), SIGNAL(taskRemoved(Task::TaskPtr)),
            this, SLOT(remove(Task::TaskPtr)));
    connect(TaskManager::self(), SIGNAL(startupAdded(Startup::StartupPtr)),
            this, SLOT(add(Startup::StartupPtr)));
    connect(TaskManager::self(), SIGNAL(startupRemoved(Startup::StartupPtr)),
            this, SLOT(remove(Startup::StartupPtr)));
    connect(TaskManager::self(), SIGNAL(desktopChanged(int)),
            this, SLOT(desktopChanged(int)));
    connect(TaskManager::self(), SIGNAL(windowChanged(Task::TaskPtr)),
            this, SLOT(windowChanged(Task::TaskPtr)));
#ifdef __GNUC__
#warning dcop signal
#endif    
#if 0
    connectDCOPSignal("", "", "kdeTaskBarConfigChanged()",
                      "configChanged()", false);
#endif

    isGrouping = shouldGroup();

    // register existant tasks
    foreach (Task::TaskPtr task, TaskManager::self()->tasks())
    {
        add(task);
    }

    // register existant startups
    foreach (Startup::StartupPtr startup, TaskManager::self()->startups())
    {
        add(startup);
    }

    blocklayout = false;

    connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), SLOT(slotSettingsChanged(int)));
    keys = new KActionCollection( this );
    KActionCollection* actionCollection = keys;
    KAction* a = 0L;
#include "taskbarbindings.cpp"
    keys->readSettings();
}

TaskBar::~TaskBar()
{
    qDeleteAll(containers);
    qDeleteAll(frames);
    delete m_windowListMenu;
}

QSize TaskBar::sizeHint() const
{
    QFontMetrics fm(KGlobalSettings::taskbarFont());
    int minButtonHeight = fm.height() > TaskBarSettings::minimumButtonHeight() ?
                          fm.height() : TaskBarSettings::minimumButtonHeight();

    return QSize(BUTTON_MIN_WIDTH, minButtonHeight);
}

QSize TaskBar::sizeHint( Plasma::Position p, QSize maxSize) const
{
    // number of rows simply depends on our height which is either the
    // minimum button height or the height of the font in use, whichever is
    // largest
    QFontMetrics fm(KGlobalSettings::taskbarFont());
    int minButtonHeight = fm.height() > TaskBarSettings::minimumButtonHeight() ?
                          fm.height() : TaskBarSettings::minimumButtonHeight();

    if ( p == Plasma::Left || p == Plasma::Right )
    {
        int actualMax = minButtonHeight * containerCount() + (m_showWindowListButton ?
                                                              WINDOWLISTBUTTON_SIZE : 0);

        if (containerCount() == 0)
        {
            actualMax = minButtonHeight;
        }

        if (actualMax > maxSize.height())
        {
            return maxSize;
        }
        return QSize( maxSize.width(), actualMax );
    }
    else
    {
        int rows = KickerSettings::conserveSpace() ?
                   contentsRect().height() / minButtonHeight :
                   1;
        if (rows < 1)
        {
            rows = 1;
        }

        int maxWidth = TaskBarSettings::maximumButtonWidth();
        if (maxWidth == 0)
        {
            maxWidth = BUTTON_MAX_WIDTH;
        }

        int actualMax = maxWidth * (containerCount() / rows);

        if (containerCount() % rows > 0)
        {
            actualMax += maxWidth;
        }
        if (containerCount() == 0)
        {
            actualMax = maxWidth;
        }
        if (containerCount() == 0)
        {
            actualMax = maxWidth;
        }

        if (actualMax > maxSize.width())
        {
           return maxSize;
        }
        return QSize( actualMax, maxSize.height() );
    }
}

void TaskBar::configure()
{
    bool wasShowWindows = m_showAllWindows;
    bool wasSortByDesktop = m_sortByDesktop;
    bool wasShowIcon = m_showIcon;

    setFont(TaskBarSettings::taskbarFont());
    m_showWindowListButton = TaskBarSettings::showWindowListBtn();

    if (!m_showWindowListButton)
    {
        delete m_windowListButton;
        m_windowListButton = 0;
        delete m_windowListMenu;
        m_windowListMenu = 0;
    }
    else if (m_windowListButton == 0)
    {
        // window list button
        m_windowListButton = new SimpleButton(this);
        m_windowListMenu= new KWindowListMenu;
        connect(m_windowListButton, SIGNAL(pressed()),
                SLOT(showWindowListMenu()));
        connect(m_windowListMenu, SIGNAL(aboutToHide()),
                SLOT(windowListMenuAboutToHide()));

        // geometry
        QString icon;
        switch (m_direction)
        {
            case Plasma::Up:
                icon = "1uparrow";
                m_windowListButton->setMaximumHeight(BUTTON_MAX_WIDTH);
                break;
            case Plasma::Down:
                icon = "1downarrow";
                m_windowListButton->setMaximumHeight(BUTTON_MAX_WIDTH);
                break;
            case Plasma::Left:
                icon = "1leftarrow";
                m_windowListButton->setMaximumWidth(BUTTON_MAX_WIDTH);
                break;
            case Plasma::Right:
                icon = "1rightarrow";
                m_windowListButton->setMaximumWidth(BUTTON_MAX_WIDTH);
                break;
	    case Plasma::Floating:
		icon = "1uparrow";
		m_windowListButton->setMaximumWidth(BUTTON_MAX_WIDTH);
		break;
        }

        m_windowListButton->setIcon(kapp->iconLoader()->loadIcon(icon,
                                                                 K3Icon::Panel,
                                                                 16));
        m_windowListButton->setMinimumSize(m_windowListButton->sizeHint());
        m_layout->insertWidget(0, m_windowListButton);
        m_windowListButton->show();
    }


    m_showAllWindows = TaskBarSettings::showAllWindows();
    m_sortByDesktop = m_showAllWindows && TaskBarSettings::sortByDesktop();
    m_showIcon = TaskBarSettings::showIcon();

    m_currentScreen = -1;    // Show all screens or re-get our screen
    m_showOnlyCurrentScreen = TaskBarSettings::showCurrentScreenOnly() &&
                              QApplication::desktop()->isVirtualDesktop() &&
                              QApplication::desktop()->numScreens() > 1;

    // we need to watch geometry issues if we aren't showing windows when we
    // are paying attention to the current Xinerama screen
    if (m_showOnlyCurrentScreen)
    {
        // disconnect first in case we've been here before
        // to avoid multiple connections
        disconnect(TaskManager::self(), SIGNAL(windowChangedGeometry(Task::TaskPtr)),
                    this, SLOT(windowChangedGeometry(Task::TaskPtr)));
        connect(TaskManager::self(), SIGNAL(windowChangedGeometry(Task::TaskPtr)),
                 this, SLOT(windowChangedGeometry(Task::TaskPtr)));
        TaskManager::self()->trackGeometry();
    }

    if (wasShowWindows != m_showAllWindows ||
        wasSortByDesktop != m_sortByDesktop ||
        wasShowIcon != m_showIcon)
    {
        // relevant settings changed, update our task containers
        foreach (TaskContainer* container, containers)
        {
            container->settingsChanged();
        }
    }

    TaskManager::self()->setXCompositeEnabled(TaskBarSettings::showThumbnails());

    if (!blocklayout)
    {
        reLayout();
    }
}

void TaskBar::setOrientation( Qt::Orientation o )
{
    m_container->setOrientation( o );
    reLayout();
}

void TaskBar::resizeEvent( QResizeEvent* e )
{
    if (m_showOnlyCurrentScreen)
    {
        QPoint topLeft = mapToGlobal(this->geometry().topLeft());
        if (m_currentScreen != QApplication::desktop()->screenNumber(topLeft))
        {
            // we have been moved to another screen!
            m_currentScreen = -1;
            reGroup();
        }
    }
#ifdef __GNUC__
    #warning Fix this!!
#endif    
    //Panner::resizeEvent( e );

    if ( !blocklayout )
    {
        reLayout();
    }
}

void TaskBar::add(Task::TaskPtr task)
{
    if (!task ||
        (m_showOnlyCurrentScreen &&
         !TaskManager::isOnScreen(showScreen(), task->window())))
    {
        return;
    }

    // try to group
    if (isGrouping)
    {
        for (TaskContainer::Iterator it = containers.begin();
             it != containers.end();
             ++it)
        {
            TaskContainer* c = *it;

            if (idMatch(task->classClass(), c->id()))
            {
                c->add(task);

                if (!blocklayout)
                {
                    reLayout();
                }

                return;
            }
        }
    }

    // create new container
    TaskContainer *container = new TaskContainer(task, this, m_container->viewport());
    m_hiddenContainers.append(container);

    // even though there is a signal to listen to, we have to add this
    // immediately to ensure grouping doesn't break (primarily on startup)
    // we still add the container to m_hiddenContainers in case the event
    // loop gets re-entered here and something bizarre happens. call it
    // insurance =)
    showTaskContainer(container);
}

void TaskBar::add(Startup::StartupPtr startup)
{
    if (!startup)
    {
        return;
    }

    for (TaskContainer::Iterator it = containers.begin();
         it != containers.end();
         ++it)
    {
        if ((*it)->contains(startup))
        {
            return;
        }
    }

    // create new container
    TaskContainer *container = new TaskContainer(startup, frames, this, m_container->viewport());
    m_hiddenContainers.append(container);
    connect(container, SIGNAL(showMe(TaskContainer*)), this, SLOT(showTaskContainer(TaskContainer*)));
}

void TaskBar::showTaskContainer(TaskContainer* container)
{
    int idx = m_hiddenContainers.indexOf(container);
    if (idx != -1)
    {
        m_hiddenContainers.removeAt(idx);
    }

    if (container->isEmpty())
    {
        return;
    }

    // try to place the container after one of the same app
    if (TaskBarSettings::sortByApp() && !isGrouping)
    {
        TaskContainer::Iterator it = containers.begin();
        for (; it != containers.end(); ++it)
        {
            TaskContainer* c = *it;

            if (container->id().toLower() == c->id().toLower())
            {
                // search for the last occurrence of this app
                for (; it != containers.end(); ++it)
                {
                    c = *it;

                    if (container->id().toLower() != c->id().toLower())
                    {
                        break;
                    }
                }
                break;
            }
        }

        if (it != containers.end())
        {
            containers.insert(it, container);
        }
        else
        {
            containers.append(container);
        }
    }
    else
    {
        containers.append(container);
    }

    //setWidget(container);

    if (!blocklayout)
    {
        emit containerCountChanged();
        reLayout();
    }
}

void TaskBar::remove(Task::TaskPtr task, TaskContainer* container)
{
    for (TaskContainer::Iterator it = m_hiddenContainers.begin();
         it != m_hiddenContainers.end();
         ++it)
    {
        if ((*it)->contains(task))
        {
            (*it)->deleteLater();
            m_hiddenContainers.erase(it);
            break;
        }
    }

    if (!container)
    {
        for (TaskContainer::Iterator it = containers.begin();
             it != containers.end();
             ++it)
        {
            if ((*it)->contains(task))
            {
                container = *it;
                break;
            }
        }

        if (!container)
        {
            return;
        }
    }

    container->remove(task);

    if (container->isEmpty())
    {
        int idx = containers.indexOf(container);
        if (idx != -1)
        {
            containers.removeAt(idx);
        }
        container->deleteLater();

        if (!blocklayout)
        {
            emit containerCountChanged();
            reLayout();
        }
    }
    else if (!blocklayout && container->filteredTaskCount() < 1)
    {
        emit containerCountChanged();
        reLayout();
    }
}

void TaskBar::remove(Startup::StartupPtr startup, TaskContainer* container)
{
    for (TaskContainer::Iterator it = m_hiddenContainers.begin();
         it != m_hiddenContainers.end();
         ++it)
    {
        if ((*it)->contains(startup))
        {
            (*it)->remove(startup);

            if ((*it)->isEmpty())
            {
                (*it)->deleteLater();
                m_hiddenContainers.erase(it);
            }

            break;
        }
    }

    if (!container)
    {
        for (TaskContainer::Iterator it = containers.begin();
             it != containers.end();
             ++it)
        {
            if ((*it)->contains(startup))
            {
                container = *it;
                break;
            }
        }

        if (!container)
        {
            return;
        }
    }

    container->remove(startup);
    if (!container->isEmpty())
    {
        return;
    }

    int idx = containers.indexOf(container);
    if (idx != -1)
    {
        containers.removeAt(idx);
    }

    container->deleteLater();

    if (!blocklayout)
    {
        emit containerCountChanged();
        reLayout();
    }
}

void TaskBar::desktopChanged(int desktop)
{
    if (m_showAllWindows)
    {
        return;
    }

    foreach (TaskContainer* container, containers)
    {
        container->desktopChanged(desktop);
    }

    emit containerCountChanged();
    reLayout();
}

void TaskBar::windowChanged(Task::TaskPtr task)
{
    if (m_showOnlyCurrentScreen &&
        !TaskManager::isOnScreen(showScreen(), task->window()))
    {
        return; // we don't care about this window
    }

    TaskContainer* container = 0;
    for (TaskContainer::List::const_iterator it = containers.constBegin();
         it != containers.constEnd();
         ++it)
    {
        TaskContainer* c = *it;

        if (c->contains(task))
        {
            container = c;
            break;
        }
    }

    if (!container)
    {
        return;
    }

    container->windowChanged(task);

    if (!m_showAllWindows || TaskBarSettings::showOnlyIconified())
    {
        emit containerCountChanged();
    }

    reLayout();
}

void TaskBar::windowChangedGeometry(Task::TaskPtr task)
{
    //TODO: this gets called every time a window's geom changes
    //      when we are in "show only on the same Xinerama screen"
    //      mode it would be Good(tm) to compress these events so this
    //      gets run less often, but always when needed
    TaskContainer* container = 0;
    for (TaskContainer::Iterator it = containers.begin();
         it != containers.end();
         ++it)
    {
        TaskContainer* c = *it;
        if (c->contains(task))
        {
            container = c;
            break;
        }
    }

    if ((!!container) == TaskManager::isOnScreen(showScreen(), task->window()))
    {
        // we have this window covered, so we don't need to do anything
        return;
    }

    if (container)
    {
        remove(task, container);
    }
    else
    {
        add(task);
    }
}

void TaskBar::reLayout()
{
    // filter task container list
    TaskContainer::List list = filteredContainers();
    if (list.count() < 1)
    {
        resize(contentsRect().width(), contentsRect().height());
        return;
    }

    if (isGrouping != shouldGroup())
    {
        reGroup();
        return;
    }

    // sort container list by desktop
    if (m_sortByDesktop)
    {
        sortContainersByDesktop(list);
    }

    // init content size
    resize( contentsRect().width(), contentsRect().height() );

    // number of rows simply depends on our height which is either the
    // minimum button height or the height of the font in use, whichever is
    // largest
    QFontMetrics fm(KGlobalSettings::taskbarFont());
    int minButtonHeight = fm.height() > TaskBarSettings::minimumButtonHeight() ?
                          fm.height() : TaskBarSettings::minimumButtonHeight();

    // horizontal layout
    if ( m_container->orientation() == Qt::Horizontal )
    {
        int bwidth = BUTTON_MIN_WIDTH;
        int rows = contentsRect().height() / minButtonHeight;

        if (rows < 1)
        {
            rows = 1;
        }

        // actual button height
        int bheight = contentsRect().height() / rows;

        // avoid zero division later
        if (bheight < 1)
        {
            bheight = 1;
        }

        // buttons per row
        int bpr = (int)ceil( (double)list.count() / rows);

        // adjust content size
        if ( contentsRect().width() < bpr * BUTTON_MIN_WIDTH )
        {
            resize( bpr * BUTTON_MIN_WIDTH, contentsRect().height() );
        }

        // maximum number of buttons per row
        int mbpr = contentsRect().width() / BUTTON_MIN_WIDTH;

        // expand button width if space permits
        if (mbpr > bpr)
        {
            bwidth = contentsRect().width() / bpr;
            int maxWidth = TaskBarSettings::maximumButtonWidth();
            if (maxWidth > 0 && bwidth > maxWidth)
            {
                bwidth = maxWidth;
            }
        }

        // layout containers

        // for taskbars at the bottom, we need to ensure that the bottom
        // buttons touch the bottom of the screen. since we layout from
        // top to bottom this means seeing if we have any padding and
        // popping it on the top. this preserves Fitt's Law behaviour
        // for taskbars on the bottom
        int topPadding = 0;
        if (arrowType == Qt::UpArrow)
        {
            topPadding = contentsRect().height() % (rows * bheight);
        }

        int i = 0;
        bool reverseLayout = QApplication::isRightToLeft();
        for (TaskContainer::Iterator it = list.begin();
             it != list.end();
             ++it, i++)
        {
            TaskContainer* c = *it;

            int row = i % rows;

            c->setArrowType(arrowType);
            c->resize( bwidth, bheight );
            c->show();

            int x = ( i / rows ) * bwidth;
            if (reverseLayout)
            {
                x = contentsRect().width() - x - bwidth;
            }

            c->move( x, (row * bheight) + topPadding );
            c->update();
        }
    }
    else // vertical layout
    {
        // adjust content size
        if (contentsRect().height() < (int)list.count() * minButtonHeight)
        {
            resize(contentsRect().width(), list.count() * minButtonHeight);
        }

        // layout containers
        int i = 0;
        for (TaskContainer::Iterator it = list.begin();
             it != list.end();
             ++it)
        {
            TaskContainer* c = *it;

            c->setArrowType(arrowType);
            c->resize(contentsRect().width(), minButtonHeight);
            c->show();

            c->move(0, i * minButtonHeight);
            i++;
            c->update();
        }
    }

    QTimer::singleShot(100, this, SLOT(publishIconGeometry()));
}

void TaskBar::setArrowType(Qt::ArrowType at)
{
    if (arrowType == at)
    {
        return;
    }

    arrowType = at;
    foreach (TaskContainer* container, containers)
    {
        container->setArrowType(arrowType);
    }
}

void TaskBar::publishIconGeometry()
{
    QPoint p = mapToGlobal(QPoint(0,0)); // roundtrip, don't do that too often

    foreach (TaskContainer* container, containers)
    {
        container->publishIconGeometry(p);
    }
}

bool TaskBar::idMatch( const QString& id1, const QString& id2 )
{
    if ( id1.isEmpty() || id2.isEmpty() )
        return false;

    return id1.toLower() == id2.toLower();
}

int TaskBar::containerCount() const
{
    int i = 0;

    for (TaskContainer::List::const_iterator it = containers.constBegin();
         it != containers.constEnd();
         ++it)
    {
        if ((m_showAllWindows || (*it)->onCurrentDesktop()) &&
            ((showScreen() == -1) || ((*it)->isOnScreen())))
        {
            i++;
        }
    }

    return i;
}

int TaskBar::taskCount() const
{
    int i = 0;

    for (TaskContainer::List::const_iterator it = containers.constBegin();
         it != containers.constEnd();
         ++it)
    {
        if ((m_showAllWindows || (*it)->onCurrentDesktop()) &&
            ((showScreen() == -1) || ((*it)->isOnScreen())))
        {
            i += (*it)->filteredTaskCount();
        }
    }

    return i;
}

int TaskBar::maximumButtonsWithoutShrinking() const
{
    QFontMetrics fm(KGlobalSettings::taskbarFont());
    int minButtonHeight = fm.height() > TaskBarSettings::minimumButtonHeight() ?
                          fm.height() : TaskBarSettings::minimumButtonHeight();

    int rows = contentsRect().height() / minButtonHeight;
    if (rows < 1)
    {
        rows = 1;
    }

    if ( m_container->orientation() == Qt::Horizontal ) {
        // maxWidth of 0 means no max width, drop back to default
        int maxWidth = TaskBarSettings::maximumButtonWidth();
        if (maxWidth == 0)
        {
            maxWidth = BUTTON_MAX_WIDTH;
        }

        // They squash a bit before they pop, hence the 2
        return rows * (contentsRect().width() / maxWidth) + 2;
    }
    else
    {
        // Overlap slightly and ugly arrows appear, hence -1
        return rows - 1;
    }
}

bool TaskBar::shouldGroup() const
{
    return TaskBarSettings::groupTasks() == TaskBarSettings::GroupAlways ||
           (TaskBarSettings::groupTasks() == TaskBarSettings::GroupWhenFull &&
            taskCount() > maximumButtonsWithoutShrinking());
}

void TaskBar::reGroup()
{
    isGrouping = shouldGroup();
    blocklayout = true;

    qDeleteAll(m_hiddenContainers);
    m_hiddenContainers.clear();

    qDeleteAll(containers);
    containers.clear();

    foreach (Task::TaskPtr task, TaskManager::self()->tasks())
    {
        if (showScreen() == -1 || task->isOnScreen(showScreen()))
        {
            add(task);
        }
    }

    foreach (Startup::StartupPtr startup, TaskManager::self()->startups())
    {
        add(startup);
    }

    blocklayout = false;
    reLayout();
}

TaskContainer::List TaskBar::filteredContainers()
{
    // filter task container list
    TaskContainer::List list;

    foreach (TaskContainer* c, containers)
    {
        if ((m_showAllWindows || c->onCurrentDesktop()) &&
            (!TaskBarSettings::showOnlyIconified() || c->isIconified()) &&
            ((showScreen() == -1) || c->isOnScreen()))
        {
            list.append(c);
            c->show();
        }
        else
        {
            c->hide();
        }
    }

    return list;
}

void TaskBar::activateNextTask(bool forward)
{
    bool forcenext = false;
    TaskContainer::List list = filteredContainers();

    // this is necessary here, because 'containers' is unsorted and
    // we want to iterate over the _shown_ task containers in a linear way
    if (m_sortByDesktop)
    {
        sortContainersByDesktop(list);
    }

    QList<TaskContainer*>::iterator it = forward ? list.begin() : list.end();
    QList<TaskContainer*>::iterator itEnd = forward ? list.end() : list.begin();
    for ( ; it != itEnd; forward ? ++it : --it )
    {
        if ( it != list.end() && (*it)->activateNextTask( forward, forcenext ) )
        {
            return;
        }
    }

    if (forcenext)
    {
        QList<TaskContainer*>::iterator it2 = forward ? list.begin() : list.end();
        QList<TaskContainer*>::iterator itEnd2 = forward ? list.end() : list.begin();
        for ( ; it2 != itEnd2; forward ? ++it2 : --it2 )
        {
            if ( it2 != list.end() && (*it2)->activateNextTask( forward, forcenext ) )
            {
                return;
            }
        }

        return;
    }

    forcenext = true; // select first

    QList<TaskContainer*>::iterator it3 = forward ? list.begin() : list.end();
    QList<TaskContainer*>::iterator itEnd3 = forward ? list.end() : list.begin();
    for ( ; it3 != itEnd3; forward ? ++it3 : --it3 )
    {
        if ( it3 == list.end() )
        {
            break;
        }

        TaskContainer *c = *it;
        if ( m_sortByDesktop )
        {
            if ( forward ? c->desktop() < TaskManager::self()->currentDesktop()
                         : c->desktop() > TaskManager::self()->currentDesktop() )
            {
                continue;
            }
        }

        if ( c->activateNextTask( forward, forcenext ) )
        {
            return;
        }
    }
}

void TaskBar::wheelEvent(QWheelEvent* e)
{
    if (e->delta() > 0)
    {
        // scroll away from user, previous task
        activateNextTask(false);
    }
    else
    {
        // scroll towards user, next task
        activateNextTask(true);
    }
}

void TaskBar::slotActivateNextTask()
{
    activateNextTask( true );
}

void TaskBar::slotActivatePreviousTask()
{
    activateNextTask( false );
}

void TaskBar::slotSettingsChanged( int category )
{
    if( category == KGlobalSettings::SETTINGS_SHORTCUTS )
    {
        keys->readSettings();
    }
}

int TaskBar::showScreen() const
{
    if (m_showOnlyCurrentScreen && m_currentScreen == -1)
    {
        const_cast<TaskBar*>(this)->m_currentScreen =
            QApplication::desktop()->screenNumber(mapToGlobal(this->geometry().topLeft()));
    }

    return m_currentScreen;
}

// taken from mtaskbar, by Sebastian Wolff
void TaskBar::drawShadowText(QPainter  &p, QRect tr, int tf, const QString & str,
                             int len, QRect * brect)
{
    // get the color of the shadow: white for dark text, black for bright text
    QPen textPen = p.pen();
    QColor shadCol = textPen.color();

    if (shadCol.red() +
        shadCol.green() +
        shadCol.blue() <= 3*256/2-1)
    {
        shadCol = QColor(255,255,255);
    }
    else
    {
        shadCol = QColor(0,0,0);
    }

    // get a transparent pixmap
    QPainter pixPainter;
    QPixmap textPixmap(width(), height());

    textPixmap.fill(QColor(0,0,0));
    textPixmap.setMask(textPixmap.createHeuristicMask(true));

    // draw text
    pixPainter.begin(&textPixmap);
    pixPainter.setPen(Qt::white);
    pixPainter.setFont(p.font()); // get the font from the root painter
    pixPainter.drawText( tr, tf, str.left( len ), brect );
    pixPainter.end();

    if (!m_textShadowEngine)
    {
        KShadowSettings * shadset = new KShadowSettings();
        shadset->setOffsetX(0);
        shadset->setOffsetY(0);
        shadset->setThickness(1);
        shadset->setMaxOpacity(96);
        m_textShadowEngine = new KShadowEngine(shadset);
    }

    // draw shadow
    QImage img = m_textShadowEngine->makeShadow(textPixmap, shadCol);

    // return
    p.drawImage(0, 0, img);
    p.drawText(tr, tf, str.left(len), brect);
}

void TaskBar::sortContainersByDesktop(TaskContainer::List& list)
{
    typedef QVector<QPair<int, QPair<int, TaskContainer*> > > SortVector;
    SortVector sorted;
    sorted.resize(list.count());
    int i = 0;

    TaskContainer::List::ConstIterator lastUnsorted(list.constEnd());
    for (TaskContainer::List::ConstIterator it = list.constBegin();
         it != lastUnsorted;
         ++it)
    {
        sorted[i] = qMakePair((*it)->desktop(), qMakePair(i, *it));
        ++i;
    }

    qHeapSort(sorted);

    list.clear();
    SortVector::const_iterator lastSorted(sorted.constEnd());
    for (SortVector::const_iterator it = sorted.constBegin();
         it != lastSorted;
         ++it)
    {
        list.append((*it).second.second);
    }
}

void TaskBar::configChanged() {
    TaskBarSettings::self()->readConfig();

    configure();
}

void TaskBar::preferences()
{
    QDBusInterface ref( "org.kde.kicker",
                        "/MainApplication", "org.kde.Kicker" );
    ref.call( "showTaskBarConfig" );
}

void TaskBar::orientationChange(Qt::Orientation o)
{
    if (o == Qt::Horizontal)
     {
        if (m_windowListButton)
        {
            m_windowListButton->setFixedWidth(WINDOWLISTBUTTON_SIZE);
            m_windowListButton->setMaximumHeight(BUTTON_MAX_WIDTH);
        }
        m_layout->setDirection(QApplication::isRightToLeft() ?
                                QBoxLayout::RightToLeft :
                                QBoxLayout::LeftToRight);
    }
    else
    {
        if (m_windowListButton)
        {
            m_windowListButton->setMaximumWidth(BUTTON_MAX_WIDTH);
            m_windowListButton->setFixedHeight(WINDOWLISTBUTTON_SIZE);
        }
        m_layout->setDirection(QBoxLayout::TopToBottom);
    }

    m_container->setOrientation(o);
    if (m_windowListButton)
    {
        m_windowListButton->setOrientation(o);
    }
    m_layout->activate();
}

void TaskBar::popupDirectionChange(Plasma::Position d)
{
    m_direction = d;
    Qt::ArrowType at = Qt::UpArrow;

    QString icon;
    switch (d)
    {
        case Plasma::Up:
            icon = "1downarrow";
            at = Qt::DownArrow;
            break;
        case Plasma::Down:
            icon = "1uparrow";
            at = Qt::UpArrow;
            break;
        case Plasma::Left:
            icon = "1rightarrow";
            at = Qt::RightArrow;
            break;
        case Plasma::Right:
            icon = "1leftarrow";
            at = Qt::LeftArrow;
            break;
	case Plasma::Floating:
	    icon = "1uparrow";
	    at = Qt::UpArrow;
	    break;
    }

    setArrowType(at);

    if (m_windowListButton)
    {
        m_windowListButton->setIcon(kapp->iconLoader()->loadIcon(icon,
                                                                 K3Icon::Panel,
                                                                 16));
        m_windowListButton->setMinimumSize(m_windowListButton->sizeHint());
    }
}

void TaskBar::showWindowListMenu()
{
    if (!m_windowListMenu)
        return;

    m_windowListMenu->init();

    // calc popup menu position
    QPoint pos( mapToGlobal( QPoint(0,0) ) );

    switch( m_direction ) {
        case Plasma::Right:
            pos.setX( pos.x() + width() );
            break;
        case Plasma::Left:
            pos.setX( pos.x() - m_windowListMenu->sizeHint().width() );
            break;
        case Plasma::Down:
            pos.setY( pos.y() + height() );
            break;
        case Plasma::Up:
            pos.setY( pos.y() - m_windowListMenu->sizeHint().height() );
        default:
            break;
    }

    disconnect( m_windowListButton, SIGNAL( pressed() ), this, SLOT( showWindowListMenu() ) );
    m_windowListMenu->exec( pos );
    QTimer::singleShot(100, this, SLOT(reconnectWindowListButton()));
}

void TaskBar::windowListMenuAboutToHide()
{
    // this ensures that when clicked AGAIN, the window list button doesn't cause the
    // window list menu to show again. usability, you see. hoorah.
    m_windowListButton->setDown( false );
}

void TaskBar::reconnectWindowListButton()
{
    connect( m_windowListButton, SIGNAL( pressed() ), SLOT( showWindowListMenu() ) );
}
