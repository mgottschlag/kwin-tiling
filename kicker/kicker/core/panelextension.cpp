/*****************************************************************

Copyright (c) 2000 Matthias Elter
              2004 Aaron J. Seigo <aseigo@kde.org>

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

#include <QValidator>
#include <QLayout>
#include <QPainter>
#include <QStyle>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QEvent>
#include <QByteArray>
#include <QMenu>

#include <kdebug.h>
#include <khelpmenu.h>
#include <klocale.h>
#include <kglobal.h>
#include <kguiitem.h>
#include <kmenu.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kstdguiitem.h>
#include <kauthorized.h>
#include <QtDBus/qdbusconnection.h>

#include "container_applet.h"
#include "container_extension.h"
#include "containerarea.h"
#include "extensionmanager.h"
#include "kicker.h"
#include "removecontainer_mnu.h"
#include "removeextension_mnu.h"

#include "addapplet_mnu.h"
#include "addbutton_mnu.h"
#include "addextension_mnu.h"

#include "panelextension.h"
#include "panelextension.moc"

PanelExtension::PanelExtension(const QString& configFile, QWidget *parent)
    : KPanelExtension(configFile, 0, parent),
      m_opMenu(0),
      m_panelAddMenu(0),
      m_removeMenu(0),
      m_addExtensionMenu(0),
      m_removeExtensionMenu(0),
      m_configFile(configFile),
      m_opMenuBuilt(false)
{
    QString nameRegister = QString("/Panel_") + QString::number((ulong)this).toLatin1();
    QDBus::sessionBus().registerObject(nameRegister, this, QDBusConnection::ExportSlots);
    setAcceptDrops(!Kicker::self()->isImmutable());
    setCustomMenu( opMenu() );

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);

    // container area
    _containerArea = new ContainerArea( config(), this, opMenu() );
    connect(_containerArea, SIGNAL(maintainFocus(bool)), this, SIGNAL(maintainFocus(bool)));
    layout->addWidget(_containerArea);

    _containerArea->setFrameStyle(QFrame::NoFrame);
    _containerArea->viewport()->installEventFilter(this);
    _containerArea->configure();

    // Make sure the containerarea has the right orientation from the
    // beginning.
    positionChange(position());

    connect(Kicker::self(), SIGNAL(configurationChanged()),
            SLOT(configurationChanged()));
    connect(Kicker::self(), SIGNAL(immutabilityChanged(bool)),
            SLOT(immutabilityChanged(bool)));

    // we wait to get back to the event loop to start up the container area so that
    // the main panel in ExtensionManager will be assigned and we can tell in a
    // relatively non-hackish way that we are (or aren't) the "main panel"
    QTimer::singleShot(0, this, SLOT(populateContainerArea()));
}

PanelExtension::~PanelExtension()
{
}

void PanelExtension::populateContainerArea()
{
    _containerArea->show();

    if (ExtensionManager::self()->isMainPanel(topLevelWidget()))
    {
#warning "kde4 port it"
        //setObjId("Panel");
        _containerArea->initialize(true);
    }
    else
    {
        _containerArea->initialize(false);
    }
}

void PanelExtension::configurationChanged()
{
    _containerArea->configure();
}

void PanelExtension::immutabilityChanged(bool)
{
    m_opMenuBuilt = false;
}

QMenu* PanelExtension::opMenu()
{
    if (m_opMenu)
    {
        return m_opMenu;
    }

    m_opMenu = new QMenu(this);
    connect(m_opMenu, SIGNAL(aboutToShow()), this, SLOT(slotBuildOpMenu()));

    return m_opMenu;
}

void PanelExtension::positionChange(Plasma::Position p)
{
    _containerArea->setPosition(p);
}

QSize PanelExtension::sizeHint(Plasma::Position p, QSize maxSize) const
{
    QSize size;

    if (p == Plasma::Left || p == Plasma::Right)
    {
        size = QSize(sizeInPixels(),
                     _containerArea->heightForWidth(sizeInPixels()));
    }
    else
    {
        size = QSize(_containerArea->widthForHeight(sizeInPixels()),
                     sizeInPixels());
    }

    return size.boundedTo(maxSize);
}

bool PanelExtension::eventFilter(QObject*, QEvent * e)
{
    if ( e->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent* me = (QMouseEvent*) e;
        if ( me->button() == Qt::RightButton && KAuthorized::authorizeKAction("action/kicker_rmb"))
        {
            Kicker::self()->setInsertionPoint(me->globalPos());
            opMenu()->exec(me->globalPos());
            Kicker::self()->setInsertionPoint(QPoint());
            return true;
        }
    }
    else
    if ( e->type() == QEvent::Resize )
    {
        emit updateLayout();
    }

    return false;
}

void PanelExtension::setPanelSize(int size)
{
    setSize(static_cast<Plasma::Size>(size), customSize());

    // save the size setting here if it isn't a custom setting
    config()->setGroup("General");
    config()->writeEntry("Size", size);
    config()->sync();
}

void PanelExtension::addKMenuButton()
{
    _containerArea->addKMenuButton();
}

void PanelExtension::addDesktopButton()
{
    _containerArea->addDesktopButton();
}

void PanelExtension::addWindowListButton()
{
    _containerArea->addWindowListButton();
}

void PanelExtension::addURLButton(const QString &url)
{
    _containerArea->addURLButton(url);
}

void PanelExtension::addBrowserButton(const QString &startDir)
{
    _containerArea->addBrowserButton(startDir);
}

void PanelExtension::addServiceButton(const QString& desktopEntry)
{
    _containerArea->addServiceButton(desktopEntry);
}

void PanelExtension::addServiceMenuButton(const QString &,
                                          const QString& relPath)
{
    _containerArea->addServiceMenuButton(relPath);
}

void PanelExtension::addNonKDEAppButton(const QString &filePath,
                                        const QString &icon,
                                        const QString &cmdLine, bool inTerm)
{
    _containerArea->addNonKDEAppButton(filePath, QString(), filePath, icon,
                                       cmdLine, inTerm);
}

void PanelExtension::addNonKDEAppButton(const QString &title,
                                        const QString &description,
                                        const QString &filePath,
                                        const QString &icon,
                                        const QString &cmdLine, bool inTerm)
{
    _containerArea->addNonKDEAppButton(title, description, filePath, icon,
                                       cmdLine, inTerm);
}

void PanelExtension::addApplet(const QString &desktopFile)
{
    _containerArea->addApplet(AppletInfo(desktopFile, QString::null, AppletInfo::Applet));
}

void PanelExtension::slotBuildOpMenu()
{
    const int REMOVE_EXTENSION_ID = 1000;
    if (m_opMenuBuilt || !m_opMenu)
    {
        if (m_opMenu)
        {
            bool haveExtensions = ExtensionManager::self()->containers().count() > 0;
            m_opMenu->setItemEnabled(REMOVE_EXTENSION_ID, haveExtensions);
        }

        return;
    }

    m_opMenu->clear();

    delete m_panelAddMenu;
    m_panelAddMenu = 0;
    delete m_removeMenu;
    m_removeMenu = 0;
    delete m_addExtensionMenu;
    m_addExtensionMenu = 0;
    delete m_removeExtensionMenu;
    m_removeExtensionMenu = 0;

    m_opMenuBuilt = true;
    bool kickerImmutable = Kicker::self()->isImmutable();
    if (!kickerImmutable)
    {
        bool isMenuBar = ExtensionManager::self()->isMenuBar(
                                            dynamic_cast<QWidget*>(parent()));

        // setup addmenu and removemenu
        if (_containerArea->canAddContainers())
        {
            m_opMenu->insertItem(isMenuBar ? i18n("Add &Applet to Menubar")
                                         : i18n("Add &Applet to Panel..."),
                               _containerArea, SLOT(showAddAppletDialog()));
            m_panelAddMenu = new PanelAddButtonMenu(_containerArea, this);
            m_opMenu->insertItem(isMenuBar ? i18n("Add Appli&cation to Menubar")
                                         : i18n("Add Appli&cation to Panel"),
                               m_panelAddMenu);

            m_removeMenu = new RemoveContainerMenu(_containerArea, this);
            m_opMenu->insertItem(isMenuBar ? i18n("&Remove from Menubar")
                                         : i18n("&Remove From Panel"),
                                         m_removeMenu);
            m_opMenu->addSeparator();

            m_addExtensionMenu = new PanelAddExtensionMenu(this);
            m_opMenu->insertItem(i18n("Add New &Panel"), m_addExtensionMenu);
            m_removeExtensionMenu = new PanelRemoveExtensionMenu(this);
            m_opMenu->insertItem(i18n("Remove Pa&nel"), m_removeExtensionMenu,
                               REMOVE_EXTENSION_ID);
            m_opMenu->setItemEnabled(REMOVE_EXTENSION_ID,
                            ExtensionManager::self()->containers().count() > 0);
            m_opMenu->addSeparator();
        }

        if (!isMenuBar)
        {
            m_opMenu->insertItem(SmallIconSet("lock"), i18n("&Lock Panel"),
                               Kicker::self(), SLOT(toggleLock()));

            m_opMenu->insertItem(SmallIconSet("configure"),
                               i18n("&Configure Panel..."),
                               this, SLOT(showConfig()));
            m_opMenu->addSeparator();
        }
    }
    else if (!Kicker::self()->isKioskImmutable())
    {
        m_opMenu->insertItem(kickerImmutable? SmallIconSet("unlock") :
                                            SmallIconSet("lock"),
                           kickerImmutable ? i18n("Un&lock Panels") :
                                             i18n("&Lock Panels"),
                           Kicker::self(), SLOT(toggleLock()));
    }

    if (KAuthorized::authorizeKAction("action/help"))
    {
        KHelpMenu* help = new KHelpMenu( this, KGlobal::instance()->aboutData(), false);
        m_opMenu->insertItem(SmallIconSet("help"), KStdGuiItem::help().text(), help->menu());
    }
    m_opMenu->adjustSize();
}

void PanelExtension::showConfig()
{
    Kicker::self()->showConfig(m_configFile);
}

MenubarExtension::MenubarExtension(const AppletInfo& info)
    : PanelExtension(info.configFile()),
      m_menubar(0)
{
}

MenubarExtension::~MenubarExtension()
{
    if (m_menubar)
    {
        m_menubar->setImmutable(false);
        _containerArea->slotSaveContainerConfig();
    }
}

void MenubarExtension::populateContainerArea()
{
    PanelExtension::populateContainerArea();
    BaseContainer::List containers = _containerArea->containers("All");
    for (BaseContainer::Iterator it = containers.begin();
         it != containers.end();
         ++it)
    {
        if ((*it)->appletType() == "Applet")
        {
            AppletContainer* applet = dynamic_cast<AppletContainer*>(*it);
            if (applet && applet->info().desktopFile() == "menuapplet.desktop")
            {
                m_menubar = applet;
                break;
            }
        }
    }

    if (!m_menubar)
    {
        m_menubar = _containerArea->addApplet(AppletInfo("menuapplet.desktop",
                                                         QString::null,
                                                         AppletInfo::Applet));
    }

    // in the pathological case we may not have a menuapplet at all,
    // so check for it =/
    if (m_menubar)
    {
        m_menubar->setImmutable(true);
    }
}

