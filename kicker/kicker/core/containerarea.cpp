/*****************************************************************

Copyright (c) 1996-2004 the kicker authors. See file AUTHORS.

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

#include <unistd.h>

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <QStyleOptionFocusRect>
#include <qtextstream.h>
#include <qtimer.h>
#include <qmatrix.h>
#include <QMenuItem>
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>

#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <kprocess.h>
#include <krootpixmap.h>
#include <kpixmap.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kservice.h>
#include <kmenu.h>
#include <kauthorized.h>

#include "addapplet.h"
#include "browser_dlg.h"
#include "container_applet.h"
#include "container_button.h"
#include "containerarealayout.h"
#include "exe_dlg.h"
#include "extensionmanager.h"
#include "kicker.h"
#include "kickerSettings.h"
#include "kickertip.h"
#include "pluginmanager.h"

#include "containerarea.h"

using namespace Qt;

// for multihead
extern int kicker_screen_number;

ContainerArea::ContainerArea(KConfig* _c,
                             QWidget* parent,
                             QMenu* opMenu,
                             const char* name)
    : Panner(parent, name),
      _moveAC(0),
      _pos(Plasma::Left),
      _config(_c),
      _dragIndicator(0),
      _dragMoveAC(0),
      _dragMoveOffset(QPoint(0,0)),
      m_opMenu(opMenu),
      _rootPixmap(0),
      _useBgTheme(false),
      _bgSet(false),
      m_canAddContainers(true),
      m_immutable(_c->isImmutable()),
      m_layout(0),
      m_addAppletDialog(0)
{
    m_contents = new QWidget(viewport());
    m_layout = new ContainerAreaLayout(m_contents);
    setWidget(m_contents);

    // Install an event filter to propagate layout hints coming from
    // m_contents.
    m_contents->installEventFilter(this);

    setAcceptDrops( !Kicker::self()->isImmutable() );
    connect(&_autoScrollTimer, SIGNAL(timeout()), SLOT(autoScroll()));
    connect(Kicker::self(), SIGNAL(immutabilityChanged(bool)),
            SLOT(immutabilityChanged(bool)));
}

ContainerArea::~ContainerArea()
{
    // don't emit signals from destructor
    blockSignals( true );
    // clear applets
    removeAllContainers();
}

void ContainerArea::initialize(bool useDefaultConfig)
{
    // do we really need to do this?
    removeAllContainers();

    // restore applet layout or load a default panel layout
    _config->setGroup("General");
    if (_config->hasKey("Applets2"))
    {
        if (_config->groupIsImmutable("General"))
        {
            m_immutable = true;
        }

        m_canAddContainers = !m_immutable &&
                             !_config->entryIsImmutable("Applets2");
        loadContainers(_config->readEntry("Applets2", QStringList() ));
    }
    else if (useDefaultConfig)
    {
        defaultContainerConfig();
    }
}

void ContainerArea::defaultContainerConfig()
{
    //FIXME: make this use a file template so it isn't hardcoded anymore
    BaseContainer::List containers;

    containers.append(new KMenuButtonContainer(m_opMenu, m_contents));
    containers.append(new DesktopButtonContainer(m_opMenu, m_contents));

    int dsize;
    if (orientation() == Horizontal)
    {
        dsize = width();
    }
    else
    {
        dsize = height();
    }

    dsize -= 560;
    QStringList buttons;

    QFile f(locate("data", "kicker/default-apps"));
    if (f.open(QIODevice::ReadOnly))
    {
        QTextStream is(&f);

        while (!is.atEnd())
            buttons << is.readLine();

        f.close();
    }
    else
    {
        buttons << "kde-Home.desktop"
                << "kde-konqbrowser.desktop"
                << "kde-Kontact.desktop"
                << "kde-kword.desktop"
                << "kde-Help.desktop";
    }

    //int size = dsize;
    foreach ( const QString& it, buttons )
    {
        /*size -= 42;
        if (size <= 0)
            break;*/

        BaseContainer *button;
        KService::Ptr service = KService::serviceByStorageId( it );
        if ( !service )
        {
            // look for a special button
            QString s = locate( "appdata", it );
            if ( s.isEmpty() )
            {
                continue;
            }
            QString itExt = it.section('/', 1);
            button = new ExtensionButtonContainer(itExt, m_opMenu, m_contents);
        }
        else
        {
            button = new ServiceButtonContainer(service, m_opMenu, m_contents);
        }

        if ( button->isValid() )
        {
            containers.append( button );
        }
        else
        {
            delete button;
        }
    }

    PluginManager* manager = PluginManager::self();
    AppletContainer *a = 0;

    // pager applet
    a = manager->createAppletContainer(
        "minipagerapplet.desktop",
        true,
        QString(),
        m_opMenu,
        m_contents);
    if ( a )
    {
        containers.append( a );
    }

    // taskbar applet
    a = manager->createAppletContainer(
        "taskbarapplet.desktop",
        true,
        QString(),
        m_opMenu,
        m_contents);
    if ( a )
    {
        containers.append( a );
    }

    // system tray applet
#warning "once qxembed is back and system tray works again, renable this"
/*    a = manager->createAppletContainer(
        "systemtrayapplet.desktop",
        true,
        QString(),
        m_opMenu,
        m_contents );
    if (a)
    {
        a->setFreeSpace(1);
        containers.append(a);
    } */

    // clock applet
    a = manager->createAppletContainer(
        "clockapplet.desktop",
        true,
        QString(),
        m_opMenu,
        m_contents );
    if ( a )
    {
        a->setFreeSpace(1);
        containers.append( a );
    }

    foreach ( BaseContainer *it, containers )
    {
        addContainer( it );
    }

    saveContainerConfig();
}

