////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKioFonts
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 05/03/2003
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////

/***************************************************************************

    NOTE: Large sections of this code are copied from kio_file
          -- can't just inherit from kio_file as kio_file uses "error(...);
             return;" So there is no way to know if an error occured!

 ***************************************************************************/

#include "KioFonts.h"
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <kio/global.h>
#include <kio/ioslave_defaults.h>
#include <kio/netaccess.h>
#include <kio/slaveinterface.h>
#include <kio/connection.h>
#include <qdatastream.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <qdir.h>
#include <kinstance.h>
#include <klargefile.h>
#include <ktempfile.h>
#include "FontEngine.h"
#include "Fontmap.h"
#include "XConfig.h"
#include "kxftconfig.h"
#include <kdesu/su.h>
#include <kprocess.h>

#define MAX_IPC_SIZE  (1024*32)
#define TIMEOUT       2         // Time between last mod and writing files...
#define MAX_NEW_FONTS 20        // Number of fonts that can be installed before automatically configuring (related to timeout above)
#ifdef HAVE_FONTCONFIG
#define XFT_CACHE_CMD "fc-cache"
#else
#define XFT_CACHE_CMD "xftcache"
#endif

extern "C"
{
    int kdemain(int argc, char **argv);
}

int kdemain(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: kio_" KIO_FONTS_PROTOCOL " protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    KLocale::setMainCatalogue("kfontinst");

    KInstance instance("kio_" KIO_FONTS_PROTOCOL);
    CKioFonts slave(argv[2], argv[3]);

    slave.dispatchLoop();

    return 0;
}

enum ExistsType
{
    EXISTS_DIR,
    EXISTS_FILE,
    EXISTS_NO
};

static ExistsType checkIfExists(const QStringList &list, const QString &item)
{
    QStringList::ConstIterator it;

    for(it=list.begin(); it!=list.end(); ++it)
    {
        QString item(*it+item);

        if(CMisc::fExists(item, true))
            return EXISTS_FILE;
        else if(CMisc::dExists(item))
            return EXISTS_DIR;
    }

    return EXISTS_NO;
}

static bool isSpecialDir(const QString &dir, const QString &sub, bool sys)
{
    KFI_DBUG << "isSpecialDir(" << dir << ", " << sub << ", " << sys << ')' << endl;
    QString ds(CMisc::dirSyntax(dir));

    if( (sys && -1!=CGlobal::cfg().getSysFontsDirs().findIndex(ds)) ||
        (!sys && -1!=CGlobal::cfg().getUserFontsDirs().findIndex(ds)) )
    {
        KFI_DBUG << "...need to check" << endl;
        if(sys || CMisc::root())
            return "CID"==sub || "encodings"==sub || "util"==sub;
        else
            return "kde-override"==sub;
    }
    KFI_DBUG << "...false" << endl;
    return false;
}

static void addAtom(KIO::UDSEntry &entry, unsigned int ID, long l, const QString &s=QString::null)
{
    KIO::UDSAtom atom;
    atom.m_uds = ID;
    atom.m_long = l;
    atom.m_str = s;
    entry.append(atom);
}

static bool createUDSEntry(KIO::UDSEntry &entry, const QString &name, const QString &path, const QString &url, const QString &mime, bool file)
{
    KFI_DBUG << "createUDSEntry " << name << ' ' << path << ' ' << url << endl;

    KDE_struct_stat buff;
    QString         formattedPath;
    QCString        cPath(QFile::encodeName(path));

    entry.clear();

    bool exists=-1!=KDE_lstat(cPath, &buff),
         formattedExists=false;

    if(!exists && file)
    {
        formattedPath=CMisc::formatFileName(path);
        formattedExists=-1!=KDE_lstat(QFile::encodeName(formattedPath), &buff);
    }

    if(exists || formattedExists)
    {
        if(file && (CFontEngine::isAFontOrAfm(cPath)))
        {
            QString fontName(CGlobal::fe().createName(exists ? path : formattedPath));

            if(fontName.isEmpty())
                addAtom(entry, KIO::UDS_NAME, 0, name);
            else
            {
                if(CFontEngine::isAAfm(cPath))
                    fontName+=i18n(", Metrics");
            
                if(name[0]=='.')
                    fontName+=i18n(KIO_FONTS_DISABLED);
                addAtom(entry, KIO::UDS_NAME, 0, fontName);
                addAtom(entry, KIO::UDS_URL, 0, url);
                CGlobal::fe().closeFont();
            }
        }
        else
        {
            bool    disabled=name[0]=='.';
            QString displayedName(disabled ? name.mid(1) : name);

            if(disabled)
                displayedName+=i18n(KIO_FONTS_DISABLED);
            addAtom(entry, KIO::UDS_NAME, 0, displayedName);
            addAtom(entry, KIO::UDS_URL, 0, url);
        }

        if (S_ISLNK(buff.st_mode))
        {
            KFI_DBUG << path << " is a link" << endl;

            char buffer2[1000];
            int n=readlink(cPath, buffer2, 1000);
            if(n!= -1)
                buffer2[n]='\0';

            addAtom(entry, KIO::UDS_LINK_DEST, 0, QString::fromLocal8Bit(buffer2));

            if(-1==KDE_stat(cPath, &buff))
            {
                // It is a link pointing to nowhere
                addAtom(entry, KIO::UDS_FILE_TYPE, S_IFMT - 1);
                addAtom(entry, KIO::UDS_ACCESS, S_IRWXU | S_IRWXG | S_IRWXO);
                addAtom(entry, KIO::UDS_SIZE, 0);
                goto notype;
            }
        }

        addAtom(entry, KIO::UDS_FILE_TYPE, buff.st_mode&S_IFMT);
        addAtom(entry, KIO::UDS_ACCESS, buff.st_mode&07777);
        addAtom(entry, KIO::UDS_SIZE, buff.st_size);

        notype:
        addAtom(entry, KIO::UDS_MODIFICATION_TIME, buff.st_mtime);

        struct passwd *user = getpwuid(buff.st_uid);
        addAtom(entry, KIO::UDS_USER, 0, user ? user->pw_name : QString::number(buff.st_uid).latin1());

        struct group *grp = getgrgid(buff.st_gid);
        addAtom(entry, KIO::UDS_GROUP, 0, grp ? grp->gr_name : QString::number(buff.st_gid).latin1());

        addAtom(entry, KIO::UDS_ACCESS_TIME, buff.st_atime);
        addAtom(entry, KIO::UDS_MIME_TYPE, 0, mime);
        addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, "application/octet-stream");
        return true;
    }
    else if (!CMisc::root() && CGlobal::cfg().getSysFontsDirs().first()==path)
    {
        QStringList::ConstIterator it=CGlobal::cfg().getSysFontsDirs().begin();

        it++;  // Skip 1st one - thats this one!!!
        for(; it!=CGlobal::cfg().getSysFontsDirs().end(); ++it)
            if(createUDSEntry(entry, name, *it, url, mime, file))
                return true;
        return createUDSEntry(entry, name, "/", url, mime, file);
    }

    return false;
}

#define createDirEntry(A, B, C, D, E) createUDSEntry(A, B, C, D, E ? KIO_FONTS_PROTOCOL"/system-folder" : KIO_FONTS_PROTOCOL"/folder", false)

static bool createFileEntry(KIO::UDSEntry &entry, const QString &fName, const QString &fPath, const QString &url)
{
    KFI_DBUG << "createFileEntry " << fName << endl;

    bool     err=false;
    QString  mime;

    switch(CFontEngine::getType(QFile::encodeName(fName)))
    {
        case CFontEngine::TRUE_TYPE:
            mime="application/x-font-ttf";
            break;
        case CFontEngine::TT_COLLECTION:
            mime="application/x-font-ttc";
            break;
        case CFontEngine::OPEN_TYPE:
            mime="application/x-font-otf";
            break;
        case CFontEngine::TYPE_1:
            mime="application/x-font-type1";
            break;
        case CFontEngine::SPEEDO:
            mime="application/x-font-speedo";
            break;
        case CFontEngine::TYPE_1_AFM:
            mime="application/x-afm";
            break;
        case CFontEngine::BDF:
            mime="application/x-font-bdf";
            break;
        case CFontEngine::SNF:
            mime="application/x-font-snf";
            break;
        case CFontEngine::PCF:
            mime="application/x-font-pcf";
            break;
        default:
            err=true;
    }

    if(!err)
        err=!createUDSEntry(entry, fName, fPath, QString(KIO_FONTS_PROTOCOL)+QChar(':')+url, mime, true);

    return !err;
}

