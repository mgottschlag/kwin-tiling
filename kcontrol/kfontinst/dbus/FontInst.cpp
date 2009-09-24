/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2009 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QtDBus/QDBusConnection>
#include <QtCore/QTimer>
#include <KDE/KDebug>
#include <PolicyKit/polkit-qt/Auth>
#include <kio/global.h>
#include <kde_file.h>
#include <fontconfig/fontconfig.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "FontInst.h"
#include "fontinstadaptor.h"
#include "Misc.h"
#include "Fc.h"
#include "WritingSystems.h"
#include "Utils.h"
#include "FontinstIface.h"

#define KFI_DBUG kDebug() << time(0L)

namespace KFI
{

static void decompose(const QString &name, QString &family, QString &style)
{
    int commaPos=name.lastIndexOf(',');

    family=-1==commaPos ? name : name.left(commaPos);
    style=-1==commaPos ? KFI_WEIGHT_REGULAR : name.mid(commaPos+2);
}

static bool      theSystemService=false;
static Folder    theFolders[FontInst::FOLDER_COUNT];
static const int constSystemReconfigured=-1;
static const int constConnectionsTimeout = 5*60*1000;
static const int constFontListTimeout    = 10 * 1000;

typedef void (*SignalHandler)(int);

static void registerSignalHandler(SignalHandler handler)
{
    if (!handler)
        handler = SIG_DFL;

    sigset_t mask;
    sigemptyset(&mask);

#ifdef SIGSEGV
    signal(SIGSEGV, handler);
    sigaddset(&mask, SIGSEGV);
#endif
#ifdef SIGFPE
    signal(SIGFPE, handler);
    sigaddset(&mask, SIGFPE);
#endif
#ifdef SIGILL
    signal(SIGILL, handler);
    sigaddset(&mask, SIGILL);
#endif
#ifdef SIGABRT
    signal(SIGABRT, handler);
    sigaddset(&mask, SIGABRT);
#endif

    sigprocmask(SIG_UNBLOCK, &mask, 0);
}

void signalHander(int)
{
    registerSignalHandler(0L);
    theFolders[theSystemService ? FontInst::FOLDER_SYS : FontInst::FOLDER_USER].saveDisabled();
    registerSignalHandler(signalHander);
}

FontInst::FontInst(bool onSystemBus)
        : itsSystemInterface(0L)
{
    theSystemService=onSystemBus;
    registerTypes();

    new FontinstAdaptor(this);
    QDBusConnection bus=theSystemService ? QDBusConnection::systemBus() : QDBusConnection::sessionBus();
    
    KFI_DBUG << "Connecting to" << (theSystemService ? "system" : "session") << "bus";
    if(!bus.registerService(OrgKdeFontinstInterface::staticInterfaceName()))
    {
        KFI_DBUG << "Failed to register service!";
        ::exit(-1);
    }
    if(!bus.registerObject(FONTINST_PATH, this))
    {
        KFI_DBUG << "Failed to register object!";
        ::exit(-1);
    }

    registerSignalHandler(signalHander);
    itsConnectionsTimer=new QTimer(this);
    itsFontListTimer=new QTimer(this);
    connect(itsConnectionsTimer, SIGNAL(timeout()), SLOT(connectionsTimeout()));
    if(!theSystemService)
        connect(itsFontListTimer, SIGNAL(timeout()), SLOT(fontListTimeout()));
    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);

    for(int i=0; i<(theSystemService ? 1 : FOLDER_COUNT); ++i)
        theFolders[i].init(FOLDER_SYS==i, theSystemService);
    
    updateFontList(false);
}

FontInst::~FontInst()
{
    closeSystemConnection();
    for(int i=0; i<(theSystemService ? 1 : FOLDER_COUNT); ++i)
        theFolders[i].saveDisabled();
}

void FontInst::list(int folders, int pid)
{
    KFI_DBUG << folders << pid;

    itsConnections.insert(pid);
    updateFontList(false);
    QList<KFI::Families> fonts;

    for(int i=0; i<(theSystemService ? 1 : FOLDER_COUNT); ++i)
        if(0==folders || folders&(1<<i))
            fonts+=theFolders[i].list();

    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);
    emit fontList(pid, fonts);
}

void FontInst::stat(const QString &name, int folders, int pid)
{
    KFI_DBUG << name << folders << pid;

    bool                      checkSystem=0==folders || folders&SYS_MASK || theSystemService,
                              checkUser=0==folders || (folders&USR_MASK && !theSystemService);
    FamilyCont::ConstIterator fam;
    StyleCont::ConstIterator  st;

    itsConnections.insert(pid);
    if( (checkSystem && findFont(name, FOLDER_SYS, fam, st)) ||
        (checkUser && findFont(name, FOLDER_USER, fam, st, !checkSystem)) )
    {
        Family rv((*fam).name());
        rv.add(*st);
        KFI_DBUG << "Found font, emit details...";
        emit fontStat(pid, rv);
    }
    else
    {
        KFI_DBUG << "Font not found, emit empty details...";
        emit fontStat(pid, Family(name));
    }
}

