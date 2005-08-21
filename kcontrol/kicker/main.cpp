/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#include <qcombobox.h>

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmodulecontainer.h>
#include <kdirwatch.h>
#include <kimageio.h>
#include <klistview.h>
#include <kstaticdeleter.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "hidingconfig.h"
#include "kickerSettings.h"
#include "lookandfeelconfig.h"
#include "menuconfig.h"
#include "positionconfig.h"

#include "main.h"
#include "main.moc"

#include <X11/Xlib.h>
#include <QX11Info>

KickerConfig *KickerConfig::m_self = 0;
static KStaticDeleter<KickerConfig> staticKickerConfigDeleter;

KickerConfig *KickerConfig::self()
{
    if (!m_self)
    {
        staticKickerConfigDeleter.setObject(m_self, new KickerConfig());
    }
    return m_self;
}

KickerConfig::KickerConfig(QWidget *parent, const char *name)
  : QObject(parent, name),
    DCOPObject("KickerConfig"),
    configFileWatch(new KDirWatch(this)),
    m_currentPanelIndex(0)
{
    m_screenNumber = QX11Info::display() ? DefaultScreen(QX11Info::display()) : 0;

    KickerSettings::instance(configName().latin1());

    init();

    kapp->dcopClient()->setNotifications(true);
    connectDCOPSignal("kicker", "kicker", "configSwitchToPanel(QString)",
                      "jumpToPanel(QString)", false);
    kapp->dcopClient()->send("kicker", "kicker", "configLaunched()", QByteArray());

    connect(this, SIGNAL(hidingPanelChanged(int)),
            this, SLOT(setCurrentPanelIndex(int)));
    connect(this, SIGNAL(positionPanelChanged(int)),
            this, SLOT(setCurrentPanelIndex(int)));
}

KickerConfig::~KickerConfig()
{
    // QValueList::setAutoDelete where for art thou?
    ExtensionInfoList::iterator it = m_extensionInfo.begin();
    while (it != m_extensionInfo.end())
    {
        ExtensionInfo* info = *it;
        it = m_extensionInfo.erase(it);
        delete info;
    }
}

// TODO: This is not true anymore:
// this method may get called multiple times during the life of the control panel!
void KickerConfig::init()
{
    disconnect(configFileWatch, SIGNAL(dirty(const QString&)), this, SLOT(configChanged(const QString&)));
    configFileWatch->stopScan();
    for (ExtensionInfoList::iterator it = m_extensionInfo.begin();
         it != m_extensionInfo.end();
         ++it)
    {
        configFileWatch->removeFile((*it)->_configPath);
    }

    QString configname = configName();
    QString configpath = KGlobal::dirs()->findResource("config", configname);
    if (configpath.isEmpty())
       configpath = locateLocal("config", configname);
    KSharedConfig::Ptr config = KSharedConfig::openConfig(configname);

    if (m_extensionInfo.isEmpty())
    {
        // our list is empty, so add the main kicker config
        m_extensionInfo.append(new ExtensionInfo(QString::null, configname, configpath));
        configFileWatch->addFile(configpath);
    }
    else
    {
        // this isn't our first trip through here, which means we are reloading
        // so reload the kicker config (first we have to find it ;)
        ExtensionInfoList::iterator it = m_extensionInfo.begin();
        for (; it != m_extensionInfo.end(); ++it)
        {
            if (configpath == (*it)->_configPath)
            {
                (*it)->load();
                break;
            }
        }
    }

    setupExtensionInfo(*config, true, true);

    connect(configFileWatch, SIGNAL(dirty(const QString&)), this, SLOT(configChanged(const QString&)));
    configFileWatch->startScan();
}

void KickerConfig::notifyKicker()
{
    kdDebug() << "KickerConfig::notifyKicker()" << endl;

    emit aboutToNotifyKicker();

    // Tell kicker about the new config file.
    if (!kapp->dcopClient()->isAttached())
    {
        kapp->dcopClient()->attach();
    }

    QByteArray data;
    QCString appname;

    if (m_screenNumber == 0)
    {
        appname = "kicker";
    }
    else
    {
        appname.sprintf("kicker-screen-%d", m_screenNumber);
    }

    kapp->dcopClient()->send(appname, "kicker", "configure()", data);
}

void KickerConfig::setupExtensionInfo(KConfig& config, bool checkExists, bool reloadIfExists)
{
    config.setGroup("General");
    QStringList elist = config.readListEntry("Extensions2");

    // all of our existing extensions
    // we'll remove ones we find which are still there the oldExtensions, and delete
    // all the extensions that remain (e.g. are no longer active)
    ExtensionInfoList oldExtensions(m_extensionInfo);

    for (QStringList::Iterator it = elist.begin(); it != elist.end(); ++it)
    {
        // extension id
        QString group(*it);

        // is there a config group for this extension?
        if (!config.hasGroup(group) || group.contains("Extension") < 1)
        {
            continue;
        }

        // set config group
        config.setGroup(group);

        QString df = KGlobal::dirs()->findResource("extensions", config.readEntry("DesktopFile"));
        QString configname = config.readEntry("ConfigFile");
        QString configpath = KGlobal::dirs()->findResource("config", configname);

        if (checkExists)
        {
            ExtensionInfoList::iterator extIt = m_extensionInfo.begin();
            for (; extIt != m_extensionInfo.end(); ++extIt)
            {
                if (configpath == (*extIt)->_configPath)
                {
                    // we have found it in the config file and it exists
                    // so remove it from our list of existing extensions
                    oldExtensions.remove(*extIt);
                    if (reloadIfExists)
                    {
                        (*extIt)->load();
                    }
                    break;
                }
            }

            if (extIt != m_extensionInfo.end())
            {
                continue;
            }
        }

        configFileWatch->addFile(configpath);
        ExtensionInfo* info = new ExtensionInfo(df, configname, configpath);
        m_extensionInfo.append(info);
        emit extensionAdded(info);
    }

    if (checkExists)
    {
        // now remove all the left overs that weren't in the file
        ExtensionInfoList::iterator extIt = oldExtensions.begin();
        for (; extIt != oldExtensions.end(); ++extIt)
        {
            // don't remove the kickerrc!
            if (!(*extIt)->_configPath.endsWith(configName()))
            {
                emit extensionRemoved(*extIt);
                m_extensionInfo.remove(*extIt);
            }
        }
    }
}