void ContainerArea::loadContainers(const QStringList& containers)
{
    // read applet list
    bool badApplets = false;

    // now restore the applets
    foreach (const QString& appletId, containers )
    {
        // is there a config group for this applet?
        if ( !_config->hasGroup(appletId) )
        {
            continue;
        }

        KConfigGroup group( _config, appletId.latin1() );

        BaseContainer* a = 0;

        int sep = appletId.lastIndexOf('_');
        Q_ASSERT( sep != -1 );
        QString appletType = appletId.left( sep );

        // create a matching applet container
        if (appletType == "KMenuButton")
        {
            a = new KMenuButtonContainer( group, m_opMenu, m_contents );
        }
        else if (appletType == "DesktopButton")
        {
            a = new DesktopButtonContainer( group, m_opMenu, m_contents );
        }
        else if (appletType == "WindowListButton")
        {
            a = new WindowListButtonContainer( group, m_opMenu, m_contents );
        }
        else if ((appletType == "BookmarksButton") && KAuthorized::authorizeKAction("bookmarks"))
        {
            a = new BookmarksButtonContainer( group, m_opMenu, m_contents );
        }
        else if (appletType == "ServiceButton")
        {
            a = new ServiceButtonContainer( group, m_opMenu, m_contents );
        }
        else if (appletType == "URLButton")
        {
            a = new URLButtonContainer( group, m_opMenu, m_contents );
        }
        else if (appletType == "BrowserButton")
        {
            a = new BrowserButtonContainer( group, m_opMenu, m_contents );
        }
        else if (appletType == "ServiceMenuButton")
        {
            a = new ServiceMenuButtonContainer( group, m_opMenu, m_contents );
        }
        else if (appletType == "ExecButton")
        {
            a = new NonKDEAppButtonContainer( group, m_opMenu, m_contents );
        }
        else if (appletType == "ExtensionButton")
        {
            a = new ExtensionButtonContainer( group, m_opMenu, m_contents );
        }
        else if (appletType == "Applet")
        {
            bool immutable = Kicker::self()->isImmutable() ||
                             group.groupIsImmutable() ||
                             group.entryIsImmutable("ConfigFile");
            a = PluginManager::self()->createAppletContainer(
                   group.readPathEntry("DesktopFile"),
                   true, // isStartup
                   group.readPathEntry("ConfigFile"),
                   m_opMenu,
                   m_contents,
                   immutable);
        }

        if ( a && a->isValid() )
        {
            a->setAppletId(appletId);
            a->loadConfiguration(group);
            addContainer(a);
        }
        else
        {
            badApplets = true;
            delete a;
        }
    }

    if ( badApplets )
    {
        // since we may have had Bad Applets in our list
        // let's save it again, just in case
        saveContainerConfig();
    }
}

void ContainerArea::saveContainerConfig(bool layoutOnly)
{
    if (!canAddContainers())
    {
        return;
    }

    // Save the applet list
    QStringList alist;
    QLayout *layout = m_layout->layout();
    for ( int i = 0; i < layout->count(); ++i )
    {
        BaseContainer *a = dynamic_cast<BaseContainer*>( layout->itemAt(i)->widget() );
        if ( a )
        {
            KConfigGroup group( _config, a->appletId().latin1() );
            a->saveConfiguration( group, layoutOnly );
            alist.append( a->appletId() );
        }
    }

    KConfigGroup group( _config, "General" );
    group.writeEntry( "Applets2", alist );

    _config->sync();
}

void ContainerArea::removeAllContainers()
{
    foreach ( BaseContainer *it, m_containers )
    {
        delete it;
    }

    m_containers.clear();
}

void ContainerArea::configure()
{
    foreach ( BaseContainer *b, m_containers )
    {
        b->configure();
    }
}

const QWidget* ContainerArea::addButton(const AppletInfo& info)
{
    if (!canAddContainers())
    {
        return 0;
    }

    QString buttonType = info.library();

    if ((buttonType == "BookmarksButton") && KAuthorized::authorizeKAction("bookmarks"))
    {
        return addBookmarksButton();
    }
    else if (buttonType == "BrowserButton")
    {
        return addBrowserButton();
    }
    else if (buttonType == "DesktopButton")
    {
        return addDesktopButton();
    }
    else if (buttonType == "ExecButton")
    {
        return addNonKDEAppButton();
    }
    else if (buttonType == "KMenuButton")
    {
        return addKMenuButton();
    }
    else if (buttonType == "WindowListButton")
    {
        return addWindowListButton();
    }
    else // ExtensionButton
    {
        return addExtensionButton(info.desktopFile());
    }
}

const QWidget* ContainerArea::addKMenuButton()
{
    if (!canAddContainers())
    {
        return 0;
    }

    KMenuButtonContainer *b = new KMenuButtonContainer( m_opMenu, m_contents );
    completeContainerAddition(b);
    return b;
}

const QWidget* ContainerArea::addDesktopButton()
{
    if (!canAddContainers())
    {
        return 0;
    }

    DesktopButtonContainer *b = new DesktopButtonContainer( m_opMenu, m_contents );
    completeContainerAddition(b);
    return b;
}

const QWidget* ContainerArea::addWindowListButton()
{
    if (!canAddContainers())
    {
        return 0;
    }

    WindowListButtonContainer *b = new WindowListButtonContainer( m_opMenu, m_contents );
    completeContainerAddition(b);
    return b;
}