void FontInst::install(const QString &file, bool createAfm, bool toSystem, int pid, unsigned int xid, bool checkConfig)
{
    KFI_DBUG << file << createAfm << toSystem << pid << xid << checkConfig;

    itsConnections.insert(pid);

    if(toSystem && !theSystemService)
    {
        if(authenticate(pid, xid))
        {
            if(openSystemConnection(pid))
            {
                KFI_DBUG << "forward install command to system...";
                itsSystemActivePids.append(pid);
                itsSystemInterface->install(file, createAfm, true, pid, xid, checkConfig);
            }
        }
        else
        {
            KFI_DBUG << "authentication failed";
            emit status(pid, KIO::ERR_COULD_NOT_AUTHENTICATE);
        }
    }
    else if(!isAuthenticated(pid))
    {
        KFI_DBUG << "not authenticated";
        emit status(pid, KIO::ERR_COULD_NOT_AUTHENTICATE);
    }
    else
    {
        KFI_DBUG << "process";
        if(checkConfig)
            updateFontList();

        EFolder          folder=theSystemService || toSystem ? FOLDER_SYS : FOLDER_USER;
        Family           font;
        Utils::EFileType type=Utils::check(file, font);

        int result=Utils::FILE_BITMAP==type && !FC::bitmapsEnabled()
                        ? STATUS_BITMAPS_DISABLED
                        : Utils::FILE_INVALID==type
                            ? STATUS_NOT_FONT_FILE
                            : STATUS_OK;

        if(STATUS_OK==result)
        {
            if(Utils::FILE_AFM!=type && Utils::FILE_PFM!=type)
                for(int i=0; i<(theSystemService ? 1 : FOLDER_COUNT) && STATUS_OK==result; ++i)
                    if(theFolders[i].contains(font.name(), (*font.styles().begin()).value()))
                        result=STATUS_ALREADY_INSTALLED;

            if(STATUS_OK==result)
            {
                QString name(Utils::modifyName(Misc::getFile(file))),
                        destFolder(Utils::getDestFolder(theFolders[folder].location(), name));

                result=Utils::FILE_AFM!=type && Utils::FILE_PFM!=type && Misc::fExists(destFolder+name) ? (int)KIO::ERR_FILE_ALREADY_EXIST : (int)STATUS_OK;
                if(STATUS_OK==result)
                {
                    if(!Misc::dExists(destFolder))
                        result=Misc::createDir(destFolder) ? (int)STATUS_OK : (int)KIO::ERR_WRITE_ACCESS_DENIED;

                    if(STATUS_OK==result)
                        result=QFile::copy(file, destFolder+name) ? (int)STATUS_OK : (int)KIO::ERR_WRITE_ACCESS_DENIED;

                    if(STATUS_OK==result)
                    {
                        Misc::setFilePerms(QFile::encodeName(destFolder+name));
                        if((Utils::FILE_SCALABLE==type || Utils::FILE_PFM==type) && createAfm)
                            Utils::createAfm(destFolder+name, type);
                    }

                    if(STATUS_OK==result && Utils::FILE_AFM!=type && Utils::FILE_PFM!=type)
                    {
                        StyleCont::ConstIterator st(font.styles().begin());
                        FileCont::ConstIterator  f((*st).files().begin());
                        File                     df(destFolder+name, (*f).foundry(), (*f).index());

                        (*st).clearFiles();
                        (*st).add(df);
                        theFolders[folder].add(font);
                        theFolders[folder].addModifiedDir(destFolder);
                        emit fontsAdded(Families(font, FOLDER_SYS==folder));
                    }
                }
            }
        }
        
        emit status(pid, result);
    }
    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);
}

void FontInst::uninstall(const QString &family, quint32 style, bool fromSystem, int pid, unsigned int xid, bool checkConfig)
{
    KFI_DBUG << family << style << fromSystem << pid << xid << checkConfig;

    itsConnections.insert(pid);
    if(fromSystem && !theSystemService)
    {
        if(authenticate(pid, xid))
        {
            if(openSystemConnection(pid))
            {
                KFI_DBUG << "forward uninstall command to system...";
                itsSystemActivePids.append(pid);
                itsSystemInterface->uninstall(family, style, true, pid, xid, checkConfig);
            }
        }
        else
        {
            KFI_DBUG << "authentication failed";
            emit status(pid, KIO::ERR_COULD_NOT_AUTHENTICATE);
        }
    }
    else if(!isAuthenticated(pid))
    {
        KFI_DBUG << "not authenticated";
        emit status(pid, KIO::ERR_COULD_NOT_AUTHENTICATE);
    }
    else
    {
        KFI_DBUG << "process";
        if(checkConfig)
            updateFontList();
        
        EFolder                   folder=theSystemService || fromSystem ? FOLDER_SYS : FOLDER_USER;
        FamilyCont::ConstIterator fam;
        StyleCont::ConstIterator  st;
        int                       result=checkWriteAction(family, style, folder, fam, st);

        if(STATUS_OK==result)
        {
            Family                  del((*fam).name());
            Style                   s((*st).value(), (*st).scalable(), (*st).writingSystems());
            FileCont                files((*st).files());
            FileCont::ConstIterator it(files.begin()),
                                    end(files.end());

            for(; it!=end; ++it)
                if(!Misc::fExists((*it).path()) || QFile::remove((*it).path()))
                {
                    // Also remove any AFM or PFM files...
                    QStringList other;
                    Misc::getAssociatedFiles((*it).path(), other);
                    QStringList::ConstIterator oit(other.constBegin()),
                                               oend(other.constEnd());
                    for(; oit!=oend; ++oit)
                        QFile::remove(*oit);

                    theFolders[folder].addModifiedDir(Misc::getDir((*it).path()));
                    (*st).remove(*it);
                    s.add(*it);
                }

            if((*st).files().isEmpty())
                (*fam).remove(*st);
            else
                result=STATUS_PARTIAL_DELETE;
            del.add(s);
            emit fontsRemoved(Families(del, FOLDER_SYS==folder));
        }
        KFI_DBUG << "status" << result;
        emit status(pid, result);
    }

    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);
}

