/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>

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

#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

#include <QtDBus/QtDBus>

#include <QList>
#include <QDesktopWidget>

#include <kaboutdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kwinmodule.h>

#include "utils.h"
#include "kicker.h"
#include "panelextension.h"
#include "pluginmanager.h"

#include "extensionmanager.h"

static KStaticDeleter<ExtensionManager> extensionManagerDeleter;
ExtensionManager* ExtensionManager::m_self = 0;

ExtensionManager* ExtensionManager::self()
{
    if (!m_self)
    {
        extensionManagerDeleter.setObject(m_self, new ExtensionManager());
    }

    return m_self;
}

ExtensionManager::ExtensionManager()
    : AbstractPanelManager(),
      m_menubarPanel(0),
      m_mainPanel(0),
      m_panelCounter(-1)
{
    setObjectName("ExtensionManager");
}

ExtensionManager::~ExtensionManager()
{
    if (this == m_self)
    {
        m_self = 0;
    }

    while (!_containers.isEmpty())
    {
        delete _containers.takeFirst();
    }

    delete m_menubarPanel;
    delete m_mainPanel;
    m_self= 0;
}

void ExtensionManager::initialize()
{
//    kDebug(1210) << "ExtensionManager::loadContainerConfig()" << endl;
    KConfig* config = KGlobal::config();
    PluginManager* pm = PluginManager::self();

    // set up the "main" panel
    if (config->hasGroup("Main Panel"))
    {
        config->setGroup("Main Panel");
        if (config->hasKey("DesktopFile"))
        {
            m_mainPanel = pm->createExtensionContainer(config->readPathEntry("DesktopFile"),
                                                       true, config->readPathEntry("ConfigFile"),
                                                       "Main Panel");
        }
    }

    if (!m_mainPanel)
    {
        // fall back to a regular ol' PanelExtension
        m_mainPanel = pm->createExtensionContainer(
                            "childpanelextension.desktop",
                            true,
                            QString(kapp->aboutData()->appName()) + "rc",
                            "Main Panel");
    }

    if (!m_mainPanel)
    {
        KMessageBox::error(0, i18n("The KDE panel (kicker) could not load the main panel "
                                   "due to a problem with your installation. "),
                           i18n("Fatal Error"));
        exit(1);
    }

    configureMenubar(true);

    m_mainPanel->readConfig();
    m_mainPanel->show();
    kapp->processEvents();

    // read extension list
    config->setGroup("General");
    QStringList elist = config->readEntry("Extensions2", QStringList() );

    // now restore the extensions
    foreach (QString extensionId, elist)
    {
        // create a matching applet container
        if (extensionId.indexOf("Extension") == -1)
        {
            continue;
        }

        // is there a config group for this extension?
        if (!config->hasGroup(extensionId))
        {
            continue;
        }

        // set config group
        config->setGroup(extensionId);

        ExtensionContainer* e =
            pm->createExtensionContainer(config->readPathEntry("DesktopFile"),
                                         true, // is startup
                                         config->readPathEntry("ConfigFile"),
                                         extensionId);

        if (e)
        {
            addContainer(e);
            e->readConfig();
            e->show();
            kapp->processEvents();
        }
    }

    pm->clearUntrustedLists();
    connect(Kicker::self(), SIGNAL(configurationChanged()), SLOT(configurationChanged()));

    QDBusInterface dbus("org.kde.ksmserver", "/ksmserver");
    dbus.call("resumeStartup", "kicker");
}

