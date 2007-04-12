/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#include <QFile>
#include <QTimer>
#include <Qt3Support/q3tl.h>
//Added by qt3to4:
#include <QMenu>

#include <kstaticdeleter.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klibloader.h>
#include <kpanelapplet.h>
#include <kpanelextension.h>
#include <kstandarddirs.h>

#include "appletinfo.h"
#include "container_applet.h"
#include "container_extension.h"
#include "panelextension.h"
#include "pluginmanager.h"
#include <kconfiggroup.h>

static KStaticDeleter<PluginManager> pluginManagerDeleter;
PluginManager* PluginManager::m_self = 0;

PluginManager* PluginManager::self()
{
    if (!m_self)
    {
        pluginManagerDeleter.setObject(m_self, new PluginManager());
    }

    return m_self;
}

AppletInfo::List PluginManager::applets(bool sort, AppletInfo::List* list)
{
    QStringList rel;
    KGlobal::dirs()->findAllResources("applets", "*.desktop", KStandardDirs::NoDuplicates, rel);
    return plugins(rel, AppletInfo::Applet, sort, list);
}

AppletInfo::List PluginManager::extensions(bool sort, AppletInfo::List* list)
{
    QStringList rel;
    KGlobal::dirs()->findAllResources("extensions", "*.desktop", KStandardDirs::NoDuplicates, rel);
    return plugins(rel, AppletInfo::Extension, sort, list);
}

AppletInfo::List PluginManager::builtinButtons(bool sort, AppletInfo::List* list)
{
    QStringList rel;
    KGlobal::dirs()->findAllResources("builtinbuttons", "*.desktop", KStandardDirs::NoDuplicates, rel);
    return plugins(rel, AppletInfo::BuiltinButton, sort, list);
}

AppletInfo::List PluginManager::specialButtons(bool sort, AppletInfo::List* list)
{
    QStringList rel;
    KGlobal::dirs()->findAllResources("specialbuttons", "*.desktop", KStandardDirs::NoDuplicates, rel);
    return plugins(rel, AppletInfo::SpecialButton, sort, list);
}

AppletInfo::List PluginManager::plugins(const QStringList& desktopFiles,
                                        AppletInfo::AppletType type,
                                        bool sort,
                                        AppletInfo::List* list)
{
    AppletInfo::List plugins;

    if (list)
    {
        plugins = *list;
    }

    for (QStringList::ConstIterator it = desktopFiles.constBegin();
         it != desktopFiles.constEnd(); ++it)
    {
        AppletInfo info(*it, QString::null, type);

        if (!info.isHidden())
        {
            plugins.append(info);
        }
    }

    if (sort)
    {
        qHeapSort(plugins.begin(), plugins.end());
    }

    return plugins;
}

PluginManager::PluginManager()
{
    KConfigGroup generalGroup(KGlobal::config(), "General");
    m_untrustedApplets = generalGroup.readEntry("UntrustedApplets", QStringList() );
    m_untrustedExtensions = generalGroup.readEntry("UntrustedExtensions", QStringList() );
}

PluginManager::~PluginManager()
{
    AppletInfo::Dict::const_iterator it = m_appletDict.constBegin();
    for (; it != m_appletDict.constEnd(); ++it)
    {
        disconnect(it.key(), SIGNAL(destroyed( QObject*)),
                   this, SLOT(slotPluginDestroyed(QObject*)));
        delete it.value();
    }

    // clear the untrusted lists
    clearUntrustedLists();
}

KPanelApplet* PluginManager::loadApplet(const AppletInfo& info,
                                        QWidget* parent )
{
    KLibLoader* loader = KLibLoader::self();
    KLibrary* lib = loader->library( QFile::encodeName(info.library()) );

    if (!lib)
    {
        kWarning() << "cannot open applet: " << info.library()
                    << " because of " << loader->lastErrorMessage() << endl;
        return 0;
    }

    KPanelApplet* (*init_ptr)(QWidget *, const QString&);
    init_ptr = (KPanelApplet* (*)(QWidget *, const QString&))lib->resolveFunction( "init" );

    if (!init_ptr)
    {
        kWarning() << info.library() << " is not a kicker extension!" << endl;
        loader->unloadLibrary( QFile::encodeName(info.library()) );
        return 0;
    }

    KPanelApplet* applet = init_ptr( parent, info.configFile() );

    if (applet)
    {
        m_appletDict.insert( applet, new AppletInfo( info ) );
        connect( applet, SIGNAL( destroyed( QObject* ) ),
                 SLOT( slotPluginDestroyed( QObject* ) ) );
    }

    return applet;
}