const QWidget* ContainerArea::addBookmarksButton()
{
    if (!canAddContainers())
    {
        return 0;
    }

    BookmarksButtonContainer *b = new BookmarksButtonContainer( m_opMenu, m_contents );
    completeContainerAddition(b);
    return b;
}

const QWidget* ContainerArea::addServiceButton(const QString& desktopFile)
{
    if (!canAddContainers())
    {
        return 0;
    }

    ServiceButtonContainer *b = new ServiceButtonContainer( desktopFile, m_opMenu, m_contents );
    completeContainerAddition(b);
    return b;
}

const QWidget* ContainerArea::addURLButton(const QString &url)
{
    if (!canAddContainers())
    {
        return 0;
    }

    URLButtonContainer *b = new URLButtonContainer( url, m_opMenu, m_contents );
    completeContainerAddition(b);
    return b;
}

const QWidget* ContainerArea::addBrowserButton()
{
    if (!canAddContainers())
    {
        return 0;
    }

    PanelBrowserDialog *dlg = new PanelBrowserDialog( QDir::home().path(), "kdisknav" );

    if (dlg->exec() == QDialog::Accepted)
    {
        return addBrowserButton( dlg->path(), dlg->icon() );
    }

    return 0;
}

const QWidget* ContainerArea::addBrowserButton( const QString &startDir, const QString& icon )
{
    if (!canAddContainers())
    {
        return 0;
    }

    BrowserButtonContainer *b = new BrowserButtonContainer( startDir, m_opMenu, icon, m_contents );
    completeContainerAddition(b);
    return b;
}

const QWidget* ContainerArea::addServiceMenuButton(const QString& relPath)
{
    if (!canAddContainers())
    {
        return 0;
    }

    ServiceMenuButtonContainer *b = new ServiceMenuButtonContainer( relPath, m_opMenu, m_contents );
    completeContainerAddition(b);
    return b;
}

const QWidget* ContainerArea::addNonKDEAppButton()
{
    if (!canAddContainers())
    {
        return 0;
    }

    PanelExeDialog dlg( QString::null, QString::null, QString::null,
                       QString::null, QString::null, false, 0 );

    if ( dlg.exec() == QDialog::Accepted )
    {
        return addNonKDEAppButton(dlg.title(), dlg.description(),
                                  dlg.command(), dlg.iconPath(),
                                  dlg.commandLine(),
                                  dlg.useTerminal());
    }

    return 0;
}

const QWidget* ContainerArea::addNonKDEAppButton(const QString &name,
                                                 const QString &description,
                                                 const QString& filePath,
                                                 const QString &icon,
                                                 const QString &cmdLine,
                                                 bool inTerm)
{
    if (!canAddContainers())
    {
        return 0;
    }

    NonKDEAppButtonContainer *b = new NonKDEAppButtonContainer(name,
                                                               description,
                                                               filePath, icon,
                                                               cmdLine, inTerm,
                                                               m_opMenu,
                                                               m_contents);
    completeContainerAddition(b);
    return b;
}

const QWidget* ContainerArea::addExtensionButton(const QString& df)
{
    if (!canAddContainers())
    {
        return 0;
    }

    ExtensionButtonContainer *b = new ExtensionButtonContainer(df, m_opMenu, m_contents);
    completeContainerAddition(b);
    return b;
}

void ContainerArea::completeContainerAddition(BaseContainer* container)
{
    //FIXME: the original comment was:
    //       Set freespace to one since the container will be added at the end.
    //       yet this is not always true =/
    container->setFreeSpace(1);
    addContainer(container, true);
    scrollTo(container);
    saveContainerConfig();
}


AppletContainer* ContainerArea::addApplet(const QString& desktopFile, bool isImmutable)
{
    if (!canAddContainers())
    {
        return 0;
    }

    AppletContainer* a = PluginManager::self()->createAppletContainer(
        desktopFile,
        false,         // not startup
        QString(), // no config
        m_opMenu,
        m_contents,
        isImmutable);

    if (!a || !a->isValid())
    {
        delete a;
        return 0;
    }

    a->setFreeSpace(1);
    addContainer( a, true );
    scrollTo( a );
    saveContainerConfig();

    return a;
}

void ContainerArea::addContainer(BaseContainer* a, bool arrange)
{
    if (!a)
    {
        return;
    }

    if (a->appletId().isNull())
    {
        a->setAppletId(createUniqueId(a->appletType()));
    }

    m_containers.append(a);

    if (arrange)
    {
        if (Kicker::self()->insertionPoint().isNull())
        {
            m_layout->insertIntoFreeSpace(a, QPoint());
        }
        else
        {
            m_layout->insertIntoFreeSpace(a, mapFromGlobal(Kicker::self()->insertionPoint()));
        }
    }
    else
    {
        m_layout->add(a);
    }

    connect(a, SIGNAL(moveme(BaseContainer*)),
            SLOT(startContainerMove(BaseContainer*)));
    connect(a, SIGNAL(removeme(BaseContainer*)),
            SLOT(removeContainer(BaseContainer*)));
    connect(a, SIGNAL(takeme(BaseContainer*)),
            SLOT(takeContainer(BaseContainer*)));
    connect(a, SIGNAL(requestSave()),
            SLOT(slotSaveContainerConfig()));
    connect(a, SIGNAL(maintainFocus(bool)),
            this, SIGNAL(maintainFocus(bool)));

    a->configure(orientation(), popupDirection());
    a->show();
    resizeContents();
}