static bool checkUrl(const KURL &u)
{
    if(CMisc::root())
        return true;
    else
    {
        QString sect(CMisc::getSect(u.path()));

        return i18n(KIO_FONTS_USER)==sect || i18n(KIO_FONTS_SYS)==sect;
    }
}

static bool nonRootSys(const KURL &u)
{
    return !CMisc::root() && i18n(KIO_FONTS_SYS)==CMisc::getSect(u.path());
}

#define CHECK_URL(U) \
if(!checkUrl(U))\
{ \
    error(KIO::ERR_SLAVE_DEFINED, i18n("Please specify \"%1\" or \"%2\".") \
    .arg(i18n(KIO_FONTS_USER)).arg(i18n(KIO_FONTS_SYS))); \
    return; \
}

#define CHECK_URL_ROOT_OK(U) \
if("/"!=url.path() && !checkUrl(U)) \
{ \
    error(KIO::ERR_SLAVE_DEFINED, i18n("Please specify \"%1\" or \"%2\".") \
    .arg(i18n(KIO_FONTS_USER)).arg(i18n(KIO_FONTS_SYS))); \
    return; \
}

#define CHECK_ALLOWED(u) \
if (u.path()==QString(QChar('/')+i18n(KIO_FONTS_USER)) || \
    u.path()==QString(QChar('/')+i18n(KIO_FONTS_SYS)) ) \
{ \
    error(KIO::ERR_SLAVE_DEFINED, i18n("Sorry, you cannot rename, move, or delete either \"%1\" or \"%2\".") \
    .arg(i18n(KIO_FONTS_USER)).arg(i18n(KIO_FONTS_SYS))); \
    return; \
}

static void checkPath(const QCString &path, bool &exists, bool &hidden)
{
    int slashPos=path.findRev('/', path.length()-2);

    if(-1!=slashPos)
    {
        QCString        item(path.mid(slashPos+1)),
                        newItem,
                        newPath=path.left(slashPos+1);
        KDE_struct_stat buff;

        if('.'==item[0])
        {
            newItem=item.mid(1);   // Remove dot
            hidden=false;
        }
        else
        {
            hidden=true;
            newItem=".";           // Add dot
            newItem+=item;
        }

        newPath+=newItem;

        exists=-1!=KDE_stat(newPath.data(), &buff);
    }
    else
        hidden=false;

    KFI_DBUG << "checkPath " << path << " " << hidden << " " << exists << endl;
}

static void checkPath(const QStringList &dirs, const QString &item, bool &exists, bool &hidden)
{
    QStringList::ConstIterator it;
    bool                       le=false,
                               lh=false;

    for(it=dirs.begin(); it!=dirs.end() && !le; ++it)
        checkPath(QFile::encodeName(*it+item), le, lh);
    exists=le;
    hidden=lh;
}

CKioFonts::CKioFonts(const QCString &pool, const QCString &app)
         : KIO::SlaveBase(KIO_FONTS_PROTOCOL, pool, app),
           itsNewFonts(0),
           itsLastDest(DEST_UNCHANGED),
           itsLastDestTime(0)
{
    KFI_DBUG << "Constructor" << endl;
    CGlobal::create(true, true); // Load X config files...
    syncDirs();

    // Set core dump size to 0 because we will have
    // root's password in memory.
    struct rlimit rlim;
    rlim.rlim_cur=rlim.rlim_max=0;
    itsCanStorePasswd=setrlimit(RLIMIT_CORE, &rlim) ? false : true;
}

CKioFonts::~CKioFonts()
{
    KFI_DBUG << "Destructor" << endl;
    doModifiedDirs();
    CGlobal::destroy();
}

void CKioFonts::listDir(const KURL &url)
{
    KFI_DBUG << "listDir " << url.path() << endl;

    KIO::UDSEntry entry;
    int           size=0;

    if(CMisc::root())
    {
        size=getSize(CGlobal::cfg().getUserFontsDirs(), url.encodedPathAndQuery(-1), false);
        totalSize(size);
        listDir(CGlobal::cfg().getUserFontsDirs(), url.encodedPathAndQuery(-1), url, false);
    }
    else if(QStringList::split('/', url.path(), false).count()==0)
    {
        size=2;
        totalSize(size);
        createDirEntry(entry, i18n(KIO_FONTS_USER), CGlobal::cfg().getUserFontsDirs().first(),
                       QString(KIO_FONTS_PROTOCOL)+QString(":/")+i18n(KIO_FONTS_USER), false);
        listEntry(entry, false);
        createDirEntry(entry, i18n(KIO_FONTS_SYS), CGlobal::cfg().getSysFontsDirs().first(),
                       QString(KIO_FONTS_PROTOCOL)+QString(":/")+i18n(KIO_FONTS_SYS), true);
        listEntry(entry, false);
        addDir(CGlobal::cfg().getUserFontsDirs().first());
        cfgDir(CGlobal::cfg().getUserFontsDirs().first(), "/");
    }
    else
    {
        const QStringList &top=CGlobal::cfg().getRealTopDirs(url.path());

        size=getSize(top, CMisc::getSub(url.path()), i18n(KIO_FONTS_SYS)==CMisc::getSect(url.path()));
        totalSize(size);
        listDir(top, CMisc::getSub(url.path()), url, i18n(KIO_FONTS_SYS)==CMisc::getSect(url.path()));
    }

    listEntry(size ? entry : KIO::UDSEntry(), true);
    finished();

    KFI_DBUG << "listDir - finished!" << endl;
}

int CKioFonts::getSize(const QStringList &top, const QString &sub, bool sys)
{
    KFI_DBUG << "getSize " << top.first() << ", " << sub << ", " << sys << endl;

    QStringList                entries;
    QStringList::ConstIterator it;

    for(it=top.begin(); it!=top.end(); ++it)
    {
        QString             ds(CMisc::dirSyntax(*it+sub));
        QDir                dir(ds);
        const QFileInfoList *files=dir.entryInfoList(QDir::Dirs|QDir::Files|QDir::Hidden);

        KFI_DBUG << "Looking in " << ds << endl;

        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;

            for(; NULL!=(fInfo=it.current()); ++it)
                if("."!=fInfo->fileName() && ".."!=fInfo->fileName() &&
                   (fInfo->isDir() || CFontEngine::isAFontOrAfm(QFile::encodeName(fInfo->fileName()))) &&
                   !isSpecialDir(fInfo->dirPath(), fInfo->fileName(), sys) && !entries.contains(fInfo->fileName()))
                    entries.append(fInfo->fileName());
        }
    }

    KFI_DBUG << "Size:" << entries.count() << endl;
    return entries.count();
}