void ExtensionManager::configureMenubar(bool duringInit)
{
    KConfig menuConfig("kdesktoprc", true);
    if (KConfigGroup(&menuConfig, "KDE").readEntry("macStyle", QVariant(false)).toBool()
        || KConfigGroup(&menuConfig, "Menubar").readEntry("ShowMenubar", QVariant(false)).toBool())
    {
        if (KGlobal::dirs()->findResource("applets", "menuapplet.desktop").isEmpty() ||
            m_menubarPanel)
        {
            return;
        }

        if (duringInit)
        {
            AppletInfo menubarInfo("menuapplet.desktop", QString::null, AppletInfo::Applet);
            if (PluginManager::self()->hasInstance(menubarInfo))
            {
                // it's already there, in the main panel!
                return;
            }
            migrateMenubar();
        }

        AppletInfo info("childpanelextension.desktop",
                        "kicker_menubarpanelrc",
                        AppletInfo::Extension);
        KPanelExtension* menubar = new MenubarExtension(info);
        m_menubarPanel = new ExtensionContainer(menubar, info, "Menubar Panel");
        m_menubarPanel->setPanelOrder(-1);
        m_menubarPanel->readConfig();
        m_menubarPanel->setPosition(Plasma::Top);
        m_menubarPanel->setXineramaScreen(Plasma::XineramaAllScreens);
        m_menubarPanel->setHideButtons(false, false);

        // this takes care of resizing the panel so it shows with the right height
        updateMenubar();

        m_menubarPanel->show();
        connect(kapp, SIGNAL(kdisplayFontChanged()), SLOT(updateMenubar()));
    }
    else if (m_menubarPanel)
    {
        delete m_menubarPanel;
        m_menubarPanel = 0;
    }
}

void ExtensionManager::migrateMenubar()
{
    // lame, lame, lame.
    // the menubar applet was just plunked into kicker and not much
    // thought was put into how it should be used. great idea, but no
    // integration. >:-(
    // so now we have to check to see if we HAVE another extension that
    // will have a menubar in it, and if so, abort creating one of our
    // own.
    //
    // the reason i didn't do this as a kconfig_update script is that
    // most people don't use this feature, so no reason to penalize
    // everyone, and moreover the user may added this to their main
    // panel, meaning kickerrc itself would have to be vastly modified
    // with lots of complications. not work it IMHO.

    KConfig* config = KGlobal::config();
    config->setGroup("General");

    if (config->readEntry("CheckedForMenubar", QVariant(false)).toBool())
    {
        return;
    }

    if (!locate("config", "kicker_menubarpanelrc").isEmpty())
    {
        // don't overwrite/override something that's already there
        return;
    }

    QStringList elist = config->readEntry("Extensions2", QStringList() );
    foreach (QString extensionId, elist)
    {
        if (extensionId.indexOf("Extension") == -1)
        {
            continue;
        }

        // is there a config group for this extension?
        if (!config->hasGroup(extensionId))
        {
            continue;
        }

        config->setGroup(extensionId);
        QString extension = config->readPathEntry("ConfigFile");
        KConfig extensionConfig(locate("config", extension));
        extensionConfig.setGroup("General");

        if (extensionConfig.hasKey("Applets2"))
        {
            QStringList containers = extensionConfig.readEntry("Applets2", QStringList() );
            foreach (QString appletId, containers)
            {
                // is there a config group for this applet?
                if (!extensionConfig.hasGroup(appletId))
                {
                    continue;
                }

                KConfigGroup group(&extensionConfig, appletId.toLatin1());
                QString appletType = appletId.left(appletId.lastIndexOf('_'));

                if (appletType == "Applet")
                {
                    QString appletFile = group.readPathEntry("DesktopFile");
                    if (appletFile.indexOf("menuapplet.desktop") != -1)
                    {
                        QString menubarConfig = locate("config", extension);
                        KIO::NetAccess::copy(menubarConfig,
                                             locateLocal("config",
                                             "kicker_menubarpanelrc"), 0);
                        elist.removeAll(appletId);
                        config->setGroup("General");
                        config->writeEntry("Extensions2", elist);
                        config->writeEntry("CheckedForMenubar", true);
                        config->sync();
                        return;
                    }
                }
            }
        }
    }

    config->setGroup("General");
    config->writeEntry("CheckedForMenubar", true);
}

void ExtensionManager::saveContainerConfig()
{
//    kDebug(1210) << "ExtensionManager::saveContainerConfig()" << endl;

    KConfig *config = KGlobal::config();

    // build the extension list
    QStringList elist;
    foreach (ExtensionContainer *it, _containers)
    {
        elist.append(it->extensionId());
    }

    // write extension list
    config->setGroup("General");
    config->writeEntry("Extensions2", elist);

    config->sync();
}