void ContainerArea::removeContainer(BaseContainer *a)
{
    if (!a || isImmutable() || a->isImmutable())
    {
        return;
    }

    a->slotRemoved( _config );
    m_containers.remove( a );
    m_layout->remove( a );
    a->deleteLater();
    saveContainerConfig( true );
    resizeContents();
}

void ContainerArea::removeContainers(BaseContainer::List containers)
{
    if (isImmutable())
    {
        return;
    }

    m_layout->setEnabled(false);

    foreach (BaseContainer *a, containers)
    {
        if (a->isImmutable())
        {
            continue;
        }

        a->slotRemoved(_config);
        m_containers.remove(a);
        m_layout->remove(a);
        a->deleteLater();
    }

    m_layout->setEnabled(true);
    saveContainerConfig(true);
    resizeContents();
}

void ContainerArea::takeContainer(BaseContainer* a)
{
    if (!a)
    {
        return;
    }

    disconnect(a, SIGNAL(moveme(BaseContainer*)),
               this, SLOT(startContainerMove(BaseContainer*)));
    disconnect(a, SIGNAL(removeme(BaseContainer*)),
               this, SLOT(removeContainer(BaseContainer*)));
    disconnect(a, SIGNAL(takeme(BaseContainer*)),
               this, SLOT(takeContainer(BaseContainer*)));
    disconnect(a, SIGNAL(requestSave()),
               this, SLOT(slotSaveContainerConfig()));
    disconnect(a, SIGNAL(maintainFocus(bool)),
               this, SIGNAL(maintainFocus(bool)));

    // Just remove the group from our own config file. Leave separate config
    // files untouched.
    _config->deleteGroup(a->appletId().latin1());
    _config->sync();
    m_containers.remove(a);
    m_layout->remove(a);
    saveContainerConfig(true);
    resizeContents();
}

void ContainerArea::resizeContents()
{
    int w = width();
    int h = height();

    if (orientation() == Qt::Horizontal)
    {
        int newWidth = m_layout->widthForHeight(h);
        if (newWidth > w)
        {
            Panner::resize(newWidth, h);
        }
        else
        {
            Panner::resize(w, h);
        }
    }
    else
    {
        int newHeight = m_layout->heightForWidth(w);

        if (newHeight > h)
        {
            Panner::resize(w, newHeight);
        }
        else
        {
            Panner::resize(w, h);
        }
    }
}

QString ContainerArea::createUniqueId(const QString& appletType) const
{
    QString idBase = appletType + "_%1";
    QString newId;
    int i = 0;
    bool unique = false;

    while (!unique)
    {
        i++;
        newId = idBase.arg(i);

        unique = true;
        foreach ( BaseContainer *it, m_containers )
        {
            if ( it->appletId() == newId )
            {
                unique = false;
                break;
            }
        }
    }

    return newId;
}

bool ContainerArea::canAddContainers() const
{
    return m_canAddContainers && Kicker::self()->canAddContainers();
}

void ContainerArea::startContainerMove(BaseContainer *a)
{
    if (!a || isImmutable())
    {
        return;
    }

    _moveAC = a;

    KickerTip::enableTipping(false);
    emit maintainFocus(true);
    setMouseTracking(true);
    grabMouse(sizeAllCursor);

    m_layout->setStretchEnabled(false);
    a->raise();
}

void ContainerArea::mouseReleaseEvent(QMouseEvent *)
{
    if (!_moveAC)
    {
        return;
    }

    // start container move was caled successfuly
    // so we need to complete the move here
    _autoScrollTimer.stop();
    releaseMouse();
    setCursor(arrowCursor);
    setMouseTracking(false);

    _moveAC->completeMoveOperation();
    KickerTip::enableTipping(true);

    _moveAC = 0;

    emit maintainFocus(false);
    m_layout->setStretchEnabled(true);
    saveContainerConfig(true);
}

void ContainerArea::mouseMoveEvent(QMouseEvent *ev)
{
    if (!_moveAC)
    {
        Panner::mouseMoveEvent(ev);
        return;
    }

    if (ev->state() == LeftButton && !rect().contains(ev->pos()))
    {
        // leaveEvent() don't work, while grabbing the mouse
        _autoScrollTimer.stop();
        releaseMouse();
        setCursor(arrowCursor);
        setMouseTracking(false);

        _moveAC->completeMoveOperation();
        KickerTip::enableTipping(true);

        emit maintainFocus(false);
        m_layout->setStretchEnabled(true);
        saveContainerConfig(true);

        QMimeData* data = new QMimeData;
        _moveAC->populateMimeData(data);
        QDrag* dd = new QDrag(this);
        dd->setMimeData(data);
        dd->setPixmap(kapp->iconLoader()->loadIcon(_moveAC->icon(), KIcon::Small));
        grabKeyboard();
        dd->start();
        releaseKeyboard();
        return;
    }

    if (orientation() == Horizontal)
    {
        int oldX = _moveAC->x() + _moveAC->moveOffset().x();
        int x = ev->pos().x();
        if (ev->state() & ShiftButton)
        {
            m_layout->moveContainerPush(_moveAC, x - oldX);
        }
        else
        {
            m_layout->moveContainerSwitch(_moveAC, x - oldX);
            /* FIXME: Scrolling when the container moves out of the viewport
            bool scroll = false;
            if (rtl)
                if (newPos - 80 <= 0)
                    scroll = true;
            else
                if (newPos + 80 >= (horizontal ? geometry().width()  - moving->geometry().width()
                                               : geometry().height() - moving->geometry().height()))
                    scroll = true;
            [...]
            if (scroll) {
                if (!_autoScrollTimer.isActive())
                    _autoScrollTimer.start(50);

                if (horizontal)
                    scrollBy(dir*10, 0);
                else
                    scrollBy(0, dir*10);
            }
            */
        }
    }
    else
    {
        int oldY = _moveAC->y() + _moveAC->moveOffset().y();
        int y = ev->pos().y();
        if (ev->state() & ShiftButton)
        {
            m_layout->moveContainerPush(_moveAC, y - oldY);
        }
        else
        {
            m_layout->moveContainerSwitch(_moveAC, y - oldY);
            // TODO: Scrolling
        }
    }
}