void FontInst::uninstall(const QString &name, bool fromSystem, int pid, unsigned int xid, bool checkConfig)
{
    KFI_DBUG << name << fromSystem  << pid << xid << checkConfig;

    FamilyCont::ConstIterator fam;
    StyleCont::ConstIterator  st;
    if(findFont(name, fromSystem || theSystemService ? FOLDER_SYS : FOLDER_USER, fam, st))
        uninstall((*fam).name(), (*st).value(), fromSystem, pid, xid, checkConfig);
    else
        emit status(pid, KIO::ERR_DOES_NOT_EXIST);
}

void FontInst::move(const QString &family, quint32 style, bool toSystem, int pid, unsigned int xid, bool checkConfig)
{
    KFI_DBUG << family << style << toSystem << pid << xid << checkConfig;

    itsConnections.insert(pid);
    if(checkConfig)
        updateFontList();

    if(theSystemService)
        emit status(pid, KIO::ERR_UNSUPPORTED_ACTION);
    else
    {
        FamilyCont::ConstIterator fam;
        StyleCont::ConstIterator  st;
        if(findFont(family, style, toSystem ? FOLDER_USER : FOLDER_SYS, fam, st))
        {
            if(authenticate(pid, xid))
            {
                if(openSystemConnection(pid))
                {
                    KFI_DBUG << "forward move command to system...";
                    itsSystemActivePids.append(pid);
                    Family moveFam((*fam).name());
                    moveFam.add(*st);
                    itsSystemInterface->move(Families(moveFam, !toSystem),
                                             theFolders[toSystem ? FOLDER_SYS : FOLDER_USER].location(),
                                             toSystem, getuid(), getgid(), pid);
                }
            }
            else
            {
                KFI_DBUG << "authentication failed";
                emit status(pid, KIO::ERR_COULD_NOT_AUTHENTICATE);
            }
        }
        else
        {
            KFI_DBUG << "does not exist";
            emit status(pid, KIO::ERR_DOES_NOT_EXIST);
        }
    }

    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);
}

static bool renameFontFile(const QString &from, const QString &to, int uid=-1, int gid=-1)
{
    QByteArray src(QFile::encodeName(from)),
               dest(QFile::encodeName(to));

    if(KDE_rename(src.data(), dest.data()))
        return false;

    Misc::setFilePerms(dest);
    if(-1!=uid && -1!=gid)
        ::chown(dest.data(), uid, gid);
    return true;
}