void CKioFonts::listDir(const QStringList &top, const QString &sub, const KURL &url, bool sys)
{
    KFI_DBUG << "listDir " << top.first() << ", " << sub << ", " << sys << endl;

    QStringList                entries;
    QStringList::ConstIterator it;

    for(it=top.begin(); it!=top.end(); ++it)
    {
        KIO::UDSEntry       entry;
        QString             dPath(CMisc::dirSyntax(*it+sub)),
                            name(CMisc::getName(sub));
        QDir                dir(dPath);
        const QFileInfoList *files=dir.entryInfoList(QDir::Dirs|QDir::Files|QDir::Hidden);
        CXConfig            &xcfg=sys ? CGlobal::sysXcfg() : CGlobal::userXcfg();

        KFI_DBUG << "Looking in " << dPath << endl;

        //
        // Ensure this dir is in fontpath - if it exists, and it contains fonts!
        if(!sys && (sub.isEmpty() || (!name.isEmpty() && !sys && QChar('.')!=name[0] && !isSpecialDir(CMisc::getDir(dPath), name, sys))))
        {
            addDir(dPath);
            cfgDir(dPath, sub);
        }

        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;

            for(; NULL!=(fInfo=it.current()); ++it)
                if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                    if(fInfo->isDir())
                    {
                        if(!entries.contains(fInfo->fileName()) && !isSpecialDir(fInfo->dirPath(), fInfo->fileName(), sys))
                        {
                            QString ds(CMisc::dirSyntax(fInfo->filePath()));

                            if((QChar('.')==fInfo->fileName()[0] || (!sys && addDir(ds)) || xcfg.subInPath(ds)) &&
                               createDirEntry(entry, fInfo->fileName(), ds, QString(KIO_FONTS_PROTOCOL)+QChar(':')+CMisc::dirSyntax(url.path())+
                                              fInfo->fileName(), sys && !CMisc::root()))
                            {
                                if(!sys && QChar('.')!=fInfo->fileName()[0])
                                    cfgDir(ds, fInfo->fileName());
                                listEntry(entry, false);
                                entries.append(fInfo->fileName());
                            }
                        }
                    }
                    else if(( CFontEngine::isAFontOrAfm(QFile::encodeName(fInfo->fileName())) ) &&
                            !entries.contains(fInfo->fileName()) &&
                            createFileEntry(entry, fInfo->fileName(), fInfo->filePath(), url.path()+QChar('/')+fInfo->fileName()))
                    {
                        listEntry(entry, false);
                        entries.append(fInfo->fileName());
                    }
        }
    }
}

void CKioFonts::stat(const KURL &url)
{
    KFI_DBUG << "stat " << url.path() << endl;
    CHECK_URL_ROOT_OK(url)

    QStringList   path(QStringList::split('/', url.path(-1), false));
    KIO::UDSEntry entry;
    bool          err=false;

    KFI_DBUG << "count:" << path.count() << endl;
    switch(path.count())
    {
        case 0:
            err=!createDirEntry(entry, i18n("Fonts"), CGlobal::cfg().getUserFontsDirs().first(),
                                QString(KIO_FONTS_PROTOCOL)+QString(":/"), false);
            break;
        case 1:
            if(CMisc::root())
                err=!createStatEntry(entry, url);
            else
                if(i18n(KIO_FONTS_USER)==path[0])
                    err=!createDirEntry(entry, i18n(KIO_FONTS_USER), CGlobal::cfg().getUserFontsDirs().first(),
                                        QString(KIO_FONTS_PROTOCOL)+QString(":/")+i18n(KIO_FONTS_USER), false);
                else if(path[0]==i18n(KIO_FONTS_SYS))
                    err=!createDirEntry(entry, i18n(KIO_FONTS_SYS), CGlobal::cfg().getSysFontsDirs().first(),
                                        QString(KIO_FONTS_PROTOCOL)+QString(":/")+i18n(KIO_FONTS_SYS), true);
                else
                {
                    error(KIO::ERR_SLAVE_DEFINED, i18n("Please specify \"%1\" or \"%2\".").arg(KIO_FONTS_USER).arg(KIO_FONTS_SYS));
                    return;
                }
            break;
        default:
            err=!createStatEntry(entry, url, i18n(KIO_FONTS_SYS)==CMisc::getSect(url.path()));
    }

    if(err)
    {
        error(KIO::ERR_DOES_NOT_EXIST, url.path(-1));
        return;
    }

    statEntry(entry);
    finished();
}

bool CKioFonts::createStatEntry(KIO::UDSEntry &entry, const KURL &url, bool sys)
{
    KFI_DBUG << "createStatEntry " << url.path() << endl;

    const QStringList          top=CGlobal::cfg().getRealTopDirs(url.path());
    QStringList::ConstIterator it;
    QString                    name(CMisc::getName(url.path()));

    for(it=top.begin(); it!=top.end(); ++it)
    {
        QString sub(CMisc::getSub(url.path())),
                ds(CMisc::dirSyntax(*it+sub));

        QDir d(ds);

        if(d.exists())
        {
            if(!isSpecialDir(CMisc::getDir(ds), name, sys))
            {
                CXConfig xcfg=sys ? CGlobal::sysXcfg() : CGlobal::userXcfg();

                if((QChar('.')==name[0] || xcfg.inPath(ds) || (!sys && addDir(ds))) && createDirEntry(entry, name, ds, 
                                QString(KIO_FONTS_PROTOCOL)+QChar(':')+url.path(), sys && !CMisc::root()))
                {
                    if(!sys && QChar('.')!=name[0])
                        cfgDir(ds, sub);
                    return true;
                }
            }
        }
        else if(CMisc::fExists(*it+sub, true) &&
                (CFontEngine::isAFontOrAfm(QFile::encodeName(name))) &&
                createFileEntry(entry, name, *it+sub, url.path()))
            return true;
    }

    return false;
}

void CKioFonts::get(const KURL &url)
{
    KFI_DBUG << "get " << url.path() << endl;
    CHECK_URL(url)

    QCString realPath=QFile::encodeName(convertUrl(url, true));

    KDE_struct_stat buff;
    if (-1==KDE_stat(realPath.data(), &buff))
        error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST, url.path());
    else if (S_ISDIR(buff.st_mode))
        error(KIO::ERR_IS_DIRECTORY, url.path());
    else if (!S_ISREG(buff.st_mode))
        error(KIO::ERR_CANNOT_OPEN_FOR_READING, url.path());
    else
    {
        int fd = KDE_open(realPath.data(), O_RDONLY);
        if (fd < 0)
            error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
        else
        {
            // Determine the mimetype of the file to be retrieved, and emit it.
            // This is mandatory in all slaves (for KRun/BrowserRun to work).
            emit mimeType(KMimeType::findByURL(url, buff.st_mode, true /* local URL */ )->name());

            totalSize(buff.st_size);
            KIO::filesize_t processed=0;

            char buffer[MAX_IPC_SIZE];
            QByteArray array;

            while(1)
            {
                int n=::read(fd, buffer, MAX_IPC_SIZE);
                if (-1==n && EINTR!=errno)
                {
                    error(KIO::ERR_COULD_NOT_READ, url.path());
                    close(fd);
                    return;
                }
                if (0==n)
                    break; // Finished

                array.setRawData(buffer, n);
                data(array);
                array.resetRawData(buffer, n);

                processed+=n;
                processedSize(processed);
            }

            data(QByteArray());
            close(fd);

            processedSize(buff.st_size);
            finished();
        }
    }
}

static bool writeAll(int fd, const char *buf, size_t len)
{
   while(len>0)
   {
      ssize_t written=write(fd, buf, len);
      if (written<0 && EINTR!=errno)
          return false;
      buf+=written;
      len-=written;
   }
   return true;
}

void CKioFonts::put(const KURL &u, int mode, bool overwrite, bool resume)
{
    KFI_DBUG << "put " << u.path() << endl;

    QCString fnameC(QFile::encodeName(u.filename()));

    // Check mime-type - only want to copy fonts, or AFM files...
    if(!CFontEngine::isAFontOrAfm(fnameC))
    {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Only fonts may be installed."));
        return;
    }

    KURL       url(u);
    bool       changed=confirmUrl(url);
    ExistsType origExists=checkIfExists(CGlobal::cfg().getRealTopDirs(url.path()), CMisc::getSub(url.path()));

    if (EXISTS_NO!=origExists && !overwrite && !resume)
    {
        error(EXISTS_DIR==origExists ? KIO::ERR_DIR_ALREADY_EXIST : KIO::ERR_FILE_ALREADY_EXIST, url.path());
        return;
    }

    bool otherExists,
         otherHidden;

    checkPath(CGlobal::cfg().getRealTopDirs(url.path()), CMisc::getSub(url.path()), otherExists, otherHidden);

    if(otherExists)
    {
        error(KIO::ERR_SLAVE_DEFINED, otherHidden ? i18n("A hidden/disabled font (starting with a '.') already exists.\n"
                                                         "Please rename/enable this.")
                                                  : i18n("A normal/enabled font (not starting with a '.') already exists.\n"
                                                         "Please rename/disable this."));
        return;
    }

    QString  destOrig(CMisc::formatFileName(convertUrl(url, false)));
    QCString destOrigC(QFile::encodeName(destOrig));

    if(nonRootSys(url))
    {
        bool    err=true;
        QString passwd=getRootPasswd();

        // Need to check can get root passwd before start download...
        if(QString::null!=passwd)
        {
            // Now download to a temporary file...
            KTempFile tmpFile;
            QCString  tmpFileC(QFile::encodeName(tmpFile.name()));

            tmpFile.setAutoDelete(true);

            if(putReal(tmpFile.name(), tmpFileC, EXISTS_NO!=origExists, mode, resume))
            {
                QCString cmd;

                if(CMisc::dExists(CMisc::getDir(destOrig)))
                {
                    cmd+=("cp -f "); // "kfontinst install ");
                    cmd+=tmpFileC;
                    cmd+=" ";
                    cmd+=destOrigC;
                    cmd+="; chmod 0644 ";
                }
                else
                {
                    cmd+=("kfontinst install ");
                    cmd+=tmpFileC;
                    cmd+=" ";
                }

                cmd+=destOrigC;

                // Get root to move this to fonts folder...
                if(doRootCmd(cmd, passwd))
                {
                    modifiedDir(CMisc::getDir(destOrig), true);
                    err=false;
                }
            }
        }
        if(err)
            error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(KIO_FONTS_SYS));
    }
    else
    {
        QString dir(CMisc::getDir(destOrig));

        if(!CMisc::dExists(dir))
            CMisc::createDir(dir);

        if(putReal(destOrig, destOrigC, EXISTS_NO!=origExists, mode, resume))
            modifiedDir(CMisc::getDir(destOrig));
    }

    if(++itsNewFonts>MAX_NEW_FONTS)
    {
        setTimeoutSpecialCommand(0); // Cancel timer
        doModifiedDirs();
    }

    finished();

    if(changed)
        itsLastDestTime=time(NULL);
}