int ContainerArea::position() const
{
    return static_cast<int>(_pos);
}

Plasma::Position ContainerArea::popupDirection() const
{
    return Plasma::popupDirectionForPosition(_pos);
}

bool ContainerArea::isImmutable() const
{
    return m_immutable || Kicker::self()->isImmutable();
}

void ContainerArea::dragEnterEvent(QDragEnterEvent *ev)
{
    bool canAccept = !isImmutable() &&
                     (BaseContainer::canDecode(ev->mimeData()) ||
                      AppletInfo::canDecode(ev->mimeData()) ||
                      KUrl::List::canDecode(ev->mimeData()));
    ev->accept(canAccept);

    if (!canAccept)
    {
        return;
    }

    m_layout->setStretchEnabled(false);

    if (!_dragIndicator)
    {
        _dragIndicator = new DragIndicator(m_contents);
    }

    int preferedWidth = height();
    int preferedHeight = width();
    if (BaseContainer::canDecode(ev->mimeData()))
    {
        BaseContainer *draggedContainer = BaseContainer::fromMimeData(ev->mimeData());

        if (draggedContainer)
        {
            preferedWidth = draggedContainer->widthForHeight(height());
            preferedHeight = draggedContainer->heightForWidth(width());
        }
    }

    if (orientation() == Horizontal)
    {
        _dragIndicator->setPreferredSize(QSize(preferedWidth, height()));
    }
    else
    {
        _dragIndicator->setPreferredSize(QSize(width(), preferedHeight));
    }
    _dragMoveOffset = QPoint(_dragIndicator->width()/2,
                             _dragIndicator->height()/2);

    // Find the container before the position of the dragindicator.
    BaseContainer::Iterator it = m_containers.end();

    if (it != m_containers.begin())
    {
        do
        {
            --it;
            BaseContainer* a = *it;

            if ((orientation() == Horizontal &&
                 a->x() < ev->pos().x() - _dragMoveOffset.x()) ||
                (orientation() == Vertical &&
                 a->y() < ev->pos().y() - _dragMoveOffset.y()))
            {
                _dragMoveAC = a;
                break;
            }
        } while (it != m_containers.begin());
    }

    if (orientation() == Horizontal)
    {
        moveDragIndicator((ev->pos() - _dragMoveOffset).x());
    }
    else
    {
        moveDragIndicator((ev->pos() - _dragMoveOffset).y());
    }

    _dragIndicator->show();
}

void ContainerArea::dragMoveEvent(QDragMoveEvent* ev)
{
    if (ev->source() == this)
    {
        // Abort the drag and go back to container sliding.
        // Actually, this should be placed in dragEnterEvent(), but
        // then it does work only on every second event.

        // Cancel the drag by faking an Escape keystroke.
        QKeyEvent fakedKeyPress(QEvent::KeyPress, Key_Escape, 0, 0);
        QKeyEvent fakedKeyRelease(QEvent::KeyRelease, Key_Escape, 0, 0);
        QApplication::sendEvent(this, &fakedKeyPress);
        QApplication::sendEvent(this, &fakedKeyRelease);
        qApp->processEvents();
        startContainerMove(_moveAC);

        // Align the container to the mouse position.
        if (orientation() == Horizontal)
        {
            m_layout->moveContainerSwitch(_moveAC, ev->pos().x() - _moveAC->x());
        }
        else
        {
            m_layout->moveContainerSwitch(_moveAC, ev->pos().y() - _moveAC->y());
        }
        return;
    }

    if (!_dragIndicator)
    {
        return;
    }

    if (orientation() == Horizontal)
    {
        moveDragIndicator((ev->pos() - _dragMoveOffset).x());
    }
    else
    {
        moveDragIndicator((ev->pos() - _dragMoveOffset).y());
    }
}

void ContainerArea::dragLeaveEvent(QDragLeaveEvent*)
{
    if (_dragIndicator)
    {
        _dragIndicator->hide();
    }
    m_layout->setStretchEnabled(true);
    _dragMoveAC = 0;
}