void FontInst::move(const Families &families, const QString &dest, bool toSystem, int uid, int gid, int pid)
{
    KFI_DBUG;

    if(!theSystemService)
        emit status(pid, KIO::ERR_UNSUPPORTED_ACTION);
    else if(!isAuthenticated(pid))
    {
        KFI_DBUG << "not authenticated";
        emit status(pid, KIO::ERR_COULD_NOT_AUTHENTICATE);
    }
    else
    {
        int result=KIO::ERR_COULD_NOT_READ;

        if(1==families.items.count())
        {
            FamilyCont::ConstIterator family(families.items.begin());

            if(1==(*family).styles().count())
            {
                StyleCont::ConstIterator st((*family).styles().begin());
                FileCont                 files((*st).files()),
                                         movedFiles;
                FileCont::ConstIterator  it(files.begin()),
                                         end(files.end());
                QHash<File, QString>     movedFonts;
                QHash<QString, QString>  movedAssoc;
                int                      toUid=toSystem ? getuid() : uid,
                                         fromUid=toSystem ? uid : getuid(),
                                         toGid=toSystem ? getgid() : gid,
                                         fromGid=toSystem ? gid : getgid();
                bool                     disabled=true;

                result=STATUS_OK;
                // Move fonts!
                for(; it!=end && STATUS_OK==result; ++it)
                {
                    QString name(Utils::modifyName(Misc::getFile((*it).path()))),
                            destFolder(Utils::getDestFolder(dest, name));

                    if(!Misc::dExists(destFolder))
                    {
                        result=Misc::createDir(destFolder) ? (int)STATUS_OK : (int)KIO::ERR_WRITE_ACCESS_DENIED;
                        if(STATUS_OK==result)
                            ::chown(QFile::encodeName(destFolder).data(), toUid, toGid);
                    }

                    if(renameFontFile((*it).path(), destFolder+name))
                    {
                        if(!Misc::isHidden(name))
                            disabled=false;
                        movedFiles.insert(File(destFolder+name, (*it).foundry(), (*it).index()));
                        movedFonts[*it]=destFolder+name;
                        // Now try to move an associated AFM or PFM files...
                        QStringList assoc;

                        Misc::getAssociatedFiles((*it).path(), assoc);

                        QStringList::ConstIterator ait(assoc.constBegin()),
                                                   aend(assoc.constEnd());

                        for(; ait!=aend && STATUS_OK==result; ++ait)
                        {
                            name=Misc::getFile(*ait);
                            if(renameFontFile(*ait, destFolder+name, toUid, toGid))
                                movedAssoc[*ait]=destFolder+name;
                            else
                                result=KIO::ERR_WRITE_ACCESS_DENIED;
                        }
                    }
                    else
                        result=KIO::ERR_WRITE_ACCESS_DENIED;
                }

                if(STATUS_OK==result)
                {
                    if(disabled)
                        theFolders[FOLDER_SYS].setDisabledDirty();

                    Family addFam((*family).name());
                    Style  addStyle((*st).value(), (*st).scalable(), (*st).writingSystems());

                    addStyle.setFiles(movedFiles);
                    addFam.add(addStyle);

                    emit fontsAdded(Families(addFam, toSystem));
                    emit fontsRemoved(families);

                    if(toSystem)
                    {
                        FamilyCont::ConstIterator f=theFolders[FOLDER_SYS].fonts().find(*family);

                        if(f==theFolders[FOLDER_SYS].fonts().end())
                            theFolders[FOLDER_SYS].addFont(addFam);
                        else
                        {
                            StyleCont::ConstIterator s=(*f).styles().find(*st);

                            if(s==(*f).styles().end())
                                (*f).add(addStyle);
                            else
                                (*s).addFiles(movedFiles);
                        }
                    }
                    else // Remove from system!
                    {
                        FamilyCont::ConstIterator f=theFolders[FOLDER_SYS].fonts().find(*family);
                        if(f!=theFolders[FOLDER_SYS].fonts().end())
                        {
                            (*f).remove(*st);
                            if((*f).styles().isEmpty())
                                theFolders[FOLDER_SYS].removeFont(*f);
                        }
                    }
                }
                else // un-move fonts!
                {
                    QHash<File, QString>::ConstIterator    fit(movedFonts.constBegin()),
                                                           fend(movedFonts.constEnd());
                    QHash<QString, QString>::ConstIterator ait(movedAssoc.constBegin()),
                                                           aend(movedAssoc.constEnd());

                    for(; fit!=fend; ++fit)
                        renameFontFile(fit.value(), fit.key().path(), fromUid, fromGid);
                    for(; ait!=aend; ++ait)
                        renameFontFile(ait.value(), ait.key(), fromUid, fromGid);
                }
            }
        }
        emit status(pid, result);
    }
}

void FontInst::enable(const QString &family, quint32 style, bool inSystem, int pid, unsigned int xid, bool checkConfig)
{
    KFI_DBUG << family << style << inSystem << pid << xid << checkConfig;
    toggle(true, family, style, inSystem, pid, xid, checkConfig);
}

void FontInst::disable(const QString &family, quint32 style, bool inSystem, int pid, unsigned int xid, bool checkConfig)
{
    KFI_DBUG << family << style << inSystem << pid << xid << checkConfig;
    toggle(false, family, style, inSystem, pid, xid, checkConfig);
}

void FontInst::removeFile(const QString &family, quint32 style, const QString &file, bool fromSystem, int pid,
                          unsigned int xid, bool checkConfig)
{
    KFI_DBUG << family << style << file << fromSystem << pid << xid << checkConfig;

    itsConnections.insert(pid);

    if(fromSystem && !theSystemService)
    {
        if(authenticate(pid, xid))
        {
            if(openSystemConnection(pid))
            {
                KFI_DBUG << "forward removeFile command to system...";
                itsSystemActivePids.append(pid);
                itsSystemInterface->removeFile(family, style, file, true, pid, xid, checkConfig);
            }
        }
        else
        {
            KFI_DBUG << "authentication failed";
            emit status(pid, KIO::ERR_COULD_NOT_AUTHENTICATE);
        }
    }
    else if(!isAuthenticated(pid))
    {
        KFI_DBUG << "not authenticated";
        emit status(pid, KIO::ERR_COULD_NOT_AUTHENTICATE);
    }
    else
    {
        KFI_DBUG << "process";
        if(checkConfig)
            updateFontList();

        // First find the family/style
        EFolder                   folder=theSystemService || fromSystem ? FOLDER_SYS : FOLDER_USER;
        FamilyCont::ConstIterator fam;
        StyleCont::ConstIterator  st;
        int                       result=findFont(family, style, folder, fam, st) ? STATUS_OK : KIO::ERR_DOES_NOT_EXIST;

        if(STATUS_OK==result)
        {
            // Family/style found - now check that the requested file is *within* the same folder as one
            // of the files linked to this font...
            FileCont                files((*st).files());
            FileCont::ConstIterator it(files.begin()),
                                    end(files.end());
            QString                 dir(Misc::getDir(file));

            result=KIO::ERR_DOES_NOT_EXIST;
            for(; it!=end && STATUS_OK!=result; ++it)
                if(Misc::getDir((*it).path())==dir)
                    result=STATUS_OK;

            if(STATUS_OK==result)
            {
                // OK, found folder - so can now proceed to delete the file...
                result=Misc::fExists(file)
                        ? QFile::remove(file)
                            ? STATUS_OK
                            : result=KIO::ERR_WRITE_ACCESS_DENIED
                        : KIO::ERR_DOES_NOT_EXIST;

                if(STATUS_OK==result)
                    theFolders[folder].addModifiedDir(dir);
            }
        }
        
        emit status(pid, result);
    }
}