bool CKioFonts::putReal(const QString &destOrig, const QCString &destOrigC, bool origExists,
                        int mode, bool resume)
{
    bool    markPartial=config()->readBoolEntry("MarkPartial", true);
    QString dest;

    if (markPartial)
    {
        QString  destPart(destOrig+QString::fromLatin1(".part"));
        QCString destPartC(QFile::encodeName(destPart));

        dest = destPart;

        KDE_struct_stat buffPart;
        bool            partExists=(-1!=KDE_stat(destPartC.data(), &buffPart));

        if (partExists && !resume && buffPart.st_size>0)
        {
             // Maybe we can use this partial file for resuming
             // Tell about the size we have, and the app will tell us
             // if it's ok to resume or not.
             resume=canResume(buffPart.st_size);

             if (!resume)
                 if (!::remove(destPartC.data()))
                     partExists = false;
                 else
                 {
                     error(KIO::ERR_CANNOT_DELETE_PARTIAL, destPart);
                     return false;
                 }
        }
    }
    else
    {
        dest = destOrig;
        if (origExists && !resume)
            ::remove(destOrigC.data());
            // Catch errors when we try to open the file.
    }

    QCString destC(QFile::encodeName(dest));

    int fd;

    if (resume)
    {
        fd = KDE_open(destC.data(), O_RDWR);  // append if resuming
        KDE_lseek(fd, 0, SEEK_END); // Seek to end
    }
    else
    {
        // WABA: Make sure that we keep writing permissions ourselves,
        // otherwise we can be in for a surprise on NFS.
        fd = KDE_open(destC.data(), O_CREAT | O_TRUNC | O_WRONLY, -1==mode ? 0666 :  mode | S_IWUSR | S_IRUSR);
    }

    if (fd < 0)
    {
        error(EACCES==errno ? KIO::ERR_WRITE_ACCESS_DENIED : KIO::ERR_CANNOT_OPEN_FOR_WRITING, dest);
        return false;
    }

    int result;
    // Loop until we got 0 (end of data)
    do
    {
        QByteArray buffer;

        dataReq(); // Request for data
        result = readData(buffer);
        if(result > 0 && !writeAll(fd, buffer.data(), buffer.size()))
            if(ENOSPC==errno) // disk full
            {
                error(KIO::ERR_DISK_FULL, destOrig);
                result = -2; // means: remove dest file
            }
            else
            {
                error(KIO::ERR_COULD_NOT_WRITE, destOrig);
                result = -1;
            }
    }
    while(result>0);

    if (result!=0)
    {
        close(fd);
        if (-1==result)
           ::remove(destC.data());
        else if (markPartial)
        {
           KDE_struct_stat buff;

           if ((-1==KDE_stat(destC.data(), &buff)) || (buff.st_size<config()->readNumEntry("MinimumKeepSize", DEFAULT_MINIMUM_KEEP_SIZE)))
               ::remove(destC.data());
        }
        ::exit(255);
    }

    if (close(fd))
    {
        error(KIO::ERR_COULD_NOT_WRITE, destOrig);
        return false;
    }

    // after full download rename the file back to original name
    if (markPartial && ::rename(destC.data(), destOrigC.data()))
    {
        error(KIO::ERR_CANNOT_RENAME_PARTIAL, destOrig);
        return false;
    }

    ::chmod(destOrigC.data(), CMisc::FILE_PERMS);
    return true;
}

void CKioFonts::copy(const KURL &src, const KURL &d, int mode, bool overwrite)
{
    KFI_DBUG << "copy " << src.path() << " - " << d.path() << endl;
    CHECK_URL(src)

    QCString        realSrc=QFile::encodeName(convertUrl(src, true));
    KDE_struct_stat buffSrc;

    if (-1==KDE_stat(realSrc.data(), &buffSrc))
    {
        error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST, src.path());
        return;
    }
    if(S_ISDIR(buffSrc.st_mode))
    {
        error(KIO::ERR_IS_DIRECTORY, src.path());
        return;
    }
    if(S_ISFIFO(buffSrc.st_mode) || S_ISSOCK(buffSrc.st_mode))
    {
        error(KIO::ERR_CANNOT_OPEN_FOR_READING, src.path());
        return;
    }

    KURL       dest(d);
    bool       changed=confirmUrl(dest);
    QCString   realDest=QFile::encodeName(CMisc::formatFileName(convertUrl(dest, false)));
    ExistsType destExists=checkIfExists(CGlobal::cfg().getRealTopDirs(dest.path()), CMisc::getSub(dest.path()));

    KFI_DBUG << "REAL:" << realSrc << " TO REAL:" << realDest << endl;

    if (EXISTS_NO!=destExists)
    {
        if(EXISTS_DIR==destExists)
        {
           error(KIO::ERR_DIR_ALREADY_EXIST, dest.path());
           return;
        }
        if (!overwrite)
        {
           error(KIO::ERR_FILE_ALREADY_EXIST, dest.path());
           return;
        }
    }

    if(nonRootSys(dest))
    {
        QCString cmd(CMisc::dExists(CMisc::getDir(realDest)) ? "cp -f " : "kfontinst install ");

        cmd+=realSrc;
        cmd+=" ";
        cmd+=realDest;

        if(doRootCmd(cmd))
            modifiedDir(CMisc::getDir(realDest), true);
        else
        {
            error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(KIO_FONTS_SYS));
            return;
        }
    }
    else
    {
        if(-1==KDE_stat(realSrc.data(), &buffSrc))
        {
            error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST, src.path());
            return;
        }

        int srcFd=KDE_open(realSrc.data(), O_RDONLY);

        if (srcFd<0)
        {
            error(KIO::ERR_CANNOT_OPEN_FOR_READING, src.path());
            return;
        }

        QString destDir(CMisc::getDir(realDest));

        if(!CMisc::dExists(destDir))
            CMisc::createDir(destDir);

        // WABA: Make sure that we keep writing permissions ourselves,
        // otherwise we can be in for a surprise on NFS.
        int destFd=KDE_open(realDest.data(), O_CREAT | O_TRUNC | O_WRONLY, -1==mode ? 0666 : mode | S_IWUSR);

        if (destFd<0)
        {
            error(EACCES==errno ? KIO::ERR_WRITE_ACCESS_DENIED : KIO::ERR_CANNOT_OPEN_FOR_WRITING, dest.path() );
            close(srcFd);
            return;
        }

        totalSize(buffSrc.st_size);

        KIO::filesize_t processed = 0;
        char            buffer[MAX_IPC_SIZE];
        QByteArray      array;

        while(1)
        {

            int n=::read(srcFd, buffer, MAX_IPC_SIZE);

            if(-1==n && EINTR!=errno)
            {
                error( KIO::ERR_COULD_NOT_READ, src.path());
                close(srcFd);
                close(destFd);
                return;
            }
            if(0==n)
                break; // Finished

            if(!writeAll(destFd, buffer, n))
            {
                close(srcFd);
                close(destFd);
                if (ENOSPC==errno) // disk full
                {
                    error(KIO::ERR_DISK_FULL, dest.path());
                    remove(realDest.data());
                }
                else
                    error(KIO::ERR_COULD_NOT_WRITE, dest.path());
                return;
            }

            processed += n;
            processedSize(processed);
        }

        close(srcFd);

        if (close(destFd))
        {
            error(KIO::ERR_COULD_NOT_WRITE, dest.path());
            return;
        }

        ::chmod(realDest.data(), CMisc::FILE_PERMS);

        // copy access and modification time
        struct utimbuf ut;

        ut.actime = buffSrc.st_atime;
        ut.modtime = buffSrc.st_mtime;
        ::utime(realDest.data(), &ut);

        processedSize(buffSrc.st_size);
        modifiedDir(CMisc::getDir(realDest));
    }

    if(++itsNewFonts>MAX_NEW_FONTS)
    {
        setTimeoutSpecialCommand(0); // Cancel timer
        doModifiedDirs();
    }

    finished();

    if(changed)
        itsLastDestTime=time(NULL);
}