KPanelExtension* PluginManager::loadExtension(const AppletInfo& info, QWidget* parent )
{
    if (info.library() == "childpanel_panelextension"
        /* KDE4? || info.library() == "panel" */)
    {
        return new PanelExtension(info.configFile(), parent);
    }

    KLibLoader* loader = KLibLoader::self();
    KLibrary* lib = loader->library( QFile::encodeName(info.library()) );

    if( !lib ) {
        kWarning() << "cannot open extension: " << info.library()
                    << " because of " << loader->lastErrorMessage() << endl;
        return 0;
    }

    KPanelExtension* (*init_ptr)(QWidget *, const QString&);
    init_ptr = (KPanelExtension* (*)(QWidget *, const QString&))lib->resolveFunction( "init" );

    if(!init_ptr){
        kWarning() << info.library() << " is not a kicker extension!" << endl;
        loader->unloadLibrary( QFile::encodeName(info.library()) );
        return 0;
    }

    KPanelExtension* extension = init_ptr( parent, info.configFile() );

    if( extension ) {
        m_appletDict.insert( extension, new AppletInfo( info ) );
        connect( extension, SIGNAL( destroyed( QObject* ) ),
                 SLOT( slotPluginDestroyed( QObject* ) ) );
    }

    return extension;
}

bool PluginManager::hasInstance( const AppletInfo& info ) const
{
    AppletInfo::Dict::const_iterator it = m_appletDict.constBegin();
    for (; it != m_appletDict.constEnd(); ++it)
    {
        if (it.value()->library() == info.library())
        {
            return true;
        }
    }

    return false;
}

void PluginManager::slotPluginDestroyed(QObject* object)
{
    AppletInfo* info = 0;
    AppletInfo::Dict::iterator it = m_appletDict.begin();
    for (; it != m_appletDict.end(); ++it)
    {
        if (it.key() == object)
        {
            info = dynamic_cast<AppletInfo*>(it.value());
            m_appletDict.erase(it);
            break;
        }
    }

    if (!info)
    {
        return;
    }

    LibUnloader::unload(info->library());
    delete info;

    emit pluginDestroyed();
}

AppletContainer* PluginManager::createAppletContainer(
        const QString& desktopFile,
        bool isStartup,
        const QString& configFile,
        QMenu* opMenu,
        QWidget* parent,
        bool isImmutable)
{
    QString desktopPath = KGlobal::dirs()->findResource( "applets", desktopFile );

    if (desktopPath.isEmpty())
    {
        return 0;
    }

    AppletInfo info( desktopPath, configFile, AppletInfo::Applet );

    bool instanceFound = hasInstance(info);
    if (info.isUniqueApplet() && instanceFound)
    {
        return 0;
    }

    bool untrusted = m_untrustedApplets.indexOf(desktopFile) != -1;
    if (isStartup && untrusted)
    {
        // don't load extensions that bombed on us previously!
        return 0;
    }
    else if (!isStartup && !instanceFound && !untrusted)
    {
        // we haven't loaded this puppy before and we're not in the untrusted list
        m_untrustedApplets.append(desktopFile);
        KConfigGroup generalGroup(KGlobal::config(), "General");
        generalGroup.writeEntry("UntrustedApplets", m_untrustedApplets);
        generalGroup.sync();
    }

    AppletContainer* container = new AppletContainer(info, opMenu, isImmutable, parent);

    if (!container->isValid())
    {
        delete container;
        return 0;
    }

    return container;
}

ExtensionContainer* PluginManager::createExtensionContainer(const QString& desktopFile,
                                                            bool isStartup,
                                                            const QString& configFile,
                                                            const QString& extensionId)
{
    if (desktopFile.isEmpty())
    {
        return 0;
    }

    QString desktopPath = KGlobal::dirs()->findResource("extensions", desktopFile);
    if (desktopPath.isEmpty())
    {
        return 0;
    }

    AppletInfo info(desktopPath, configFile, AppletInfo::Extension);

    bool internal = (info.library() == "childpanel_panelextension");
    bool instance = !internal && hasInstance(info);
    if (instance && info.isUniqueApplet())
    {
        return 0;
    }

    if (!internal)
    {
        bool untrusted = m_untrustedExtensions.indexOf(desktopFile) != -1;
        if (isStartup && untrusted)
        {
            // don't load extensions that bombed on us previously!
            return 0;
        }
        else if (!isStartup && !instance && !untrusted)
        {
            // we don't have an instance around and we're not in the untrusted list!
            m_untrustedExtensions.append(desktopFile);
            KConfigGroup generalGroup(KGlobal::config(), "General");
            generalGroup.writeEntry("UntrustedExtensions", m_untrustedExtensions);
            generalGroup.sync();
        }
    }

    return new ExtensionContainer(info, extensionId);
}

void PluginManager::clearUntrustedLists()
{
    m_untrustedExtensions.clear();
    m_untrustedApplets.clear();

    KConfigGroup generalGroup(KGlobal::config(), "General");
    generalGroup.writeEntry("UntrustedApplets", m_untrustedApplets);
    generalGroup.writeEntry("UntrustedExtensions", m_untrustedExtensions);
    generalGroup.sync();
}

LibUnloader::LibUnloader( const QString &libName, QObject *parent )
    : QObject( parent ), _libName( libName )
{
    // NOTE: this doesn't work on kicker shutdown because the timer never gets
    //       fired.
    QTimer::singleShot( 0, this, SLOT( unload() ) );
}

void LibUnloader::unload( const QString &libName )
{
    (void)new LibUnloader( libName, kapp );
}

void LibUnloader::unload()
{
    KLibLoader::self()->unloadLibrary( QFile::encodeName( _libName ) );
    deleteLater();
}

#include "pluginmanager.moc"