void ExtensionManager::configurationChanged()
{
    m_mainPanel->readConfig();

    if (m_menubarPanel)
    {
        m_menubarPanel->readConfig();
    }

    foreach (ExtensionContainer *it, _containers)
    {
        it->readConfig();
    }
}

void ExtensionManager::updateMenubar()
{
    if (!m_menubarPanel)
    {
        return;
    }

    // we need to make sure the panel is tall enough to accommodate the
    // menubar! an easy way to find out the height of a menu: make one ;)
    KMenuBar tmpmenu;
    tmpmenu.insertItem("Aaron Seigo");
    m_menubarPanel->setSize(Plasma::SizeCustom,
                            tmpmenu.sizeHint().height());
}

bool ExtensionManager::isMainPanel(const QWidget* panel) const
{
    return m_mainPanel == panel;
}

bool ExtensionManager::isMenuBar(const QWidget* panel) const
{
    return m_menubarPanel == panel;
}

void ExtensionManager::addExtension( const QString& desktopFile )
{
    PluginManager* pm = PluginManager::self();
    ExtensionContainer *e = pm->createExtensionContainer(desktopFile,
                                                         false, // is not startup
                                                         QString(), // no config
                                                         uniqueId());

    kDebug(1210) << "ExtensionManager::addExtension" << endl;

    if (e)
    {
        e->readConfig();
        kDebug(1210)<<"after e->readConfig(): pos="<<e->position()<<endl;
        addContainer(e);
        e->show();
        e->writeConfig();
        saveContainerConfig();
    }
}

void ExtensionManager::addContainer(ExtensionContainer* e)
{
    if (!e)
    {
        return;
    }

    _containers.append(e);

    connect(e, SIGNAL(removeme(ExtensionContainer*)),
            this, SLOT(removeContainer(ExtensionContainer*)));
}

void ExtensionManager::removeContainer(ExtensionContainer* e)
{
    if (!e)
    {
        return;
    }

    e->removeSessionConfigFile();
    _containers.removeAll(e);
    e->deleteLater(); // Wait till we return to the main event loop
    saveContainerConfig();
}

void ExtensionManager::removeAllContainers()
{
    while (!_containers.isEmpty())
    {
        ExtensionContainer *e = _containers.takeFirst();
        e->deleteLater();
    }

    saveContainerConfig();
}

QString ExtensionManager::uniqueId()
{
    QString idBase = "Extension_%1";
    QString newId;
    int i = 0;
    bool unique = false;

    while (!unique)
    {
        i++;
        newId = idBase.arg(i);

        unique = true;
        foreach (ExtensionContainer *it, _containers)
        {
            if (it->extensionId() == newId)
            {
                unique = false;
                break;
            }
        }
    }

    return newId;
}

Plasma::Position ExtensionManager::initialPanelPosition(Plasma::Position preferred)
{
    // Guess a good position
    bool positions[Plasma::Bottom+1];
    for( int i = 0; i <= (int) Plasma::Bottom; ++i )
    {
       positions[i] = true;
    }

    foreach (ExtensionContainer *it, _containers)
    {
       positions[(int) it->position()] = false;
    }

    Plasma::Position pos = preferred;
    if (positions[(int)pos])
       return pos;

    pos = (Plasma::Position) (pos ^ 1);
    if (positions[(int)pos])
       return pos;

    pos = (Plasma::Position) (pos ^ 3);
    if (positions[(int)pos])
       return pos;

    pos = (Plasma::Position) (pos ^ 1);
    if (positions[(int)pos])
       return pos;

    return preferred;
}