void CKioFonts::rename(const KURL &src, const KURL &dest, bool overwrite)
{
    KFI_DBUG << "rename[1] " << src.path() << " to " << dest.path() << endl;

    CHECK_URL(src)
    CHECK_ALLOWED(src)

    //
    // Konqueror's inline renaming tries to rename based upon the displayed name. However, fonts:/ displays
    // the FontName, and not filename! Therefore, to make our lives easy, only allow renaming of files in the
    // same folder if it is to add or remove a "."
    if(src.directory() == dest.directory())
    {
        QString srcFile(src.fileName()),
                destFile(dest.fileName());

        if(srcFile!=QChar('.')+destFile && destFile!=QChar('.')+srcFile)
        {
            error(KIO::ERR_SLAVE_DEFINED,
                  i18n("You can only rename fonts or folders by adding or removing a period, in order to disable or enable them respectively.\n"
                       "Also, please note the inline renaming within Konqueror will not work."));
            return;
        }
    }

    QCString srcPath(QFile::encodeName(convertUrl(src, true)));
    QString  sSub(CMisc::getSub(src.path())),
             dSub(CMisc::getSub(dest.path()));

    KDE_struct_stat buffSrc;

    if(-1==KDE_stat(srcPath.data(), &buffSrc))
    {
        error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST, src.path());
        return;
    }

    QCString destPath(QFile::encodeName(CMisc::getDir(srcPath)+CMisc::getFile(dest.path())));

    KFI_DBUG << "rename[2] " << srcPath << " to " << destPath << endl;

    ExistsType destExists=checkIfExists(CGlobal::cfg().getRealTopDirs(dest.path()), dSub);

    if(EXISTS_NO!=destExists)
    {
        if(EXISTS_DIR==destExists)
        {
           error(KIO::ERR_DIR_ALREADY_EXIST, dest.path());
           return;
        }
        if(!overwrite)
        {
           error(KIO::ERR_FILE_ALREADY_EXIST, dest.path());
           return;
        }
    }

    bool                       nrs=false,
                               dir=S_ISDIR(buffSrc.st_mode),
                               linkDir=S_ISLNK(buffSrc.st_mode) && !(CFontEngine::isAFontOrAfm(srcPath));
    QStringList::ConstIterator it;
    QStringList                list=CGlobal::cfg().getRealTopDirs(src.path());

    if(nonRootSys(dest))
    {
        QCString cmd;
        bool     first=true,
                 found=false;

        for(it=list.begin(); it!=list.end(); ++it)
            if(dir ? CMisc::dExists(*it+sSub) : CMisc::fExists(*it+sSub) || CMisc::isLink(*it+sSub))
            {
                if(first)
                    first=false;
                else
                    cmd+=" ; ";
                cmd+=dir ? "kfontinst rename " : "mv -f ";
                cmd+=QFile::encodeName(KProcess::quote(*it+sSub));
                cmd+=" ";
                cmd+=QFile::encodeName(KProcess::quote(*it+dSub));
                found=true;
            }

        nrs=true;

        if(!found || !doRootCmd(cmd))
        {
            error(KIO::ERR_CANNOT_RENAME, src.path());
            return;
        }
    }
    else
        for(it=list.begin(); it!=list.end(); ++it)
            if(dir ? CMisc::dExists(*it+sSub) : CMisc::fExists(*it+sSub) || CMisc::isLink(*it+sSub))
            {
                KFI_DBUG << "rename[3] " << *it+sSub << " to " << *it+dSub << endl;

                if(::rename(QFile::encodeName(*it+sSub).data(), QFile::encodeName(*it+dSub).data()))
                {
                    switch(errno)
                    {
                        case EACCES:
                        case EPERM:
                            error(KIO::ERR_ACCESS_DENIED, dest.path());
                            break;
                        case EXDEV:
                            error(KIO::ERR_UNSUPPORTED_ACTION, QString::fromLatin1("rename"));
                            break;
                        case EROFS: // The file is on a read-only filesystem
                            error(KIO::ERR_CANNOT_DELETE, src.path());
                            break;
                        default:
                            error(KIO::ERR_CANNOT_RENAME, src.path());
                    }
                    return;
                }
            }

    if(dir || linkDir)
    {
        if(!CMisc::hidden(srcPath, true))
            deletedDir(srcPath, nrs);
        if(!CMisc::hidden(destPath, true))
            addedDir(destPath, nrs);
    }
    else
        if(!(CMisc::hidden(srcPath) && CMisc::hidden(destPath)))
        {
            modifiedDir(CMisc::getDir(srcPath), nrs);
            if(++itsNewFonts>MAX_NEW_FONTS)
            {
                setTimeoutSpecialCommand(0); // Cancel timer
                doModifiedDirs();
            }
        }

    finished();
}

void CKioFonts::mkdir(const KURL &url, int)
{
    KFI_DBUG << "mkdir " << url.path() << endl;
    CHECK_URL(url)

    QCString        realPath=QFile::encodeName(convertUrl(url, false));
    bool            sys=nonRootSys(url);
    CXConfig        &xcfg=sys ? CGlobal::sysXcfg() : CGlobal::userXcfg();   // for root, syscfg==usercfg
    ExistsType      exists=checkIfExists(CGlobal::cfg().getRealTopDirs(url.path()), CMisc::getSub(url.path()));
    bool            otherExists,
                    otherHidden;

    if(isSpecialDir(CMisc::getDir(url.path()), CMisc::getName(url.path()), sys))
        error(KIO::ERR_SLAVE_DEFINED,
                  sys ? i18n("You cannot create a folder named \"CID\", \"encodings\", or \"util\" - as these are special "
                             "system folders (\"CID\" is for \"CID\" fonts - these are <b>not</b> handled - and "
                             "\"encodings\" and \"util\" are for X11 encoding files).")
                      : i18n("You cannot create a folder named \"kde-override\", as this is a special KDE folder."));
    else
    {
        checkPath(CGlobal::cfg().getRealTopDirs(url.path()), CMisc::getSub(url.path()), otherExists, otherHidden);

        if(EXISTS_NO!=exists && xcfg.inPath(realPath))
            error(EXISTS_DIR==exists ? KIO::ERR_DIR_ALREADY_EXIST : KIO::ERR_FILE_ALREADY_EXIST, url.path());
        else if(otherExists)
        {
            error(KIO::ERR_SLAVE_DEFINED, otherHidden ? i18n("A hidden/disabled folder (starting with a '.') already exists.\n"
                                                             "Please rename/enable this.")
                                                      : i18n("A normal/enabled folder (not starting with a '.') already exists.\n"
                                                             "Please rename/disable this."));
        }
        else
            if(sys)
            {
                QCString cmd(EXISTS_NO!=exists ? "kfontinst adddir " : "kfontinst mkdir ");
                cmd+=realPath;

                if(doRootCmd(cmd))
                {
                    addedDir(realPath, true);
                    CGlobal::cfg().storeSysXConfigFileTs();
                    finished();
                }
                else
                    error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(KIO_FONTS_SYS));
            }
            else
                if(!CMisc::createDir(realPath))
                    error(KIO::ERR_COULD_NOT_MKDIR, url.path());
                else
                {
                    addedDir(realPath);

                    ::chmod(realPath.data(), CMisc::DIR_PERMS);
                    finished();
                }
    }
}