void ContainerArea::dropEvent(QDropEvent *ev)
{
    if (!_dragIndicator)
    {
        // we assume that this is the result of a successful drag enter
        // which means we'll have a _dragIndicator. if for
        // some reason we don't, let's not go down this code path
        return;
    }

    if (BaseContainer::canDecode(ev->mimeData()))
    {
        BaseContainer* a = BaseContainer::fromMimeData(ev->mimeData());
        if (!a)
        {
            _dragMoveAC = 0;
            _dragIndicator->hide();
            m_layout->setStretchEnabled(true);
            return;
        }

        QObject *parent = ev->source() ? ev->source()->parent() : 0;
        while (parent && (parent != this))
        {
            parent = parent->parent();
        }

        if (parent)
        {
            // Move container a
            if (orientation() == Horizontal)
            {
                int oldX = a->x();
                int x = _dragIndicator->x();
                m_layout->moveContainerSwitch(a, x - oldX);
            }
            else if (orientation() == Vertical)
            {
                int oldY = a->y();
                int y = _dragIndicator->y();
                m_layout->moveContainerSwitch(a, y - oldY);
            }

            _dragMoveAC = 0;
            _dragIndicator->hide();
            m_layout->setEnabled(true);
            m_layout->setStretchEnabled(true);
            saveContainerConfig(true);
            return;
        }

        // it came from another panel
        Kicker::self()->setInsertionPoint(_dragIndicator->pos());
        a->reparent(m_contents, 0, _dragIndicator->pos(), true);
        a->setAppletId(createUniqueId(a->appletType()));
        addContainer(a, true);
        Kicker::self()->setInsertionPoint(QPoint());
        m_layout->updateFreeSpaceValues();
        _dragMoveAC = 0;
        _dragIndicator->hide();
        m_layout->setStretchEnabled(true);
        saveContainerConfig();
        return;
    }

    // is it an applet info?
    if (AppletInfo::canDecode(ev->mimeData()))
    {
        AppletInfo info = AppletInfo::fromMimeData(ev->mimeData());
        Kicker::self()->setInsertionPoint(_dragIndicator->pos());
        _dragIndicator->hide();
        m_layout->setStretchEnabled(true);

        if (info.type() == AppletInfo::SpecialButton)
        {
            addButton(info);
        }
        else if (info.type() == AppletInfo::Applet)
        {
           addApplet(info.desktopFile());
        }

        Kicker::self()->setInsertionPoint(QPoint());
        return;
    }

    // ok, let's try a KUrl drag
    if (!KUrl::List::canDecode(ev->mimeData()))
    {
        _dragMoveAC = 0;
        _dragIndicator->hide();
        m_layout->setStretchEnabled(true);
        return;
    }

    Kicker::self()->setInsertionPoint(_dragIndicator->pos());

    KUrl::List uriList = KUrl::List::fromMimeData(ev->mimeData());
    KUrl::List::const_iterator it(uriList.begin());
    for (; it != uriList.constEnd(); ++it)
    {
        const KUrl &url = *it;
        BaseContainer* a = 0;

        // Create a new PanelButton for this URL.

        // see if it's a executable or directory
        if (url.protocol() == "programs")
        {
            QString relPath = url.path();
            if (relPath[0] == '/')
            {
                relPath = relPath.right(relPath.length() - 1);
            }
            a = new ServiceMenuButtonContainer(relPath, m_opMenu, m_contents);
        }
	else if (url.isLocalFile()) {
	    QFileInfo fi(url.path());
	    if (fi.isDir())
            {
                // directory
                QMenu* menu = new QMenu(this);
                QAction* fileManager = menu->addAction(SmallIconSet("folder"),
                                                       i18n("Add as &File Manager URL"),
                                                       0, 0, Qt::CTRL+Qt::Key_F);
                /* QAction* urlBrowser = */ menu->addAction(SmallIconSet("kdisknav"),
                                                      i18n("Add as Quick&Browser"),
                                                      0, 0, Qt::CTRL+Qt::Key_B);
                if (menu->exec(mapToGlobal(ev->pos())) == fileManager)
                {
                    a = new BrowserButtonContainer(url.path(), m_opMenu,
                                                   KMimeType::iconNameForURL(url),
                                                   m_contents);
                }
                else
                {
                    a = new URLButtonContainer(url.url(), m_opMenu, m_contents);
                }
                delete menu;
	    }
            else if ( KMimeType::findByURL(url)->name() == "application/x-desktop" ) {
		// a local desktop file being dragged from an external program.
		// Make a copy first.
		KDesktopFile df(url.path());
		KUrl newUrl;
		newUrl.setPath(Plasma::copyDesktopFile(url));
		if (df.readType() == "Link")
		   a = new URLButtonContainer(newUrl.url(), m_opMenu, m_contents);
		else
		   a = new ServiceButtonContainer(newUrl.path(), m_opMenu, m_contents);
	    }
            else if (fi.isExecutable())
            {
                // non-KDE executable
                QString pixmapFile;
                KMimeType::pixmapForURL(url, 0, KIcon::Panel, 0,
                                        KIcon::DefaultState, &pixmapFile);
                PanelExeDialog dlg(QString::null, QString::null, url.path(),
                                   pixmapFile, QString::null, false, 0);
                if (dlg.exec() == QDialog::Accepted)
                {
                    // KIconloader returns a full path, we only want the name
                    QFileInfo iconfi(dlg.iconPath());
                    a = new NonKDEAppButtonContainer(dlg.title(),
                                                     dlg.description(),
                                                     dlg.command(),
                                                     iconfi.fileName(),
                                                     dlg.commandLine(),
                                                     dlg.useTerminal(),
                                                     m_opMenu,
                                                     m_contents);
                }
            }
            else // some unknown local file
            {
                a = new URLButtonContainer(url.url(), m_opMenu, m_contents);
            }
        }
        else // a internet URL
        {
            a = new URLButtonContainer(url.url(), m_opMenu, m_contents);
        }

        if (!a)
        {
            _dragIndicator->hide();
            Kicker::self()->setInsertionPoint(QPoint());
            m_layout->setStretchEnabled(true);
            return;
        }

        // Move the neighbour containers if there isn't enough space
        if (_dragIndicator->size() != _dragIndicator->preferredSize()) {
            int neededSpace;
            int distanceMoved;
            BaseContainer* next = 0;

            if (_dragMoveAC)
            {
                BaseContainer::Iterator it = m_containers.find(a);
                if (it != m_containers.end() &&
                        ++it != m_containers.end())
                {
                    next = (*it);
                }
            }

            if (!next)
            {
                BaseContainer::Iterator it = m_containers.begin();
                if (it != m_containers.end())
                {
                    next = (*it);
                }
            }

            if (orientation() == Horizontal) {
                neededSpace = _dragIndicator->preferredSize().width() - _dragIndicator->width();
                if (_dragMoveAC) {
                    distanceMoved = m_layout->moveContainerPush(_dragMoveAC, -neededSpace/2);
                    _dragIndicator->move(
                            _dragIndicator->x() + distanceMoved,
                            _dragIndicator->y());
                    neededSpace += distanceMoved;
                }
                if (next)
                    neededSpace -= m_layout->moveContainerPush(next, neededSpace);
                if (_dragMoveAC) {
                    distanceMoved = m_layout->moveContainerPush(_dragMoveAC, -neededSpace);
                    _dragIndicator->move(
                            _dragIndicator->x() + distanceMoved,
                            _dragIndicator->y());
                }
            } else {
                neededSpace = _dragIndicator->preferredSize().height() - _dragIndicator->height();
                if (_dragMoveAC) {
                    distanceMoved = m_layout->moveContainerPush(_dragMoveAC, -neededSpace/2);
                    _dragIndicator->move(
                            _dragIndicator->x(),
                            _dragIndicator->y() + distanceMoved);
                    neededSpace += distanceMoved;
                }
                if (next)
                    neededSpace -= m_layout->moveContainerPush(next, neededSpace);
                if (_dragMoveAC) {
                    distanceMoved = m_layout->moveContainerPush(_dragMoveAC, -neededSpace);
                    _dragIndicator->move(
                            _dragIndicator->x(),
                            _dragIndicator->y() + distanceMoved);
                }
            }
        }

        addContainer(a, true);
        m_layout->updateFreeSpaceValues();
    }

    saveContainerConfig();
    _dragMoveAC = 0;
    _dragIndicator->hide();
    Kicker::self()->setInsertionPoint(QPoint());
    m_layout->setStretchEnabled(true);
}