bool ExtensionManager::shouldExclude(int XineramaScreen,
                                     ExtensionContainer* extension,
                                     ExtensionContainer* exclude)
{
    // Rules of Exclusion:
    // 0. Exclude ourselves
    // 1. Exclude panels not on our Xinerama screen
    // 2. Exclude panels on the same side of the screen as ourselves that are above us
    // 3. Exclude panels on the opposite side of the screen. Breaks down if the user
    //    dabbles in insane layouts where a top/bottom or left/right pair overlap?
    // 4. Exclude panels on adjacent sides of the screen that do not overlap with us

    if (exclude->winId() == extension->winId())
    {
        // Rule 0 Exclusion
        return true;
    }

    if (extension->xineramaScreen()!= Plasma::XineramaAllScreens &&
        exclude->xineramaScreen() != Plasma::XineramaAllScreens &&
        exclude->xineramaScreen() != XineramaScreen)
    {
        // Rule 1 exclusion
        return true;
    }

    if (!exclude->reserveStrut())
    {
        return true;
    }

    bool lowerInStack = extension->panelOrder() < exclude->panelOrder();
    if (exclude->position() == extension->position())
    {
        // Rule 2 Exclusion
        if (extension->position() == Plasma::Bottom &&
            exclude->geometry().bottom() == extension->geometry().bottom() &&
            !exclude->geometry().intersects(extension->geometry()))
        {
            return false;
        }
        else if (extension->position() == Plasma::Top &&
                 exclude->geometry().top() == extension->geometry().top() &&
                 !exclude->geometry().intersects(extension->geometry()))
        {
            return false;
        }
        else if (extension->position() == Plasma::Left &&
                 exclude->geometry().left() == extension->geometry().left() &&
                 !exclude->geometry().intersects(extension->geometry()))
        {
            return false;
        }
        else if (extension->position() == Plasma::Right &&
                 exclude->geometry().right() == extension->geometry().right() &&
                 !exclude->geometry().intersects(extension->geometry()))
        {
            return false;
        }

        return lowerInStack;
    }

    // Rule 3 exclusion
    if (exclude->orientation() == extension->orientation())
    {
        // on the opposite side of the screen from us.
        return true;
    }

    // Rule 4 exclusion
    if (extension->position() == Plasma::Bottom)
    {
        if (exclude->geometry().bottom() > extension->geometry().top())
        {
            return lowerInStack;
        }
    }
    else if (extension->position() == Plasma::Top)
    {
        if (exclude->geometry().top() < extension->geometry().bottom())
        {
            return lowerInStack;
        }
    }
    else if (extension->position() == Plasma::Left)
    {
        if (exclude->geometry().left() < extension->geometry().right())
        {
            return lowerInStack;
        }
    }
    else /* if (extension->position() == KPanelExtension::Right) */
    {
        if (exclude->geometry().right() > extension->geometry().left())
        {
            return lowerInStack;
        }
    }

    return true;
}

QRect ExtensionManager::workArea(ExtensionContainer* extension,
                                 int XineramaScreen)
{
    if (!extension)
    {
        return Kicker::self()->kwinModule()->workArea(XineramaScreen);
    }

    QList<WId> list;

    // If the hide mode is Manual, exclude the struts of
    // panels below this one in the list. Else exclude the
    // struts of all panels.
    if (extension->reserveStrut() &&
        extension != m_menubarPanel &&
        extension->hideMode() == ExtensionContainer::ManualHide)
    {
        if (m_mainPanel && shouldExclude(XineramaScreen, extension, m_mainPanel))
        {
            list.append(m_mainPanel->winId());
        }

        foreach (ExtensionContainer *it, _containers)
        {
            if (shouldExclude(XineramaScreen, extension, it))
            {
                list.append(it->winId());
            }
        }
    }
    else
    {
        // auto hide panel? just ignore everything else for now.
        if (extension == m_menubarPanel)
        {
            list.append(m_menubarPanel->winId());
        }

        if (m_mainPanel)
        {
            list.append(m_mainPanel->winId());
        }

        foreach (ExtensionContainer *it, _containers)
        {
            list.append(it->winId());
        }
    }

    QRect workArea;
    if (XineramaScreen == Plasma::XineramaAllScreens)
    {
         /* special value for all screens */
         workArea = Kicker::self()->kwinModule()->workArea(list);
    }
    else
    {
        workArea = Kicker::self()->kwinModule()->workArea(list, XineramaScreen)
                   .intersect(QApplication::desktop()->screenGeometry(XineramaScreen));
    }

    return workArea;
}

int ExtensionManager::nextPanelOrder()
{
    ++m_panelCounter;
    return m_panelCounter;
}

#include "extensionmanager.moc"
