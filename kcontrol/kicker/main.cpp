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
#include <qradiobutton.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdirwatch.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kimageio.h>

#include <dcopclient.h>

#include "main.h"
#include "main.moc"
#include "positiontab_impl.h"
#include "hidingtab_impl.h"
#include "menutab_impl.h"
//#include "lookandfeeltab_impl.h"
//#include "applettab_impl.h"

#include "lookandfeeltab_kcm.h"

#include <X11/Xlib.h>
#include <kaboutdata.h>

// for multihead
int KickerConfig::kickerconfig_screen_number = 0;

KickerConfig::KickerConfig(QWidget *parent, const char *name)
  : KCModule(parent, name),
    configFileWatch(new KDirWatch(this))
{
    m_extensionInfo.setAutoDelete(true);

    initScreenNumber();

    QString configname = configName();
    QString configpath = KGlobal::dirs()->findResource("config", configname);
    configFileWatch->addFile(configpath);
    m_extensionInfo.append(new extensionInfo(QString::null, configname, configpath));
    KConfig c(configname, false, false);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QTabWidget *tab = new QTabWidget(this);
    layout->addWidget(tab);

    setupExtensionInfo(c, false);

    positiontab = new PositionTab(this);
    tab->addTab(positiontab, i18n("Arran&gement"));
    connect(positiontab, SIGNAL(changed()), this, SLOT(configChanged()));

    hidingtab = new HidingTab(this);
    tab->addTab(hidingtab, i18n("&Hiding"));
    connect(hidingtab, SIGNAL(changed()), this, SLOT(configChanged()));

    menutab = new MenuTab(this);
    tab->addTab(menutab, i18n("&Menus"));
    connect(menutab, SIGNAL(changed()), this, SLOT(configChanged()));
    
//    lookandfeeltab = new LookAndFeelTab(this);
//    tab->addTab(lookandfeeltab, i18n("A&ppearance"));
//    connect(lookandfeeltab, SIGNAL(changed()), this, SLOT(configChanged()));

//    menutab = new MenuTab(this);
//    tab->addTab(menutab, i18n("&Menus"));
//    connect(menutab, SIGNAL(changed()), this, SLOT(configChanged()));

    load();

    QObject::connect(positiontab->m_panelList, SIGNAL(selectionChanged(QListViewItem*)),
                     this, SLOT(positionPanelChanged(QListViewItem*)));
    QObject::connect(hidingtab->m_panelList, SIGNAL(selectionChanged(QListViewItem*)),
                     this, SLOT(hidingPanelChanged(QListViewItem*)));

    //applettab = new AppletTab(this);
    //tab->addTab(applettab, i18n("&Applets"));
    //connect(applettab, SIGNAL(changed()), this, SLOT(configChanged()));
    connect(configFileWatch, SIGNAL(dirty(const QString&)), this, SLOT(configChanged(const QString&)));
    connect(configFileWatch, SIGNAL(deleted(const QString&)), this, SLOT(configRemoved(const QString&)));
    configFileWatch->startScan();
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

void KickerConfig::load()
{
    positiontab->load();
    hidingtab->load();
    menutab->load();
    //lookandfeeltab->load();
    //applettab->load();
    emit changed(false);
}

void KickerConfig::save()
{
    positiontab->save();
    hidingtab->save();
    menutab->save();
    //lookandfeeltab->save();
    //applettab->save();

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
    //lookandfeeltab->defaults();
    //applettab->defaults();

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
                  I18N_NOOP("(c) 1999 - 2001 Matthias Elter\n(c) 2002 Aaron J. Seigo"));

    about->addAuthor("Aaron J. Seigo", 0, "aseigo@olympusproject.org");
    about->addAuthor("Matthias Elter", 0, "elter@kde.org");

    return about;
}

void KickerConfig::setupExtensionInfo(KConfig& c, bool checkExists)
{
    c.setGroup("General");
    QStringList elist = c.readListEntry("Extensions2");
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
            QPtrListIterator<extensionInfo> extIt(m_extensionInfo);
            for (; extIt; ++extIt)
            {
                if (configpath == (*extIt)->_configPath)
                {
                    break;
                }
            }
            
            if (extIt)
            {
                continue;
            }
        }
        
        configFileWatch->addFile(configpath);
        extensionInfo* info = new extensionInfo(df, configname, configpath);
        m_extensionInfo.append(info);
        emit extensionAdded(info);
    }
}

void KickerConfig::configChanged(const QString& config)
{
    if (config.right(8) == "kickerrc")
    {
        KConfig c(configName(), false, false);
        setupExtensionInfo(c, true);
    }
        
    // find the extension and delete it
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

void KickerConfig::configRemoved(const QString& config)
{
    if (config.right(8) == "kickerrc")
    {
        // the main panel config has been removed???
        // ditch the extensions, i suppose - AJS
        for (QPtrListIterator<extensionInfo> it(m_extensionInfo); it; ++it)
        {
            if ((*it)->_configFile != config)
            {
                hidingtab->removeExtension(*it);         
                positiontab->removeExtension(*it);
                m_extensionInfo.remove(*it);
            }
        }
    }
    else
    {
        // find the extension and delete it
        for (QPtrListIterator<extensionInfo> it(m_extensionInfo); it; ++it)
        {
            if (config == (*it)->_configPath)
            {
                hidingtab->removeExtension(*it);               
                positiontab->removeExtension(*it);               
                m_extensionInfo.remove(*it);
                break;
            }
        }
    }
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
        return new KickerConfig(parent, "kcmkicker");
    };

    KCModule *create_kicker_behaviour(QWidget *parent, const char *)
    {
        KImageIO::registerFormats();
        KGlobal::dirs()->addResourceType("tiles", KStandardDirs::kde_default("data") +
                                         "kicker/tiles");
        KGlobal::dirs()->addResourceType("hb_pics", KStandardDirs::kde_default("data") +
                                         "kcmkicker/pics");
        return new LookAndFeelConfig(parent, "kcmkicker");
    };
}