void FontInst::reconfigure(int pid)
{
    KFI_DBUG << pid;
    bool sysModified(theFolders[FOLDER_SYS].isModified());

    saveDisabled();
    
KFI_DBUG << theFolders[FOLDER_USER].isModified();
    if(!theSystemService && theFolders[FOLDER_USER].isModified())
        theFolders[FOLDER_USER].configure();

    if(sysModified)
        if(theSystemService)
            theFolders[FOLDER_SYS].configure();
        else
        {
            if(openSystemConnection(pid))
            {
                itsSystemActivePids.append(pid);
                itsSystemInterface->reconfigure(pid);
                theFolders[FOLDER_SYS].clearModified();
            }
        }

    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);
    //
    // If the system font list has been modified, we cant update our list of fonts (or inform the caller) until
    // the system daemon has completed its reconfigure.
    if(!sysModified || theSystemService)
    {
        updateFontList();
        emit status(pid, theSystemService ? constSystemReconfigured : STATUS_OK);
    }
}

void FontInst::saveDisabled()
{
    if(theSystemService)
        theFolders[FOLDER_SYS].saveDisabled();
    else
        for(int i=0; i<(theSystemService ? 1 : FOLDER_COUNT); ++i)
            if(FOLDER_SYS==i && !theSystemService)
            {
                if(itsSystemInterface)
                    itsSystemInterface->saveDisabled();
                theFolders[i].saveDisabled();
            }
            else
                theFolders[i].saveDisabled();
}

void FontInst::connectionsTimeout()
{
    bool canExit(true);

    KFI_DBUG << "exiting";
    checkConnections();

    for(int i=0; i<(theSystemService ? 1 : FOLDER_COUNT); ++i)
    {
        if(theFolders[i].disabledDirty())
            canExit=false;
        theFolders[i].saveDisabled();
    }

    if(0==itsConnections.count())
    {
        if(canExit)
            qApp->exit(0);
        else // Try again later...
            itsConnectionsTimer->start(constConnectionsTimeout);
    }
}

void FontInst::fontListTimeout()
{
    updateFontList(true);
    itsFontListTimer->start(constFontListTimeout);
}

void FontInst::systemStatus(int pid, int value)
{
    KFI_DBUG;
    QList<int>::Iterator it(itsSystemActivePids.begin()),
                         end(itsSystemActivePids.end());

    for(; it!=end; ++it)
        if((*it)==pid)
        {
            itsSystemActivePids.erase(it);
            break;
        }

    if(constSystemReconfigured==value)
    {
        updateFontList();
        emit status(pid, STATUS_OK);
    }
    else
    {
        // If OK, mark system folder as being modified - so as to cause reconfigure to be passed onto
        // system service. Folder name doesn't matter here, as the system deamon will know the real
        // modified folder.
        if(STATUS_OK==value)
            theFolders[FOLDER_SYS].addModifiedDir(theFolders[FOLDER_SYS].location());
        emit status(pid, value);
    }
}

void FontInst::systemFontsAdded(const KFI::Families &families)
{
    KFI_DBUG;
    emit fontsAdded(families);

    // Merge these added fonts into our copy of the system font list...
    FamilyCont::ConstIterator family(families.items.begin()),
                              end(families.items.end());

    for(; family!=end; ++family)
    {
        if((*family).styles().count()>0)
        {
            FamilyCont::ConstIterator fam=theFolders[FOLDER_SYS].fonts().find(*family);

            if(fam!=theFolders[FOLDER_SYS].fonts().end())
            {
                StyleCont::ConstIterator style((*family).styles().begin()),
                                         end((*family).styles().end());

                for(; style!=end; ++style)
                {
                    StyleCont::ConstIterator st=(*fam).styles().find(*style);

                    if(st!=(*fam).styles().end())
                    {
                        FileCont::ConstIterator file=((*style).files().begin()),
                                                fend((*style).files().end());

                        for(; file!=fend; ++file)
                        {
                            FileCont::ConstIterator f=(*st).files().find(*file);

                            if(f==(*st).files().end())
                                (*st).add(*file);
                        }
                    }
                    else
                        (*fam).add(*style);
                }
            }
            else
                theFolders[FOLDER_SYS].addFont(*family);
            addModifedSysFolders(*family);
        }
    }
}
    
