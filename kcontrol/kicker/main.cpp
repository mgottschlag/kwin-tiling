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

#include <qtabwidget.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kdirwatch.h>
#include <kstandarddirs.h>
#include <kimageio.h>
#include <klistview.h>

#include <dcopclient.h>

#include "main.h"
#include "main.moc"
#include "positiontab_impl.h"
#include "hidingtab_impl.h"
#include "menutab_impl.h"
#include "lookandfeeltab_impl.h"


#include <X11/Xlib.h>
#include <kaboutdata.h>

// for multihead
int KickerConfig::kickerconfig_screen_number = 0;

KickerConfig::KickerConfig(QWidget *parent, const char *name)
  : KCModule(parent, name),
    DCOPObject("KickerConfig"),
    configFileWatch(new KDirWatch(this))
{
    m_extensionInfo.setAutoDelete(true);

    initScreenNumber();

    QVBoxLayout *layout = new QVBoxLayout(this);
    QTabWidget *tab = new QTabWidget(this);
    layout->addWidget(tab);

    positiontab = new PositionTab(this);
    tab->addTab(positiontab, i18n("Arran&gement"));
    connect(positiontab, SIGNAL(changed()), this, SLOT(configChanged()));

    hidingtab = new HidingTab(this);
    tab->addTab(hidingtab, i18n("H&iding"));
    connect(hidingtab, SIGNAL(changed()), this, SLOT(configChanged()));

    menutab = new MenuTab(this);
    tab->addTab(menutab, i18n("&Menus"));
    connect(menutab, SIGNAL(changed()), this, SLOT(configChanged()));

    lookandfeeltab = new LookAndFeelTab(this);
    tab->addTab(lookandfeeltab, i18n("&Appearance"));
    connect(lookandfeeltab, SIGNAL(changed()), this, SLOT(configChanged()));

    load();

    connect(positiontab->m_panelList, SIGNAL(selectionChanged(QListViewItem*)),
            this, SLOT(positionPanelChanged(QListViewItem*)));
    connect(hidingtab->m_panelList, SIGNAL(selectionChanged(QListViewItem*)),
            this, SLOT(hidingPanelChanged(QListViewItem*)));
    connect(positiontab, SIGNAL(panelPositionChanged(int)),
            hidingtab, SLOT(panelPositionChanged(int)));

    kapp->dcopClient()->setNotifications(true);
    connectDCOPSignal("kicker", "kicker", "configSwitchToPanel(QString)", "jumpToPanel(QString)", false);
    kapp->dcopClient()->send("kicker", "kicker", "configLaunched()", QByteArray());
}

void KickerConfig::initScreenNumber()
{
    if (qt_xdisplay())
        kickerconfig_screen_number = DefaultScreen(qt_xdisplay());
}

void KickerConfig::configChanged()
{
    emit changed(true);
}

// this method may get called multiple times during the life of the control panel!
void KickerConfig::load()
{
    disconnect(configFileWatch, SIGNAL(dirty(const QString&)), this, SLOT(configChanged(const QString&)));
    configFileWatch->stopScan();
    for (extensionInfoList::iterator it = m_extensionInfo.begin();
         it != m_extensionInfo.end();
         ++it)
    {
        configFileWatch->removeFile((*it)->_configPath);
    }

    QString configname = configName();
    QString configpath = KGlobal::dirs()->findResource("config", configname);
    KSharedConfig::Ptr c = KSharedConfig::openConfig(configname);

    if (m_extensionInfo.isEmpty())
    {
        // our list is empty, so add the main kicker config
        m_extensionInfo.append(new extensionInfo(QString::null, configname, configpath));
        configFileWatch->addFile(configpath);
    }
    else
    {
        // this isn't our first trip through here, which means we are reloading
        // so reload the kicker config (first we have to find it ;)
        extensionInfoList::iterator it = m_extensionInfo.begin();
        for (; it != m_extensionInfo.end(); ++it)
        {
            if (configpath == (*it)->_configPath)
            {
                (*it)->load();
                break;
            }
        }
    }

    setupExtensionInfo(*c, true, true);

    positiontab->load();
    hidingtab->load();
    menutab->load();
    lookandfeeltab->load();

    emit changed(false);
    connect(configFileWatch, SIGNAL(dirty(const QString&)), this, SLOT(configChanged(const QString&)));
    configFileWatch->startScan();
}