void CKioFonts::chmod(const KURL &url, int permissions)
{
    KFI_DBUG << "chmod " << url.path() << endl;
    CHECK_URL(url)

    QCString realPath=QFile::encodeName(convertUrl(url, true));

    if(nonRootSys(url))
    {
        QCString cmd("chmod "),
                 p;

        p.setNum(permissions);
        cmd+=p;
        cmd+=" ";
        cmd+=realPath;

        if(!doRootCmd(cmd))
            error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(KIO_FONTS_SYS));
    }
    else
        if (-1==::chmod(realPath.data(), permissions))
            error( KIO::ERR_CANNOT_CHMOD, url.path());
        else
            finished();
}

void CKioFonts::del(const KURL &url, bool isFile)
{
    KFI_DBUG << "del " << url.path() << endl;
    CHECK_URL(url)
    CHECK_ALLOWED(url)

    QString                    sub(CMisc::getSub(url.path()));
    QStringList::ConstIterator constIt;
    QStringList::Iterator      it;
    const QStringList          &list=CGlobal::cfg().getRealTopDirs(url.path());
    QStringList                items;

    KFI_DBUG << "real ";
    for(constIt=list.begin(); constIt!=list.end(); ++constIt)
        if((isFile && CMisc::fExists(*constIt+sub)) ||
           (!isFile && CMisc::dExists(*constIt+sub)))
        {

            items.append(*constIt+sub);
            KFI_DBUG << *constIt+sub;
        }

    KFI_DBUG << endl;

    if(items.count())
        if(isFile)
        {
            if(nonRootSys(url))
            {
                QCString cmd("rm -f "); // "kfontinst rmfont ");

                for(it=items.begin(); it!=items.end(); ++it)
                {
                    cmd+=QFile::encodeName(KProcess::quote(*it));
                    cmd+=" ";
                }

                if(doRootCmd(cmd))
                {
                    for(it=items.begin(); it!=items.end(); ++it)
                        modifiedDir(CMisc::getDir(*it), true);
                }
                else
                    error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(KIO_FONTS_SYS));
            }
            else
                for(it=items.begin(); it!=items.end(); ++it)
                    if (0!=unlink(QFile::encodeName(*it).data()))
                    {
                        if(EACCES==errno || EPERM==errno)
                            error(KIO::ERR_ACCESS_DENIED, url.path());
                        else if(EISDIR==errno)
                            error(KIO::ERR_IS_DIRECTORY, url.path());
                        else
                            error(KIO::ERR_CANNOT_DELETE, url.path());

                        return;
                    }
                    else
                    {
                        // Remove any other associated files - file.afm
                        // NO - AFMs are now displayed! CMisc::removeAssociatedFiles(realPath);
                        modifiedDir(CMisc::getDir(*it));
                    }
        }
        else
        {
            if(nonRootSys(url))
            {
                QCString cmd("kfontinst rmdir ");

                for(it=items.begin(); it!=items.end(); ++it)
                {
                    cmd+=QFile::encodeName(KProcess::quote(*it));
                    cmd+=" ";
                }

                if(doRootCmd(cmd))
                {
                    CGlobal::cfg().storeSysXConfigFileTs();
                    for(it=items.begin(); it!=items.end(); ++it)
                        deletedDir(*it, true);
                }
                else
                    error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(KIO_FONTS_SYS));
            }
            else
            {
                // 1st may need to remove any fonts.dir, fonts.scale, encodings.dir, XftCache*,
                // fonts.cache*
                for(it=items.begin(); it!=items.end(); ++it)
                {
                    // 1st may need to remove any fonts.dir, fonts.scale, encodings.dir, XftCache*,
                    // fonts.cache*
                    CMisc::removeAssociatedFiles(*it, true);

                    if(-1==::rmdir(QFile::encodeName(*it).data()))
                        if(EACCES==errno || EPERM==errno)
                            error(KIO::ERR_ACCESS_DENIED, url.path());
                        else
                        {
                            error(KIO::ERR_COULD_NOT_RMDIR, url.path());
                            return;
                        }
                    else
                        deletedDir(*it);
                }
            }
        }

    finished();
}

//
// This is called from listDir() - and will add any non-kio_fonts created
// dirs to the fontpath (if they contain fonts *NOPE* - see below)
bool CKioFonts::addDir(const QString &ds)
{
    KFI_DBUG << "addDir " << ds << endl;

    if(!CGlobal::userXcfg().inPath(ds) && CMisc::dExists(ds))  // Check it's not already in path though!!!
    {
#ifdef ADD_DIR_SHOULD_CHECK_FOR_PRESENCE_OF_FONTS
        QDir                dir(ds);
        const QFileInfoList *files=dir.entryInfoList(QDir::Files);

        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;

            for(; NULL!=(fInfo=it.current()); ++it)
                if(QChar('.')!=fInfo->filePath()[0] && (CFontEngine::isAFontOrAfm(QFile::encodeName(fInfo->fileName())))
                {
                    // OK - this dir contains some fonts, add to config files...
                    addedDir(ds, false);
                    return true;
                }
        }
#else

        // Add dir even if it does not contain fonts. Reason: If kio_fonts does not list a dir (as it has no fonts), and then
        // a user tries to create a dir with the same name - the creation would fail, and the users would not be able to access
        // the dir!!!
        addedDir(ds, false);
        return true;
#endif
    }

    return false;
}

//
// Make sure a dir is up-to-date - i.e. check for fonts.dir
void CKioFonts::cfgDir(const QString &ds, const QString &sub)
{
    KFI_DBUG << "cfgDir " << ds << endl;

    //
    // Check that it's not one that we're gonna update later, and that either fonts.dir does not exist, or
    // the fonts.dir is much older than the last mod time of the dir
    if(-1==itsModifiedDirs.findIndex(ds) && CMisc::dExists(ds))    // Ensure its not one that we're gonna update later...
    {
        time_t dTs=CMisc::getTimeStamp(ds);
        bool   doTs=false;

        if (!CMisc::fExists(ds+"fonts.dir") || dTs!=CMisc::getTimeStamp(ds+"fonts.dir"))
        {
            infoMessage(i18n("Configuring out of date font folder (%1).").arg(sub));

            KFI_DBUG << "configure out of date x dir " << dTs << " " << CMisc::getTimeStamp(ds+"fonts.dir") << endl;

#ifdef HAVE_FONTCONFIG
            if(CXConfig::configureDir(ds))
                CGlobal::userXcfg().refreshPaths();
#else
            QStringList symFamilies;

            if(CXConfig::configureDir(ds, symFamilies))
            {
                if(symFamilies.count())
                {
                    QStringList::Iterator it;

                    for(it=symFamilies.begin(); it!=symFamilies.end(); ++it)
                        CGlobal::userXft().addSymbolFamily(*it);
                }
                CGlobal::userXcfg().refreshPaths();
            }
#endif
            if(CGlobal::userXft().changed())
                CGlobal::userXft().apply();
#ifdef HAVE_FONTCONFIG
            QStringList::ConstIterator xftIt;

            for(xftIt=CGlobal::cfg().getUserFontsDirs().begin(); xftIt!=CGlobal::cfg().getUserFontsDirs().end(); ++xftIt)
                CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(*xftIt));
#else
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(ds));
#endif
            doTs=true;
        }

        if (!CMisc::fExists(ds+"Fontmap") || dTs!=CMisc::getTimeStamp(ds+"Fontmap"))
        {
            infoMessage(i18n("Configuring out of date font folder (%1).").arg(sub));

            KFI_DBUG << "configure out of date fontmap " << dTs << " " << CMisc::getTimeStamp(ds+"Fontmap") << endl;

            if(CFontmap::createLocal(ds))
                CFontmap::createTopLevel();
            doTs=true;
        }

        if(doTs)
            CMisc::setTimeStamps(ds);
    }
}