void FontInst::systemFontsRemoved(const KFI::Families &families)
{
    KFI_DBUG;
    emit fontsRemoved(families);

    // Remove these fonts from our copy of the system font list...
    FamilyCont::ConstIterator family(families.items.begin()),
                              end(families.items.end());

    for(; family!=end; ++family)
    {
        if((*family).styles().count()>0)
        {
            FamilyCont::ConstIterator fam=theFolders[FOLDER_SYS].fonts().find(*family);

            if(fam!=theFolders[FOLDER_SYS].fonts().end())
            {
                StyleCont::ConstIterator style((*family).styles().begin()),
                                         end((*family).styles().end());

                for(; style!=end; ++style)
                {
                    StyleCont::ConstIterator st=(*fam).styles().find(*style);

                    if(st!=(*fam).styles().end())
                    {
                        FileCont::ConstIterator file=((*style).files().begin()),
                                                fend((*style).files().end());

                        for(; file!=fend; ++file)
                            (*st).remove(*file);
                        if((*st).files().isEmpty())
                            (*fam).remove(*st);
                    }
                }

                if((*fam).styles().isEmpty())
                    theFolders[FOLDER_SYS].removeFont(*fam);

                addModifedSysFolders(*family);
            }
        }
    }
}

void FontInst::systemServiceOwnerChanged(const QString &name, const QString &from, const QString &to)
{
    if(to.isEmpty() && !from.isEmpty() && name==OrgKdeFontinstInterface::staticInterfaceName())
    {
        QSet<int>                pids=itsSystemActivePids.toSet();
        QSet<int>::ConstIterator it(pids.begin()),
                                 end(pids.end());

        itsSystemActivePids.clear();
        for(; it!=end; ++it)
        {
            KFI_DBUG << "service died";
            emit status(*it, STATUS_SERVICE_DIED);
        }
    }
}

void FontInst::updateFontList(bool emitChanges)
{
    // For some reason just the "!FcConfigUptoDate(0)" check does not always work :-(
    FcBool fcModified=!FcConfigUptoDate(0);

    if(fcModified ||
       theFolders[FOLDER_SYS].fonts().isEmpty() ||
       (!theSystemService && theFolders[FOLDER_USER].fonts().isEmpty()) ||
       theFolders[FOLDER_SYS].disabledDirty() ||
       (!theSystemService && theFolders[FOLDER_USER].disabledDirty()))
    {
        KFI_DBUG << "Need to refresh font lists";
        if(fcModified)
        {
            KFI_DBUG << "Re-init FC";
            if(!FcInitReinitialize())
                KFI_DBUG << "Re-init failed????";
        }

        Folder::Flat old[FOLDER_COUNT];

        if(emitChanges)
        {
            KFI_DBUG << "Flatten existing font lists";
            for(int i=0; i<(theSystemService ? 1 : FOLDER_COUNT); ++i)
                old[i]=theFolders[i].flatten();
        }

        saveDisabled();

        for(int i=0; i<(theSystemService ? 1 : FOLDER_COUNT); ++i)
            theFolders[i].clearFonts();
            
        KFI_DBUG << "update list of fonts";

        FcPattern   *pat = FcPatternCreate();
        FcObjectSet *os  = FcObjectSetBuild(FC_FILE, FC_FAMILY, FC_FAMILYLANG,
                                            FC_WEIGHT, FC_LANG, FC_CHARSET, FC_SCALABLE,
#ifndef KFI_FC_NO_WIDTHS
                                            FC_WIDTH,
#endif
                                            FC_SLANT, FC_INDEX, FC_FOUNDRY, (void*)0);

        FcFontSet   *list=FcFontList(0, pat, os);

        FcPatternDestroy(pat);
        FcObjectSetDestroy(os);

        theFolders[FOLDER_SYS].loadDisabled();
        if(!theSystemService)
            theFolders[FOLDER_USER].loadDisabled();

        if(list)
        {
            QString home(Misc::dirSyntax(QDir::homePath()));

            for (int i = 0; i < list->nfont; i++)
            {
                QString fileName(Misc::fileSyntax(FC::getFcString(list->fonts[i], FC_FILE)));

                if(!fileName.isEmpty() && Misc::fExists(fileName)) // && 0!=fileName.indexOf(constDefomaLocation))
                {
                    QString    family,
                               foundry;
                    quint32    styleVal;
                    int        index;
                    qulonglong writingSystems(WritingSystems::instance()->get(list->fonts[i]));
                    FcBool     scalable=FcFalse;

                    if(FcResultMatch!=FcPatternGetBool(list->fonts[i], FC_SCALABLE, 0, &scalable))
                        scalable=FcFalse;

                    FC::getDetails(list->fonts[i], family, styleVal, index, foundry);
                    FamilyCont::ConstIterator fam=theFolders[theSystemService || 0!=fileName.indexOf(home)
                                                                ? FOLDER_SYS : FOLDER_USER].addFont(Family(family));
                    StyleCont::ConstIterator  style=(*fam).add(Style(styleVal));
                    FileCont::ConstIterator   file=(*style).add(File(fileName, foundry, index));

                    (*style).setWritingSystems((*style).writingSystems()|writingSystems);
                    if(scalable)
                        (*style).setScalable();
                }
            }

            FcFontSetDestroy(list);
        }

        if(emitChanges)
        {
            KFI_DBUG << "Look for differences";
            for(int i=0; i<(theSystemService ? 1 : FOLDER_COUNT); ++i)
            {
                KFI_DBUG << "Flatten, and take copies...";
                Folder::Flat newList=theFolders[i].flatten(),
                             onlyNew=newList;

                KFI_DBUG << "Determine differences...";
                onlyNew.subtract(old[i]);
                old[i].subtract(newList);

                KFI_DBUG << "Emit changes...";
                Families families=onlyNew.build(theSystemService || i==FOLDER_SYS);

                if(!families.items.isEmpty())
                    emit fontsAdded(families);

                families=old[i].build(theSystemService || i==FOLDER_SYS);
                if(!families.items.isEmpty())
                    emit fontsRemoved(families);
            }
        }
        KFI_DBUG << "updated list of fonts";
    }
}