bool ContainerArea::eventFilter(QObject* o, QEvent* e)
{
    // Propagate the layout hints which m_contents receives. This way widgets
    // which contain a ContainerArea can react to layout changes of its
    // contents. For example: If an applets grows, the top level widget may
    // want to grow as well.
    if (o == m_contents)
    {
        if (e->type() == QEvent::LayoutRequest)
        {
            updateGeometry(); // Posts a new layout hint to our parent.
        }
        return false;
    }

    return Panner::eventFilter(o, e);
}

void ContainerArea::viewportResizeEvent(QResizeEvent* ev)
{
    Panner::resizeEvent(ev);
    if (orientation() == Horizontal)
    {
        m_contents->resize(qMax(widthForHeight(ev->size().height()),
                                ev->size().width()),
                           ev->size().height());
    }
    else
    {
        m_contents->resize(ev->size().width(),
                           qMax(heightForWidth(ev->size().width()),
                                ev->size().height()));
    }

    Panner::resize(m_contents->width(), m_contents->height());
}

void ContainerArea::immutabilityChanged(bool immutable)
{
    // we set all the child container's immutability here instead of connecting
    // the immutabilityChanged signal up everywhere so that we can control the
    // order of immutability changing and the background being updated. since
    // immutability implies applet handle visibility, those things must happen
    // first before updating our background.
    foreach ( BaseContainer *it, m_containers )
    {
        it->setImmutable( immutable );
    }
}

QRect ContainerArea::availableSpaceFollowing(BaseContainer* a)
{
    QRect availableSpace = rect();
    BaseContainer* b = 0;

    if (a)
    {
        BaseContainer::Iterator it = m_containers.find(a);
        if (it != m_containers.end() &&
            ++it != m_containers.end())
        {
            b = (*it);
        }
    }

    if (!b)
    {
        BaseContainer::Iterator it = m_containers.begin();
        if (it != m_containers.end())
        {
            b = (*it);
        }
    }

    if (orientation() == Horizontal)
    {
        if (a)
        {
            availableSpace.setLeft(a->x() + a->width());
        }

        if (b)
        {
            availableSpace.setRight(b->x() - 1);
        }
    }
    else
    {
        if (a)
        {
            availableSpace.setTop(a->y() + a->height());
        }

        if (b)
        {
            availableSpace.setBottom(b->y() - 1);
        }
    }

    return availableSpace;
}

void ContainerArea::moveDragIndicator(int pos)
{
    QRect availableSpace = availableSpaceFollowing(_dragMoveAC);

    // Move _dragIndicator to position pos, restricted by availableSpace.
    // Resize _dragIndicator if necessary.
    if (orientation() == Horizontal)
    {
        if (availableSpace.size().width() <
            _dragIndicator->preferredSize().width())
        {
            _dragIndicator->resize(availableSpace.size());
            _dragIndicator->move(availableSpace.topLeft());
        }
        else
        {
            int newX = pos;
            _dragIndicator->resize(_dragIndicator->preferredSize());
            newX = qMax(newX, availableSpace.left());
            newX = qMin(newX,
                availableSpace.right() + 1 - _dragIndicator->width() );
            _dragIndicator->move(newX, availableSpace.top());
        }
    }
    else
    {
        if (availableSpace.size().height() <
            _dragIndicator->preferredSize().height())
        {
            _dragIndicator->resize(availableSpace.size());
            _dragIndicator->move(availableSpace.topLeft());
        }
        else
        {
            int newY = pos;
            _dragIndicator->resize(_dragIndicator->preferredSize());
            newY = qMax(newY, availableSpace.top());
            newY = qMin(newY,
                availableSpace.bottom() + 1 - _dragIndicator->height() );
            _dragIndicator->move(availableSpace.left(), newY);
        }
    }
}