void KickerConfig::save()
{
    positiontab->save();
    hidingtab->save();
    menutab->save();
    lookandfeeltab->save();

    emit changed(false);

    notifyKicker();
}

void KickerConfig::notifyKicker()
{
    // Tell kicker about the new config file.
    if (!kapp->dcopClient()->isAttached())
        kapp->dcopClient()->attach();
    QByteArray data;

    QCString appname;
    if (kickerconfig_screen_number == 0)
        appname = "kicker";
    else
        appname.sprintf("kicker-screen-%d", kickerconfig_screen_number);
    kapp->dcopClient()->send( appname, "kicker", "configure()", data );
}

void KickerConfig::defaults()
{
    positiontab->defaults();
    hidingtab->defaults();
    menutab->defaults();
    lookandfeeltab->defaults();

    emit changed(true);
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

const KAboutData* KickerConfig::aboutData() const
{

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmkicker"), I18N_NOOP("KDE Panel Control Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 1999 - 2001 Matthias Elter\n(c) 2002 - 2003 Aaron J. Seigo"));

    about->addAuthor("Aaron J. Seigo", 0, "aseigo@kde.org");
    about->addAuthor("Matthias Elter", 0, "elter@kde.org");

    return about;
}

void KickerConfig::setupExtensionInfo(KConfig& c, bool checkExists, bool reloadIfExists)
{
    c.setGroup("General");
    QStringList elist = c.readListEntry("Extensions2");

    // all of our existing extensions
    // we'll remove ones we find which are still there the oldExtensions, and delete
    // all the extensions that remain (e.g. are no longer active)
    extensionInfoList oldExtensions(m_extensionInfo);

    for (QStringList::Iterator it = elist.begin(); it != elist.end(); ++it)
    {
        // extension id
        QString group(*it);

        // is there a config group for this extension?
        if(!c.hasGroup(group) ||
           group.contains("Extension") < 1)
        {
            continue;
        }

        // set config group
        c.setGroup(group);

        QString df = KGlobal::dirs()->findResource("extensions", c.readEntry("DesktopFile"));
        QString configname = c.readEntry("ConfigFile");
        QString configpath = KGlobal::dirs()->findResource("config", configname);

        if (checkExists)
        {
            extensionInfoList::iterator extIt = m_extensionInfo.begin();
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
        extensionInfo* info = new extensionInfo(df, configname, configpath);
        m_extensionInfo.append(info);
        emit extensionAdded(info);
    }

    if (checkExists)
    {
        // now remove all the left overs that weren't in the file
        extensionInfoList::iterator extIt = oldExtensions.begin();
        for (; extIt != oldExtensions.end(); ++extIt)
        {
            // don't remove the kickerrc!
            if ((*extIt)->_configPath.right(8) != "kickerrc")
            {
                hidingtab->removeExtension(*extIt);
                positiontab->removeExtension(*extIt);
                m_extensionInfo.remove(*extIt);
            }
        }
    }
}

void KickerConfig::configChanged(const QString& config)
{
    if (config.right(8) == "kickerrc")
    {
        KSharedConfig::Ptr c = KSharedConfig::openConfig(configName());
        setupExtensionInfo(*c, true);
    }

    // find the extension and change it
    for (QPtrListIterator<extensionInfo> it(m_extensionInfo); it; ++it)
    {
        if (config == (*it)->_configPath)
        {
            emit extensionAboutToChange(config);
            (*it)->configChanged();
            break;
        }
    }

    emit extensionChanged(config);
}

void KickerConfig::populateExtensionInfoList(QListView* list)
{
    extensionInfoItem* last(0);
    for (QPtrListIterator<extensionInfo> it(m_extensionInfo); it; ++it)
    {
       last = new extensionInfoItem(*it, list, last);
    }
}

const extensionInfoList& KickerConfig::extensionsInfo()
{
    return m_extensionInfo;
}

void KickerConfig::reloadExtensionInfo()
{
    for (QPtrListIterator<extensionInfo> it(m_extensionInfo); it; ++it)
    {
       (*it)->load();
    }

    emit extensionInfoChanged();
}

void KickerConfig::saveExtentionInfo()
{
    for (QPtrListIterator<extensionInfo> it(m_extensionInfo); it; ++it)
    {
       (*it)->save();
    }
}

// when someone clicks on a panel in the position panel, make it
// switch the selected panel on the hiding panel too
void KickerConfig::positionPanelChanged(QListViewItem* item)
{
    if (!item)
    {
        return;
    }

    extensionInfo* info = static_cast<extensionInfoItem*>(item)->info();
    extensionInfoItem* hidingItem =
        static_cast<extensionInfoItem*>(hidingtab->m_panelList->firstChild());

    while (hidingItem)
    {
        if (hidingItem->info() == info)
        {
            hidingtab->m_panelList->setSelected(hidingItem, true);
            return;
        }

        hidingItem = static_cast<extensionInfoItem*>(hidingItem->nextSibling());
    }
}


// when someone clicks on a panel in the hiding panel, make it
// switch the selected panel on the position panel too
void KickerConfig::hidingPanelChanged(QListViewItem* item)
{
    if (!item)
    {
        return;
    }

    extensionInfo* info = static_cast<extensionInfoItem*>(item)->info();
    extensionInfoItem* positionItem =
        static_cast<extensionInfoItem*>(positiontab->m_panelList->firstChild());

    while (positionItem)
    {
        if (positionItem->info() == info)
        {
            positiontab->m_panelList->setSelected(positionItem, true);
            return;
        }

        positionItem = static_cast<extensionInfoItem*>(positionItem->nextSibling());
    }
}

void KickerConfig::jumpToPanel(const QString& panelConfig)
{
    extensionInfoList::iterator it = m_extensionInfo.begin();
    for (; it != m_extensionInfo.end(); ++it)
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

    if (positiontab)
    {
        extensionInfoItem* positionItem =
            static_cast<extensionInfoItem*>(positiontab->m_panelList->firstChild());

        while (positionItem)
        {
            if (positionItem->info() == (*it))
            {
                positiontab->m_panelList->setSelected(positionItem, true);
                return;
            }

            positionItem = static_cast<extensionInfoItem*>(positionItem->nextSibling());
        }
    }

    if (hidingtab)
    {
        extensionInfoItem* hidingItem =
            static_cast<extensionInfoItem*>(hidingtab->m_panelList->firstChild());

        while (hidingItem)
        {
            if (hidingItem->info() == (*it))
            {
                hidingtab->m_panelList->setSelected(hidingItem, true);
                return;
            }

            hidingItem = static_cast<extensionInfoItem*>(hidingItem->nextSibling());
        }
    }
}

QString KickerConfig::configName()
{
    if (kickerconfig_screen_number == 0)
        return "kickerrc";
    else
        return QString("kicker-screen-%1rc").arg(kickerconfig_screen_number);
}

extern "C"
{
    KCModule *create_kicker(QWidget *parent, const char *)
    {
        KImageIO::registerFormats();
        KGlobal::dirs()->addResourceType("applets", KStandardDirs::kde_default("data") +
                                         "kicker/applets");
        KGlobal::dirs()->addResourceType("extensions", KStandardDirs::kde_default("data") +
                                         "kicker/extensions");
        KGlobal::dirs()->addResourceType("tiles", KStandardDirs::kde_default("data") +
                                         "kicker/tiles");
        KGlobal::dirs()->addResourceType("hb_pics", KStandardDirs::kde_default("data") +
                                         "kcmkicker/pics");
        return new KickerConfig(parent, "kcmkicker");
    }
}