void FontInst::toggle(bool enable, const QString &family, quint32 style, bool inSystem, int pid, unsigned int xid,
                      bool checkConfig)
{
    KFI_DBUG;
    itsConnections.insert(pid);
    if(inSystem && !theSystemService)
    {
        if(authenticate(pid, xid))
        {
            if(openSystemConnection(pid))
            {
                KFI_DBUG << "forward toggle command to system...";
                itsSystemActivePids.append(pid);
                if(enable)
                    itsSystemInterface->enable(family, style, true, pid, xid, checkConfig);
                else
                    itsSystemInterface->disable(family, style, true, pid, xid, checkConfig);
            }
        }
        else
        {
            KFI_DBUG << "authentication failed";
            emit status(pid, KIO::ERR_COULD_NOT_AUTHENTICATE);
        }
    }
    else if(!isAuthenticated(pid))
    {
        KFI_DBUG << "not authenticated";
        emit status(pid, KIO::ERR_COULD_NOT_AUTHENTICATE);
    }
    else
    {
        KFI_DBUG << "process";
        if(checkConfig)
            updateFontList();

        EFolder                   folder=theSystemService || inSystem ? FOLDER_SYS : FOLDER_USER;
        FamilyCont::ConstIterator fam;
        StyleCont::ConstIterator  st;
        int                       result=checkWriteAction(family, style, folder, fam, st);

        if(STATUS_OK==result)
        {
            FileCont                files((*st).files()),
                                    toggledFiles;
            FileCont::ConstIterator it(files.begin()),
                                    end(files.end());
            QHash<File, QString>    movedFonts;
            QHash<QString, QString> movedAssoc;
            QSet<QString>           modifiedDirs;

            // Move fonts!
            for(; it!=end && STATUS_OK==result; ++it)
            {
                QString to=Misc::getDir((*it).path())+
                                QString(enable ? Misc::unhide(Misc::getFile((*it).path()))
                                               : Misc::hide(Misc::getFile((*it).path())));

                if(to!=(*it).path())
                {
                    KFI_DBUG << "MOVE:" << (*it).path() << " to " << to;
                    if(renameFontFile((*it).path(), to))
                    {
                        modifiedDirs.insert(Misc::getDir(enable ? to : (*it).path()));
                        toggledFiles.insert(File(to, (*it).foundry(), (*it).index()));
                        // Now try to move an associated AFM or PFM files...
                        QStringList assoc;

                        movedFonts[*it]=to;
                        Misc::getAssociatedFiles((*it).path(), assoc);

                        QStringList::ConstIterator ait(assoc.constBegin()),
                                                   aend(assoc.constEnd());

                        for(; ait!=aend && STATUS_OK==result; ++ait)
                        {
                            to=Misc::getDir(*ait)+
                                    QString(enable ? Misc::unhide(Misc::getFile(*ait))
                                                   : Misc::hide(Misc::getFile(*ait)));

                            if(to!=*ait)
                                if(renameFontFile(*ait, to))
                                    movedAssoc[*ait]=to;
                                else
                                    result=KIO::ERR_WRITE_ACCESS_DENIED;
                        }
                    }
                    else
                        result=KIO::ERR_WRITE_ACCESS_DENIED;
                }
            }

            if(STATUS_OK==result)
            {
                Family addFam((*fam).name()),
                       delFam((*fam).name());
                Style  addStyle((*st).value(), (*st).scalable(), (*st).writingSystems()),
                       delStyle((*st).value(), (*st).scalable(), (*st).writingSystems());

                addStyle.setFiles(toggledFiles);
                addFam.add(addStyle);
                delStyle.setFiles(files);
                delFam.add(delStyle);
                (*st).setFiles(toggledFiles);

                theFolders[folder].addModifiedDirs(modifiedDirs);
                emit fontsAdded(Families(addFam, FOLDER_SYS==folder));
                emit fontsRemoved(Families(delFam, FOLDER_SYS==folder));

                theFolders[folder].setDisabledDirty();
            }
            else // un-move fonts!
            {
                QHash<File, QString>::ConstIterator    fit(movedFonts.constBegin()),
                                                       fend(movedFonts.constEnd());
                QHash<QString, QString>::ConstIterator ait(movedAssoc.constBegin()),
                                                       aend(movedAssoc.constEnd());

                for(; fit!=fend; ++fit)
                    renameFontFile(fit.value(), fit.key().path());
                for(; ait!=aend; ++ait)
                    renameFontFile(ait.value(), ait.key());
            }
        }
        emit status(pid, result);
    }

    itsConnectionsTimer->start(constConnectionsTimeout);
    itsFontListTimer->start(constFontListTimeout);
}