//
// Try to ensure both xftconfig/fonts.conf and X11 config have the same
// direcories listed.
void CKioFonts::syncDirs()
{
    //
    // *Always* ensure top-level folders are in path...

    QStringList::ConstIterator uIt;

    for(uIt=CGlobal::cfg().getUserFontsDirs().begin(); uIt!=CGlobal::cfg().getUserFontsDirs().end(); ++uIt)
    {
        CGlobal::userXcfg().addPath(*uIt);
        CGlobal::userXft().addDir(*uIt);
    }

    QStringList           xftDirs(CGlobal::userXft().getDirs()),
                          x11Dirs,
                          inXftNotX11,
                          inX11NotXft;
    QStringList::Iterator it;

    CGlobal::userXcfg().getDirs(x11Dirs);

    for(it=xftDirs.begin(); it!=xftDirs.end(); ++it)
        if(!CGlobal::userXcfg().inPath(*it))
            inXftNotX11.append(*it);

    for(it=x11Dirs.begin(); it!=x11Dirs.end(); ++it)
        if(!CGlobal::userXft().hasDir(*it))
            inX11NotXft.append(*it);

    if(inXftNotX11.count())
        for(it=inXftNotX11.begin(); it!=inXftNotX11.end(); ++it)
        {
            KFI_DBUG << *it << " in xft but not x11" << endl;
            CGlobal::userXcfg().addPath(*it);
            cfgDir(*it, *it); // TODO: 2nd parameter should really be the folder name
        }

    if(inX11NotXft.count())
    {
        for(it=inX11NotXft.begin(); it!=inX11NotXft.end(); ++it)
        {
            KFI_DBUG << *it << " in x11 but not xft" << endl;
            CGlobal::userXft().addDir(*it);
        }
        CGlobal::userXft().apply();
#ifdef HAVE_FONTCONFIG
        QStringList::ConstIterator xftIt;

        for(xftIt=CGlobal::cfg().getUserFontsDirs().begin(); xftIt!=CGlobal::cfg().getUserFontsDirs().end(); ++xftIt)
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(*xftIt));
#else
        for(it=inX11NotXft.begin(); it!=inX11NotXft.end(); ++it)
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(*it));
#endif
        for(it=inX11NotXft.begin(); it!=inX11NotXft.end(); ++it)
        {
            CFontmap::createLocal(*it);
            CMisc::setTimeStamps(*it);
        }
        CFontmap::createTopLevel();
    }

    if(CGlobal::userXcfg().madeChanges())
        if(CGlobal::userXcfg().writeConfig())
        {
            if(CMisc::root())  // user cfg == sys cfg
                CGlobal::cfg().storeSysXConfigFileTs();
            CGlobal::userXcfg().refreshPaths();
        }
        else
            CGlobal::userXcfg().readConfig();

    if(CGlobal::userXft().changed()) // Would only happen if top-level added only
    {
        CGlobal::userXft().apply();

        QStringList::ConstIterator xftIt;

        for(xftIt=CGlobal::cfg().getUserFontsDirs().begin(); xftIt!=CGlobal::cfg().getUserFontsDirs().end(); ++xftIt)
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(*xftIt));
    }
}

void CKioFonts::deletedDir(const QString &d, bool sys)
{
    QString ds(CMisc::dirSyntax(d));

    if(sys)
    {
        CGlobal::sysXcfg().readConfig();

        if(CGlobal::cfg().getSysXfs())
            doRootCmd("kfontinst refresh", false);
        else
        {
            if(!CMisc::root())  // Ensure user paths are at the front of the list!
                CGlobal::userXcfg().refreshPaths();
            CGlobal::sysXcfg().refreshPaths();
            doRootCmd("kfontinst createfontmap", false);
        }
    }
    else
    {
        if(-1!=itsModifiedDirs.findIndex(ds))
            itsModifiedDirs.remove(ds);

        CGlobal::userXcfg().removePath(ds);
        CGlobal::userXft().removeDir(ds);
        CGlobal::userXcfg().refreshPaths();
        CGlobal::userXcfg().writeConfig();
        CFontmap::createTopLevel();
        if(CMisc::root())  // user cfg == sys icfg
            CGlobal::cfg().storeSysXConfigFileTs();
        CGlobal::userXft().apply();
    }
    KFI_DBUG << "deleted dir " << ds << endl;
}

void CKioFonts::addedDir(const QString &d, bool sys)
{
    QString ds(CMisc::dirSyntax(d));

    if(sys)
    {
        CGlobal::sysXcfg().readConfig();
        CGlobal::sysXft().reset();

        if(CGlobal::cfg().getSysXfs())
            doRootCmd("kfontinst refresh", false);
        else
        {
            if(!CMisc::root())  // Ensure user paths are at the front of the list!
                CGlobal::userXcfg().refreshPaths();
            CGlobal::sysXcfg().refreshPaths();
            doRootCmd("kfontinst createfontmap", false);
        }
    }
    else
    {
        CGlobal::userXcfg().addPath(ds);

#ifdef HAVE_FONTCONFIG
        CXConfig::configureDir(ds);
#else
        QStringList symFamilies;
        CXConfig::configureDir(ds, symFamilies);
#endif
        CFontmap::createLocal(ds);
        CGlobal::userXft().addDir(ds);

#ifdef HAVE_FONTCONFIG
        QStringList::ConstIterator xftIt;

        for(xftIt=CGlobal::cfg().getUserFontsDirs().begin(); xftIt!=CGlobal::cfg().getUserFontsDirs().end(); ++xftIt)
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(*xftIt));
#else
        if(symFamilies.count())
        {
            QStringList::Iterator it;

            for(it=symFamilies.begin(); it!=symFamilies.end(); ++it)
                CGlobal::userXft().addSymbolFamily(*it);
        }
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(ds));
#endif
        CGlobal::userXcfg().refreshPaths();
        CGlobal::userXcfg().writeConfig();
        CFontmap::createTopLevel();

        if(CMisc::root())  // user cfg == sys cfg
            CGlobal::cfg().storeSysXConfigFileTs();
        CGlobal::userXft().apply();
        CMisc::setTimeStamps(ds);
    }
    KFI_DBUG << "added dir " << ds << endl;
}

void CKioFonts::modifiedDir(const QString &d, bool sys)
{
    QString ds(CMisc::dirSyntax(d));

    if(sys)
    {
        if(!CGlobal::sysXcfg().inPath(ds) || !CGlobal::sysXft().hasDir(ds))
// CPD: FIXME????
// #ifdef HAVE_FONTCONFIG   // Ensure top-level is also in
//                || !CGlobal::sysXft().hasDir(CGlobal::cfg().getSysFontsDir())
// #endif
        {
            QCString cmd(CMisc::dExists(ds) ? "kfontinst adddir " : "kfontinst mkdir ");

            cmd+=QFile::encodeName(ds);

            if(doRootCmd(cmd))
            {
                addedDir(ds, true);
                CGlobal::cfg().storeSysXConfigFileTs();
            }
        }
        else
        {
            setTimeoutSpecialCommand(TIMEOUT);
            if(-1==itsModifiedSysDirs.findIndex(ds))
                itsModifiedSysDirs+=ds;
        }
    }
    else
    {
        if(!CGlobal::userXcfg().inPath(ds) || !CGlobal::userXft().hasDir(ds))
            addedDir(d);
        else
        {
            setTimeoutSpecialCommand(TIMEOUT);
            if(-1==itsModifiedDirs.findIndex(ds))
                itsModifiedDirs+=ds;
        }
    }
    KFI_DBUG << "modified dir " << ds << endl;
}

void CKioFonts::special(const QByteArray &)
{
    KFI_DBUG << "special" << endl;
    doModifiedDirs();
}

