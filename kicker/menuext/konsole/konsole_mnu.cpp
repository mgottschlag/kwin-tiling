/*****************************************************************

Copyright (c) 1996-2001 the kicker authors. See file AUTHORS.

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

#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <QDir>
#include <QFileInfo>

#include <kapplication.h>
#include <kglobal.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kio/global.h>
#include <klocale.h>
#include <krun.h>
#include <kshell.h>
#include <kconfig.h>
#include <kdesktopfile.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>
#include <kworkspace.h>
#include "konsole_mnu.h"

extern "C"
{
    KDE_EXPORT void* init_kickermenu_konsole()
    {
	KGlobal::locale()->insertCatalog("libkickermenu_konsole");
        return new KonsoleMenuFactory;
    }
}

KonsoleMenu::KonsoleMenu(QWidget *parent)
    : KPanelMenu("", parent),
      m_profileMenu(0),
      m_bookmarksSession(0),
      m_bookmarkHandlerSession(0)
{
}

KonsoleMenu::~KonsoleMenu()
{
    KGlobal::locale()->removeCatalog("libkickermenu_konsole");
}

static void insertItemSorted(KMenu *menu,
                             const QIcon &iconSet,
                             const QString &txt, int id)
{
  const int defaultId = 1; // The id of the 'new' item.
  int index = menu->indexOf(defaultId);
  int count = menu->actions().count();
  if (index >= 0)
  {
     index++; // Skip separator
     while(true)
     {
        index++;
        if (index >= count)
        {
           index = -1; // Insert at end
           break;
        }
        if (menu->text(menu->idAt(index)) > txt)
           break; // Insert before this item
     }
  }
  menu->insertItem(iconSet, txt, id, index);
}


void KonsoleMenu::initialize()
{
    if (initialized())
    {
        clear();
    }

    setInitialized(true);

    QStringList list = KGlobal::dirs()->findAllResources("data",
                                                         "konsole/*.desktop",
                                                         KStandardDirs::NoDuplicates);

    QString defaultShell = KStandardDirs::locate("data", "konsole/shell.desktop");
    list.prepend(defaultShell);

    int id = 1;

    sessionList.clear();
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        if ((*it == defaultShell) && (id != 1))
        {
           continue;
        }

        KDesktopFile conf(*it);
        const KConfigGroup cg = conf.desktopGroup();
        QString text = cg.readEntry("Name");

        // try to locate the binary
        QString exec= cg.readPathEntry("Exec");
        if (exec.startsWith("su -c \'"))
        {
             exec = exec.mid(7,exec.length()-8);
        }

        exec = KRun::binaryName(exec, false);
        exec = KShell::tildeExpand(exec);
        QString pexec = KGlobal::dirs()->findExe(exec);
        if (text.isEmpty() ||
            cg.readEntry("Type") != "KonsoleApplication" ||
            (!exec.isEmpty() && pexec.isEmpty()))
        {
            continue;
        }
        insertItemSorted(this, KIcon(cg.readEntry("Icon", "konsole")),
                                            text, id++);
        QFileInfo fi(*it);
        sessionList.append(fi.completeBaseName());

        if (id == 2)
        {
           addSeparator();
        }
    }

    m_bookmarkHandlerSession = new KonsoleBookmarkHandler(this, false);
    m_bookmarksSession = m_bookmarkHandlerSession->menu();
    addSeparator();
    insertItem(KIcon("keditbookmarks"),
               i18n("New Session at Bookmark"), m_bookmarksSession);
    connect(m_bookmarkHandlerSession,
            SIGNAL(openUrl(const QString&, const QString&)),
            SLOT(newSession(const QString&, const QString&)));


    screenList.clear();
    QByteArray screenDir = getenv("SCREENDIR");

    if (screenDir.isEmpty())
    {
        screenDir = QFile::encodeName(QDir::homePath()) + "/.screen/";
    }

    QStringList sessions;
    // Can't use QDir as it doesn't support FIFOs :(
    DIR *dir = opendir(screenDir);
    if (dir)
    {
        struct dirent *entry;
        while ((entry = readdir(dir)))
        {
            QByteArray path = screenDir + '/' + QByteArray(entry->d_name);
            struct stat st;
            if (stat(path, &st) != 0)
            {
                continue;
            }

            int fd;
            if (S_ISFIFO(st.st_mode) && !(st.st_mode & 0111) && // xbit == attached
                (fd = open(path, O_WRONLY | O_NONBLOCK)) != -1)
            {
                ::close(fd);
                screenList.append(QFile::decodeName(entry->d_name));
                insertItem(KIcon("konsole"),
                           i18nc("Screen is a program for controlling screens",
                                "Screen at %1", entry->d_name), id);
                id++;
            }
        }
        closedir(dir);
    }

    // reset id as we are now going to populate the profiles submenu
    id = 0;

    delete m_profileMenu;
    m_profileMenu = new KMenu(this);
    QStringList profiles = KGlobal::dirs()->findAllResources("data",
                                                             "konsole/profiles/*",
                                                             KStandardDirs::NoDuplicates);
    m_profiles.resize(profiles.count());
    QStringList::ConstIterator pEnd = profiles.end();
    for (QStringList::ConstIterator pIt = profiles.begin(); pIt != pEnd; ++pIt)
    {
        QFileInfo info(*pIt);
        QString profileName = KIO::decodeFileName(info.baseName());
        QString niceName = profileName;
        KConfig cfg(*pIt, KConfig::OnlyLocal);
        if (cfg.hasGroup("Profile"))
        {
            const KConfigGroup cg( &cfg, "Profile" );
            if (cg.hasKey("Name"))
            {
                niceName = cg.readEntry("Name");
            }
        }

        m_profiles[id] = profileName;
        ++id;
        m_profileMenu->insertItem(niceName, id);
    }

    int profileID = insertItem(i18n("New Session Using Profile"),
                               m_profileMenu);
    if (id == 1)
    {
        // we don't have any profiles, disable the menu
        setItemEnabled(profileID, false);
    }
    connect(m_profileMenu, SIGNAL(activated(int)), SLOT(launchProfile(int)));

    addSeparator();
    insertItem(KIcon("reload"),
               i18n("Reload Sessions"), this, SLOT(reinitialize()));
}

void KonsoleMenu::slotExec(int id)
{
    if (id < 1)
    {
        return;
    }

    --id;
    KWorkSpace::propagateSessionManager();
    QStringList args;
    if (id < sessionList.count())
    {
        args << "--type";
        args << sessionList[id];
    }
    else
    {
        args << "-e";
        args << "screen";
        args << "-r";
        args << screenList[id - sessionList.count()];
    }
    KToolInvocation::kdeinitExec("konsole", args);
    return;
}

void KonsoleMenu::launchProfile(int id)
{
    if (id < 1 || id > m_profiles.count())
    {
        return;
    }

    --id;
    // this is a session, not a bookmark, so execute that instead
   QStringList args;
   args << "--profile" << m_profiles[id];
   KToolInvocation::kdeinitExec("konsole", args);
}

KUrl KonsoleMenu::baseURL() const
{
    KUrl url;
    /*url.setPath(se->getCwd()+'/');*/
    return url;
}