void FontInst::addModifedSysFolders(const Family &family)
{
    StyleCont::ConstIterator style(family.styles().begin()),
                             styleEnd(family.styles().end());

    for(; style!=styleEnd; ++style)
    {
        FileCont::ConstIterator file((*style).files().begin()),
                                fileEnd((*style).files().end());

        for(; file!=fileEnd; ++file)
            theFolders[FOLDER_SYS].addModifiedDir(Misc::getDir((*file).path()));
    }
}

bool FontInst::authenticate(int pid, unsigned int xid)
{
    KFI_DBUG;
    return PolkitQt::Auth::computeAndObtainAuth(OrgKdeFontinstInterface::staticInterfaceName(), xid, pid);
}

bool FontInst::isAuthenticated(int pid)
{
    KFI_DBUG;
    return !theSystemService ||
           PolkitQt::Auth::isCallerAuthorized(OrgKdeFontinstInterface::staticInterfaceName(), pid, true);
}

bool FontInst::openSystemConnection(int pid)
{
    KFI_DBUG;
    if(!itsSystemInterface)
    {
        KFI_DBUG << "create sys interface";
        itsSystemInterface=new OrgKdeFontinstInterface(OrgKdeFontinstInterface::staticInterfaceName(),
                                                       FONTINST_PATH,
                                                       QDBusConnection::systemBus(), this);

        bool started=false;
        for(int check=0; check<10 && !started; ++check)
            if((started=isStarted(itsSystemInterface)))
                ::usleep(500*1000);

        if(started)
        {
            connect(itsSystemInterface, SIGNAL(status(int, int)), SLOT(systemStatus(int, int)));
            connect(itsSystemInterface, SIGNAL(fontsAdded(const KFI::Families &)), SLOT(systemFontsAdded(const KFI::Families &)));
            connect(itsSystemInterface, SIGNAL(fontsRemoved(const KFI::Families &)), SLOT(systemFontsRemoved(const KFI::Families &)));
            connect(QDBusConnection::systemBus().interface(), SIGNAL(serviceOwnerChanged(QString, QString, QString)),
                    SLOT(systemServiceOwnerChanged(QString, QString, QString)));

            KFI_DBUG << "connect to sys";
        }
        else
        {
            delete itsSystemInterface;
            itsSystemInterface=0L;
            KFI_DBUG << "Failed to start the system daemon";
            emit status(pid, STATUS_NO_SYS_CONNECTION);
            return false;
        }
    }

    return true;
}

void FontInst::closeSystemConnection()
{
    KFI_DBUG;
    if(itsSystemInterface)
    {
        delete itsSystemInterface;
        itsSystemInterface=0L;
        disconnect(QDBusConnection::systemBus().interface(), SIGNAL(serviceOwnerChanged(QString, QString, QString)));
    }
}

void FontInst::checkConnections()
{
    KFI_DBUG;
    QSet<int>::ConstIterator it(itsConnections.begin()),
                             end(itsConnections.end());
    QSet<int>                remove;

    for(; it!=end; ++it)
        if(0!=kill(*it, 0))
            remove.insert(*it);
    itsConnections.subtract(remove);
}

bool FontInst::findFontReal(const QString &family, const QString &style, EFolder folder,
                            FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st)
{
    KFI_DBUG;
    Family f(family);
    fam=theFolders[folder].fonts().find(f);
    if(theFolders[folder].fonts().end()==fam)
        return false;

    StyleCont::ConstIterator end((*fam).styles().end());
    for(st=(*fam).styles().begin(); st!=end; ++st)
        if(FC::createStyleName((*st).value())==style)
            return true;

    return false;
}

bool FontInst::findFont(const QString &font, EFolder folder,
                        FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st,
                        bool updateList)
{
    KFI_DBUG;
    QString family,
            style;

    decompose(font, family, style);

    if(!findFontReal(family, style, folder, fam, st))
        if(updateList)
        {
            // Not found, so refresh font list and try again...
            updateFontList();
            return findFontReal(family, style, folder, fam, st);
        }
        else
            return false;

    return true;
}

bool FontInst::findFontReal(const QString &family, quint32 style, EFolder folder,
                            FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st)
{
    KFI_DBUG;
    fam=theFolders[folder].fonts().find(Family(family));

    if(theFolders[folder].fonts().end()==fam)
        return false;
    else
    {
        st=(*fam).styles().find(style);

        return (*fam).styles().end()!=st;
    }
}

bool FontInst::findFont(const QString &family, quint32 style, EFolder folder,
                        FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st,
                        bool updateList)
{
    KFI_DBUG;
    if(!findFontReal(family, style, folder, fam, st))
        if(updateList)
        {
            // Not found, so refresh font list and try again...
            updateFontList();
            return findFontReal(family, style, folder, fam, st);
        }
        else
            return false;
    return true;
}

int FontInst::checkWriteAction(const QString &family, quint32 style, EFolder folder,
                               FamilyCont::ConstIterator &fam, StyleCont::ConstIterator &st)
{
    if(!findFont(family, style, folder, fam, st))
        return KIO::ERR_DOES_NOT_EXIST;

    FileCont::ConstIterator it((*st).files().begin()),
                            end((*st).files().end());

    for(; it!=end; ++it)
        if(!Misc::dWritable(Misc::getDir((*it).path())))
            return KIO::ERR_WRITE_ACCESS_DENIED;
    return STATUS_OK;
}

}