void CKioFonts::doModifiedDirs()
{
    KFI_DBUG << "doModifiedDirs" << endl;

    QStringList::Iterator it;

    itsNewFonts=0;

    if(itsModifiedSysDirs.count())
    {
        for(it=itsModifiedSysDirs.begin(); it!=itsModifiedSysDirs.end(); ++it)
        {
            QCString cmd("kfontinst cfgdir ");

            cmd+=QFile::encodeName(*it);
            doRootCmd(cmd, false);
        }

        if(CGlobal::cfg().getSysXfs())
            doRootCmd("kfontinst refresh", false);
        else
        {
            if(!CMisc::root())  // Ensure user paths are at the front of the list!
                CGlobal::userXcfg().refreshPaths();
            CGlobal::sysXcfg().refreshPaths();
            doRootCmd("kfontinst createfontmap", false);
        }

        itsModifiedSysDirs.clear();
    }

    if(itsModifiedDirs.count())
    {
        QStringList::ConstIterator constIt;

        for(it=itsModifiedDirs.begin(); it!=itsModifiedDirs.end(); ++it)
        {
            QString ds(CMisc::dirSyntax(*it));

#ifdef HAVE_FONTCONFIG
            CXConfig::configureDir(ds);
#else
            QStringList           x11SymFamilies;
            QStringList::Iterator xit;

            CXConfig::configureDir(ds, x11SymFamilies);
            for(xit=x11SymFamilies.begin(); xit!=x11SymFamilies.end(); ++xit)
                CGlobal::userXft().addSymbolFamily(*xit);
#endif
            CFontmap::createLocal(ds);
        }

        if(CGlobal::userXft().changed())
            CGlobal::userXft().apply();
        CFontmap::createTopLevel();
#ifdef HAVE_FONTCONFIG
        for(constIt=CGlobal::cfg().getUserFontsDirs().begin(); constIt!=CGlobal::cfg().getUserFontsDirs().end(); ++constIt)
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(*constIt));
#else
        for(it=itsModifiedDirs.begin(); it!=itsModifiedDirs.end(); ++it)
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(*it));
#endif
        for(it=itsModifiedDirs.begin(); it!=itsModifiedDirs.end(); ++it)
            CMisc::setTimeStamps(CMisc::dirSyntax(*it));

        for(constIt=CGlobal::cfg().getUserFontsDirs().begin(); constIt!=CGlobal::cfg().getUserFontsDirs().end(); ++constIt)
            CMisc::setTimeStamps(*constIt);

        itsModifiedDirs.clear();
        CGlobal::userXcfg().refreshPaths();
    }
    KFI_DBUG << "finished ModifiedDirs" << endl;
}

#define SYS_USER "root"
QString CKioFonts::getRootPasswd(bool askPasswd)
{
    KFI_DBUG << "getRootPasswd" << endl;
    KIO::AuthInfo authInfo;
    SuProcess     proc(SYS_USER);
    bool          error=false;
    int           attempts=0;
    QString       errorMsg;

    authInfo.url=KURL(KIO_FONTS_PROTOCOL ":///");
    authInfo.username=SYS_USER;
    authInfo.keepPassword=true;

    if(!checkCachedAuthentication(authInfo) && !askPasswd)
        authInfo.password=itsPasswd;

    if(askPasswd)
        while(!error && 0!=proc.checkInstall(authInfo.password.local8Bit()))
        {
            KFI_DBUG << "ATTEMPT : " << attempts << endl;
            if(1==attempts)
                errorMsg=i18n("Incorrect password.\n");
            if((!openPassDlg(authInfo, errorMsg) && attempts) || ++attempts>4 || SYS_USER!=authInfo.username)
                error=true;
        }
    else
        error=proc.checkInstall(authInfo.password.local8Bit()) ? true : false;
    return error ? QString::null : authInfo.password;
}

bool CKioFonts::doRootCmd(const char *cmd, const QString &passwd)
{
    KFI_DBUG << "doRootCmd " << cmd << endl;

    if(QString::null!=passwd)
    {
        SuProcess proc(SYS_USER);

        if(itsCanStorePasswd)
            itsPasswd=passwd;

        KFI_DBUG << "Try to run command" << endl;
        proc.setCommand(cmd);
        return proc.exec(passwd.local8Bit()) ? false : true;
    }

    return false;
}

bool CKioFonts::confirmUrl(KURL &url)
{
    KFI_DBUG << "confirmUrl " << url.path() << endl;
    if(!CMisc::root())
    {
        QString sect(CMisc::getSect(url.path()));

        if(i18n(KIO_FONTS_USER)!=sect && i18n(KIO_FONTS_SYS)!=sect)
        {
            bool changeToSystem=false;

            if(DEST_UNCHANGED!=itsLastDest && itsLastDestTime && (abs(time(NULL)-itsLastDestTime) < 5))
                changeToSystem=DEST_SYS==itsLastDest;
            else
                changeToSystem=KMessageBox::No==messageBox(QuestionYesNo, i18n("Do you wish to install the font into \"%1\" (in which case the "
                                                                               "font will only be usable by you), or \"%2\" (the font will be usable "
                                                                               "by all users - but you will need to know the Administrator's password) ?")
                                                                               .arg(KIO_FONTS_USER).arg(KIO_FONTS_SYS),
                                                           i18n("Where to install..."), i18n(KIO_FONTS_USER), i18n(KIO_FONTS_SYS));

            if(changeToSystem)
            {
                switch(CFontEngine::getType(QFile::encodeName(url.path())))
                {
                    case CFontEngine::TRUE_TYPE:
                    case CFontEngine::OPEN_TYPE:
                    case CFontEngine::TT_COLLECTION:
                        if(CGlobal::cfg().getSysTTSubDir().isEmpty())
                            url.setPath(QChar('/')+i18n(KIO_FONTS_SYS)+QChar('/')+CMisc::getFile(url.path()));
                        else
                            url.setPath(QChar('/')+i18n(KIO_FONTS_SYS)+QChar('/')+CGlobal::cfg().getSysTTSubDir()+CMisc::getFile(url.path()));
                        break;
                    case CFontEngine::TYPE_1:
                    case CFontEngine::TYPE_1_AFM:
                        if(CGlobal::cfg().getSysT1SubDir().isEmpty())
                            url.setPath(QChar('/')+i18n(KIO_FONTS_SYS)+QChar('/')+CMisc::getFile(url.path()));
                        else
                            url.setPath(QChar('/')+i18n(KIO_FONTS_SYS)+QChar('/')+CGlobal::cfg().getSysT1SubDir()+CMisc::getFile(url.path()));
                        break;
                    default:
                            url.setPath(QChar('/')+i18n(KIO_FONTS_SYS)+QChar('/')+CMisc::getFile(url.path()));
                }
                itsLastDest=DEST_SYS;
            }
            else
            {
                itsLastDest=DEST_USER;
                url.setPath(QChar('/')+i18n(KIO_FONTS_USER)+QChar('/')+CMisc::getFile(url.path()));
            }

            KFI_DBUG << "Changed URL to:" << url.path() << endl;
            return true;
        }
    }

    return false;
}

QString CKioFonts::convertUrl(const KURL &url, bool checkExists)
{
    if(CMisc::root())
    {
        if(!checkExists)
            return CGlobal::cfg().getUserFontsDirs().first()+CMisc::getSub(url.path());
        else
        {
            QStringList::ConstIterator it;
            QString                    sub(CMisc::getSub(url.path()));

            for(it=CGlobal::cfg().getUserFontsDirs().begin(); it!=CGlobal::cfg().getUserFontsDirs().end(); ++it)
                if(CMisc::fExists(*it+sub) || CMisc::dExists(*it+sub))
                    return *it+sub;
            return CGlobal::cfg().getUserFontsDirs().first()+sub;
        }
    }
    else
    {
        QString sect(CMisc::getSect(url.path()));

        if(i18n(KIO_FONTS_USER)!=sect && i18n(KIO_FONTS_SYS)!=sect)
            return url.path();
        else
        {
            const QStringList &list=i18n(KIO_FONTS_USER)==sect
                                         ? CGlobal::cfg().getUserFontsDirs()
                                         : CGlobal::cfg().getSysFontsDirs();
            if(!checkExists)
                return list.first()+CMisc::getSub(url.path());
            else
            {
                QStringList::ConstIterator it;
                QString                    sub(CMisc::getSub(url.path()));

                for(it=list.begin(); it!=list.end(); ++it)
                    if(CMisc::fExists(*it+sub) || CMisc::dExists(*it+sub))
                        return *it+sub;
                return list.first()+sub;
            }
        }
    }
}