void ContainerArea::slotSaveContainerConfig()
{
    saveContainerConfig();
}

void ContainerArea::setPosition(Plasma::Position p)
{
    if (_pos == p)
    {
        return;
    }

    _pos = p;
    Qt::Orientation o = (p == Plasma::Top ||
                         p == Plasma::Bottom) ?
                        Qt::Horizontal : Qt::Vertical;
    bool orientationChanged = (orientation() != o);
    m_layout->setEnabled(false);

    if (orientationChanged)
    {
        setOrientation(o);
        m_layout->setOrientation(o);

        // when we change orientation, we will resize the "width"
        // component down to 0, forcing a resize in resizeContents()
        // when that gets called AFTER we've been moved
        // it's not always safe to do the resize here, as scroll buttons
        // from the panner may get in our way. =/
        if (o == Horizontal)
        {
            Panner::resize(0, height());
        }
        else
        {
            Panner::resize(width(), 0);
        }
    }

    foreach (BaseContainer *it, m_containers)
    {
        if (orientationChanged)
        {
            it->setOrientation(o);
        }

        it->setPopupDirection(popupDirection());
    }

    m_layout->setEnabled(true);

    // container extension repaints for us!
    //repaint();
}

void ContainerArea::setAlignment(Plasma::Alignment a)
{
    foreach (BaseContainer *it, m_containers)
    {
        it->setAlignment(a);
    }
}

void ContainerArea::autoScroll()
{
    if(!_moveAC) return;

    if(orientation() == Horizontal) {
        if(_moveAC->pos().x() <= 80)
        {
            scroll(-10, 0);
        }
        else if(_moveAC->pos().x() >= width() - _moveAC->width() - 80)
        {
            scroll(10, 0);
        }
    }
    else {
        if(_moveAC->pos().y() <= 80)
        {
            scroll(0, -10);
        }
        else if(_moveAC->pos().y() >= height() - _moveAC->height() - 80)
        {
            scroll(0, 10);
        }
    }
}

void ContainerArea::scrollTo(BaseContainer* b)
{
    if (!b)
    {
        return;
    }

#warning FIX THIS
    //int x, y;
    //viewportToContents(b->pos().x(), b->pos().y(), x, y);
    //ensureVisible(x, y);
}

BaseContainer::List ContainerArea::containers(const QString& type) const
{
    if (type.isEmpty() || type == "All")
    {
        return m_containers;
    }

    BaseContainer::List list;

    if (type == "Special Button")
    {
        foreach ( BaseContainer *it, m_containers )
        {
            QString type = it->appletType();
            if (type == "KMenuButton" ||
                type == "WindowListButton" ||
                type == "BookmarksButton" ||
                type == "DesktopButton" ||
                type == "BrowserButton" ||
                type == "ExecButton" ||
                type == "ExtensionButton")
            {
                list.append(it);
            }
        }

        return list;
    }

    foreach ( BaseContainer *it, m_containers )
    {
        if ( it->appletType() == type )
        {
            list.append( it );
        }
    }

    return list;
}

int ContainerArea::containerCount(const QString& type) const
{
    if (type.isEmpty() || type == "All")
    {
        return m_containers.count();
    }

    int count = 0;
    if (type == "Special Button")
    {
        foreach ( BaseContainer *it, m_containers )
        {
            QString type = it->appletType();
            if (type == "KMenuButton" ||
                type == "WindowListButton" ||
                type == "BookmarksButton" ||
                type == "DesktopButton" ||
                type == "BrowserButton" ||
                type == "ExecButton" ||
                type == "ExtensionButton")
            {
                ++count;
            }
        }

        return count;
    }

    foreach ( BaseContainer *it, m_containers )
    {
        if ( it->appletType() == type )
        {
            ++count;
        }
    }

    return count;
}

void ContainerArea::repaint()
{
    Panner::repaint();
}

void ContainerArea::showAddAppletDialog()
{
    if (!m_addAppletDialog)
    {
        m_addAppletDialog = new AddAppletDialog(this, this, 0);
        connect(m_addAppletDialog, SIGNAL(finished()), this, SLOT(addAppletDialogDone()));
    }
    else
    {
        // this ensures that if we get shown again via the menu
        // that the dialog picks up
        // the new place to insert things
        m_addAppletDialog->updateInsertionPoint();
    }

    KWin::setOnDesktop(m_addAppletDialog->winId(), KWin::currentDesktop());
    m_addAppletDialog->show();
    m_addAppletDialog->raise();
}

void ContainerArea::addAppletDialogDone()
{
    m_addAppletDialog->deleteLater();
    m_addAppletDialog = 0;
}

int ContainerArea::widthForHeight(int h) const
{
    return m_layout->widthForHeight(h);
}

int ContainerArea::heightForWidth(int w) const
{
    return m_layout->heightForWidth(w);
}


void DragIndicator::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QStyleOptionFocusRect frOpt;
    frOpt.init(this);
    frOpt.backgroundColor = palette().base();
    style()->drawPrimitive( QStyle::PE_FrameFocusRect, &frOpt, &painter, this );
}

void DragIndicator::mousePressEvent(QMouseEvent*)
{
    hide();
}

#include "containerarea.moc"