void KonsoleMenu::newSession(const QString& sURL, const QString& title)
{
    QStringList args;

    KUrl url = KUrl(sURL);
    if ((url.protocol() == "file") && (url.hasPath()))
    {
        args << "-T" << title;
        args << "--workdir" << url.path();
        KToolInvocation::kdeinitExec("konsole", args);
        return;
    }
    else if ((!url.protocol().isEmpty()) && (url.hasHost()))
    {
        QString protocol = url.protocol();
        QString host = url.host();
        args << "-T" << title;
        args << "-e" << protocol.toLatin1(); /* argv[0] == command to run. */
        if (url.hasUser()) {
            args << "-l" << url.user().toLatin1();
        }
        args << host.toLatin1();
        KToolInvocation::kdeinitExec("konsole", args);
        return;
    }
    /*
     * We can't create a session without a protocol.
     * We should ideally popup a warning, about an invalid bookmark.
     */
}


//*****************************************************************

KonsoleMenuFactory::KonsoleMenuFactory(QObject *parent, const char *name)
: KLibFactory(parent)
{
    setObjectName(name);
    KIconLoader::global()->addAppDir("konsole");
    KGlobal::locale()->insertCatalog("konsolemenuapplet");
}

QObject* KonsoleMenuFactory::createObject(QObject *parent, const char*, const QStringList&)
{
    return new KonsoleMenu((QWidget*)parent);
}

#include "konsole_mnu.moc"