void KickerConfig::configChanged(const QString& configPath)
{
    if (configPath.endsWith(configName()))
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig(configName());
        config->reparseConfiguration();
        setupExtensionInfo(*config, true);
    }

    // find the extension and change it
    for (ExtensionInfoList::iterator it = m_extensionInfo.begin(); it != m_extensionInfo.end(); ++it)
    {
        if (configPath == (*it)->_configPath)
        {
            emit extensionAboutToChange(configPath);
            (*it)->configChanged();
            break;
        }
    }

    emit extensionChanged(configPath);
}

void KickerConfig::populateExtensionInfoList(QComboBox* list)
{
    list->clear();
    for (ExtensionInfoList::iterator it = m_extensionInfo.begin(); it != m_extensionInfo.end(); ++it)
    {
       list->insertItem((*it)->_name);
    }
}

const ExtensionInfoList& KickerConfig::extensionsInfo()
{
    return m_extensionInfo;
}

void KickerConfig::reloadExtensionInfo()
{
    for (ExtensionInfoList::iterator it = m_extensionInfo.begin(); it != m_extensionInfo.end(); ++it)
    {
       (*it)->load();
    }

    emit extensionInfoChanged();
}

void KickerConfig::saveExtentionInfo()
{
    for (ExtensionInfoList::iterator it = m_extensionInfo.begin(); it != m_extensionInfo.end(); ++it)
    {
       (*it)->save();
    }
}

void KickerConfig::jumpToPanel(const QString& panelConfig)
{
    ExtensionInfoList::iterator it = m_extensionInfo.begin();
    int index = 0;
    for (; it != m_extensionInfo.end(); ++it, ++index)
    {
        if ((*it)->_configFile == panelConfig)
        {
            break;
        }
    }

    if (it == m_extensionInfo.end())
    {
        return;
    }

    kdDebug() << "KickerConfig::jumpToPanel: index=" << index << endl;

    emit hidingPanelChanged(index);
    emit positionPanelChanged(index);
}

QString KickerConfig::configName()
{
    if (m_screenNumber == 0)
    {
        return "kickerrc";
    }
    else
    {
        return QString("kicker-screen-%1rc").arg(m_screenNumber);
    }
}

void KickerConfig::setCurrentPanelIndex(int index)
{
    m_currentPanelIndex = index;
}

QString KickerConfig::quickHelp() const
{
    return i18n("<h1>Panel</h1> Here you can configure the KDE panel (also"
                " referred to as 'kicker'). This includes options like the position and"
                " size of the panel, as well as its hiding behavior and its looks.<p>"
                " Note that you can also access some of these options directly by clicking"
                " on the panel, e.g. dragging it with the left mouse button or using the"
                " context menu on right mouse button click. This context menu also offers you"
                " manipulation of the panel's buttons and applets.");
}

KAboutData *KickerConfig::aboutData()
{
    // the KAboutDatas are deleted by the KCModules
    KAboutData *about
          = new KAboutData(I18N_NOOP("kcmkicker"),
                           I18N_NOOP("KDE Panel Control Module"),
                           0, 0, KAboutData::License_GPL,
                           I18N_NOOP("(c) 1999 - 2001 Matthias Elter\n"
                                     "(c) 2002 - 2003 Aaron J. Seigo"));

    about->addAuthor("Aaron J. Seigo", 0, "aseigo@kde.org");
    about->addAuthor("Matthias Elter", 0, "elter@kde.org");

    return about;
}

extern "C"
{
    KDE_EXPORT KCModule *create_kicker(QWidget *parent, const char *name)
    {
        KCModuleContainer *container = new KCModuleContainer(parent, name);
        container->addModule("kicker_config_arrangement");
        container->addModule("kicker_config_hiding");
        container->addModule("kicker_config_menus");
        container->addModule("kicker_config_appearance");
        return container;
    }

    KDE_EXPORT KCModule *create_kicker_arrangement(QWidget *parent, const char * /*name*/)
    {
        KGlobal::dirs()->addResourceType("extensions", KStandardDirs::kde_default("data") +
                                         "kicker/extensions");
        return new PositionConfig(parent, "kcmkickerarrangement");
    }

    KDE_EXPORT KCModule *create_kicker_hiding(QWidget *parent, const char * /*name*/)
    {
        KGlobal::dirs()->addResourceType("extensions", KStandardDirs::kde_default("data") +
                                         "kicker/extensions");
        return new HidingConfig(parent, "kcmkickerhiding");
    }

    KDE_EXPORT KCModule *create_kicker_menus(QWidget *parent, const char * /*name*/)
    {
        return new MenuConfig(parent, "kcmkickermenus");
    }

    KDE_EXPORT KCModule *create_kicker_appearance(QWidget *parent, const char * /*name*/)
    {
        KImageIO::registerFormats();
        KGlobal::dirs()->addResourceType("tiles", KStandardDirs::kde_default("data") +
                "kicker/tiles");
        KGlobal::dirs()->addResourceType("hb_pics", KStandardDirs::kde_default("data") +
                "kcmkicker/pics");
        return new LookAndFeelConfig(parent, "kcmkickerappearance");
    }
}
