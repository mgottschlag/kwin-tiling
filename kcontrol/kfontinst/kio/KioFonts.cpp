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
#include <kdebug.h>
#include "FontEngine.h"
#include "Fontmap.h"
#include "XConfig.h"
#include "kxftconfig.h"
#include <kdesu/su.h>

#define MAX_IPC_SIZE  (1024*32)
#define TIMEOUT       2         // Time between last mod and writing files...
#define MAX_NEW_FONTS 20        // Number of fonts that can be installed before automatically configuring (related to timeout above)
#ifdef HAVE_FONTCONFIG
#define XFT_CACHE_CMD "fc-cache"
#else
#define XFT_CACHE_CMD "xftcache"
#endif

#define KDE_DBUG kdDebug(7124)

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

static bool isSpecialDir(const QString &sub, bool sys)
{
    if(sys || CMisc::root())
        return "CID"==sub || "encodings"==sub;
    else
        return "kde-override"==sub;
}

static void addAtom(KIO::UDSEntry &entry, unsigned int ID, long l, const QString &s=QString::null)
{
    KIO::UDSAtom atom;
    atom.m_uds = ID;
    atom.m_long = l;
    atom.m_str = s;
    entry.append(atom);
}

static bool createUDSEntry(KIO::UDSEntry &entry, const QString &name, const QString &path, const QString &mime)
{
    KDE_DBUG << "createUDSEntry " << name << ' ' << path << endl;

    KDE_struct_stat buff;

    entry.clear();
    if(-1!=KDE_lstat(QFile::encodeName(path), &buff ))
    {
        addAtom(entry, KIO::UDS_NAME, 0, name);

        if (S_ISLNK(buff.st_mode))
        {
            char buffer2[1000];
            int n=readlink(QFile::encodeName(path), buffer2, 1000);
            if(n!= -1)
                buffer2[n]='\0';

            addAtom(entry, KIO::UDS_LINK_DEST, 0, QString::fromLocal8Bit(buffer2));

            if(-1==KDE_stat(QFile::encodeName(path), &buff))
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
 
    return false;
}

#define createDirEntry(A, B, C, D) createUDSEntry(A, B, C, D ? KIO_FONTS_PROTOCOL "/system-folder" : KIO_FONTS_PROTOCOL "/folder")

static bool createFileEntry(KIO::UDSEntry &entry, const QString &fName, const QString &fPath)
{
    KDE_DBUG << "createFileEntry " << fName << endl;

    bool     err=false;
    QString  mime;
    QCString fn=QFile::encodeName(fName);

    switch(CFontEngine::getType(fn))
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
        case CFontEngine::BITMAP:
            if(CFontEngine::isABdf(fn))
                mime="application/x-font-bdf";
            else if(CFontEngine::isASnf(fn))
                mime="application/x-font-snf";
            else if(CFontEngine::isAPcf(fn))
                mime="application/x-font-pcf";
            else
                err=true;
            break;
        default:
            if(CFontEngine::isAAfm(fn))
                mime="application/x-afm";
            else
                err=true;
            break;
    }

    if(!err)
        err=!createUDSEntry(entry, fName, fPath, mime);

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
    error(KIO::ERR_SLAVE_DEFINED, i18n("Please specify \"%1\" or \"%2\".").arg(KIO_FONTS_USER).arg(KIO_FONTS_SYS)); \
    return; \
}

#define CHECK_URL_ROOT_OK(U) \
if("/"!=url.path() && !checkUrl(U)) \
{ \
    error(KIO::ERR_SLAVE_DEFINED, i18n("Please specify \"%1\" or \"%2\".").arg(KIO_FONTS_USER).arg(KIO_FONTS_SYS)); \
    return; \
}

#define CHECK_ALLOWED(u) \
if (u.path()==QString(QChar('/')+i18n(KIO_FONTS_USER)) || \
    u.path()==QString(QChar('/')+i18n(KIO_FONTS_SYS)) ) \
{ \
    error(KIO::ERR_SLAVE_DEFINED, i18n("Sorry, you cannot rename, move, or delete either \"%1\" or \"%2\".").arg(KIO_FONTS_USER).arg(KIO_FONTS_SYS)); \
    return; \
}

static void checkPath(const QCString path, bool &exists, bool &hidden)
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
}

CKioFonts::CKioFonts(const QCString &pool, const QCString &app)
         : KIO::SlaveBase(KIO_FONTS_PROTOCOL, pool, app),
           itsNewFonts(0)
{
    KDE_DBUG << "Constructor" << endl;
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
    KDE_DBUG << "Destructor" << endl;
    doModifiedDirs();
    CGlobal::destroy();
}

void CKioFonts::listDir(const KURL &url)
{
    KDE_DBUG << "listDir " << url.path() << endl;

    KIO::UDSEntry entry;
    int           size=0;

    if(CMisc::root())
    {
        size=getSize(CMisc::dirSyntax(CGlobal::cfg().getRealTopDir()+url.encodedPathAndQuery(-1)));
        totalSize(size);
        listDir(CGlobal::cfg().getRealTopDir(), url.encodedPathAndQuery(-1), false);
    }
    else if(QStringList::split('/', url.path(), false).count()==0)
    {
        size=2;
        totalSize(size);
        createDirEntry(entry, i18n(KIO_FONTS_USER), CGlobal::cfg().getRealTopDir(url.path()), false);
        listEntry(entry, false);
        createDirEntry(entry, i18n(KIO_FONTS_SYS), CGlobal::cfg().getRealTopDir(url.path()), true);
        listEntry(entry, false);
        addDir(CGlobal::cfg().getUserFontsDir());
        cfgDir(CGlobal::cfg().getUserFontsDir());
    }
    else
    {
        QString top(CGlobal::cfg().getRealTopDir(url.path()));

        if(!top.isNull())
        {
            size=getSize(CMisc::dirSyntax(top+CMisc::getSub(url.path())));
            totalSize(size);
            listDir(top, CMisc::getSub(url.path()), i18n(KIO_FONTS_SYS)==CMisc::getSect(url.path()));
        }
    }

    listEntry(size ? entry : KIO::UDSEntry(), true);
    finished();

    KDE_DBUG << "listDir - finished!" << endl;
}

int CKioFonts::getSize(const QString &ds)
{
    KDE_DBUG << "getSize " << ds << endl;

    QDir                dir(ds);
    const QFileInfoList *files=dir.entryInfoList(QDir::Dirs|QDir::Files|QDir::Hidden);

    int                 size=0;

    if(files)
    {
        QFileInfoListIterator it(*files);
        QFileInfo             *fInfo;

        for(; NULL!=(fInfo=it.current()); ++it)
            if("."!=fInfo->fileName() && ".."!=fInfo->fileName() &&
               (fInfo->isDir() || CFontEngine::isAFont(QFile::encodeName(fInfo->fileName()))
                               || CFontEngine::isAAfm(QFile::encodeName(fInfo->fileName()))) )
                    size++;
    }

    KDE_DBUG << "Size:" << size << endl;
    return size;
}

void CKioFonts::listDir(const QString &top, const QString &sub, bool sys)
{
    KDE_DBUG << "listDir " << top << ", " << sub << ", " << sys << endl;

    KIO::UDSEntry       entry;
    QString             dPath(CMisc::dirSyntax(top+sub)),
                        name(CMisc::getName(sub));
    QDir                dir(dPath);
    const QFileInfoList *files=dir.entryInfoList(QDir::Dirs|QDir::Files|QDir::Hidden);
    CXConfig            &xcfg=sys ? CGlobal::sysXcfg() : CGlobal::userXcfg();

    //
    // Ensure this dir is in fontpath - if it exists, and it contains fonts!
    if(sub.isNull() || (!name.isNull() && !sys && QChar('.')!=name[0] && !isSpecialDir(name, sys)))
    {
        addDir(dPath);
        cfgDir(dPath);
    }

    if(files)
    {
        QFileInfoListIterator it(*files);
        QFileInfo             *fInfo;

        for(; NULL!=(fInfo=it.current()); ++it)
            if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                if(fInfo->isDir())
                {
                    if(!isSpecialDir(fInfo->fileName(), sys))
                    {
                        QString ds(CMisc::dirSyntax(fInfo->filePath()));

                        if((QChar('.')==fInfo->fileName()[0] || (!sys && addDir(ds)) || xcfg.subInPath(ds)) &&
                           createDirEntry(entry, fInfo->fileName(), ds, sys && !CMisc::root()))
                        {
                            if(!sys && QChar('.')!=fInfo->fileName()[0])
                                cfgDir(ds);
                            listEntry(entry, false);
                        }
                    }
                }
                else if(( CFontEngine::isAFont(QFile::encodeName(fInfo->fileName())) || CFontEngine::isAAfm(QFile::encodeName(fInfo->fileName())) ) &&
                        createFileEntry(entry, fInfo->fileName(), fInfo->filePath()))
                    listEntry(entry, false);
    }
}

void CKioFonts::stat(const KURL &url)
{
    KDE_DBUG << "stat " << url.path() << endl;
    CHECK_URL_ROOT_OK(url)

    QStringList   path(QStringList::split('/', url.path(-1), false));
    KIO::UDSEntry entry;
    bool          err=false;

    KDE_DBUG << "count:" << path.count() << endl;
    switch(path.count())
    {
        case 0:
            err=!createDirEntry(entry, i18n("Fonts"), CGlobal::cfg().getRealTopDir(), false);
            break;
        case 1:
            if(CMisc::root())
                err=!createStatEntry(entry, url);
            else
                if(i18n(KIO_FONTS_USER)==path[0])
                    err=!createDirEntry(entry, i18n(KIO_FONTS_USER), CGlobal::cfg().getRealTopDir(url.path()), false);
                else if(path[0]==i18n(KIO_FONTS_SYS))
                    err=!createDirEntry(entry, i18n(KIO_FONTS_SYS), CGlobal::cfg().getRealTopDir(url.path()), true);
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
    KDE_DBUG << "createStatEntry " << url.path() << endl;

    QString top(CGlobal::cfg().getRealTopDir(url.path())),
            sub(CMisc::getSub(url.path())),
            ds(CMisc::dirSyntax(top+sub));

    QDir d(ds);

    if(d.exists())
    {
//        return (QChar('.')==sub[0] || CGlobal::userXcfg().inPath(ds) || CGlobal::sysXcfg().inPath(ds)) &&
//               createDirEntry(entry, CMisc::getName(url.path()), top+sub, !CMisc::root() && i18n(KIO_FONTS_SYS)==CMisc::getSect(url.path()));

        QString name(CMisc::getName(url.path()));

        if(!isSpecialDir(name, sys))
        {
            CXConfig xcfg=sys ? CGlobal::sysXcfg() : CGlobal::userXcfg();

            if((QChar('.')==name[0] || xcfg.inPath(ds) || (!sys && addDir(ds))) && createDirEntry(entry, name, ds, sys && !CMisc::root()))
            {
                if(!sys && QChar('.')!=name[0])
                    cfgDir(ds);
                return true;
            }
        }
    }
    else
    {
        QString fName(CMisc::getName(url.path()));

        return (CFontEngine::isAFont(QFile::encodeName(fName)) || CFontEngine::isAAfm(QFile::encodeName(fName))) &&
                createFileEntry(entry, fName, top+sub);
    }

    return false;
}

void CKioFonts::get(const KURL &url)
{
    KDE_DBUG << "get " << url.path() << endl;
    CHECK_URL(url)

    QCString realPath=QFile::encodeName(convertUrl(url));

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
            emit mimeType(KMimeType::findByURL(url.path(), buff.st_mode, true /* local URL */ )->name());

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
    KDE_DBUG << "put " << u.path() << endl;

    QString  destOrig(convertUrl(u));
    QCString destOrigC(QFile::encodeName(destOrig));

    // Check mime-type - only want to copy fonts, or AFM files...
    if(!(CFontEngine::isAFont(destOrigC) || CFontEngine::isA(destOrigC, "afm")))
    {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Only fonts may be installed."));
        return;
    }

    KURL url(u);
    if(!confirmUrl(url))
        return;

    destOrig=convertUrl(url);
    destOrigC=(QFile::encodeName(destOrig));

    KDE_struct_stat buffOrig;
    bool            origExists=(-1!=KDE_stat(destOrigC.data(), &buffOrig));

    if (origExists && !overwrite && !resume)
    {
        error(S_ISDIR(buffOrig.st_mode) ? KIO::ERR_DIR_ALREADY_EXIST : KIO::ERR_FILE_ALREADY_EXIST, destOrig);
        return;
    }

    bool otherExists,
         otherHidden;

    checkPath(destOrigC, otherExists, otherHidden);

    if(otherExists)
    {
        error(KIO::ERR_SLAVE_DEFINED, otherHidden ? i18n("A hidden/disabled font (starting with a '.') already exists.\n"
                                                         "Please rename/enable this.")
                                                  : i18n("A normal/enabled font (not starting with a '.') already exists.\n"
                                                         "Please rename/disable this."));
        return;
    }

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

            if(putReal(tmpFile.name(), tmpFileC, origExists, mode, resume))
            {
                QCString cmd("cp -f "); // "kfontinst install ");

                cmd+=tmpFileC;
                cmd+=" ";
                cmd+=destOrigC;

                cmd+="; chmod 0644 ";
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
    else if(putReal(destOrig, destOrigC, origExists, mode, resume))
        modifiedDir(CMisc::getDir(destOrig));

    if(++itsNewFonts>MAX_NEW_FONTS)
    {
        setTimeoutSpecialCommand(0); // Cancel timer
        doModifiedDirs();
    }

    finished();
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
    KDE_DBUG << "copy " << src.path() << " - " << d.path() << endl;
    CHECK_URL(src)

    QCString        realSrc=QFile::encodeName(convertUrl(src)),
                    realDest=QFile::encodeName(convertUrl(d));
    KDE_struct_stat buffSrc,
                    buffDest;

    KDE_DBUG << "REAL:" << realSrc << " TO REAL:" << realDest << endl;
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

    KURL dest(d);

    if(!confirmUrl(dest))
        return;

    realDest=QFile::encodeName(convertUrl(dest));

    if(-1!=KDE_stat(realDest.data(), &buffDest))
    {   
        if(S_ISDIR(buffDest.st_mode))
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
        QCString cmd("cp -f ");

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
}

void CKioFonts::rename(const KURL &src, const KURL &dest, bool overwrite)
{
    KDE_DBUG << "rename " << src.path() << " to " << dest.path() << endl;

    CHECK_URL(src)
    CHECK_ALLOWED(src)

    QCString srcPath(QFile::encodeName(convertUrl(src.path())));
    QCString destPath(QFile::encodeName(convertUrl(dest.path())));

    KDE_struct_stat buffSrc;

    if(-1==KDE_stat(srcPath.data(), &buffSrc))
    {
        error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST, src.path());
        return;
    }

    KDE_struct_stat buffDest;

    if(-1!=KDE_stat(destPath.data(), &buffDest))
    {
        if(S_ISDIR(buffDest.st_mode))
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

    bool nrs=false;

    if(nonRootSys(dest))
    {
        QCString cmd(S_ISDIR(buffSrc.st_mode) ? "kfontinst rename " : "mv -f ");
        
        cmd+=srcPath;
        cmd+=" ";
        cmd+=destPath;
        nrs=true;
            
        if(!doRootCmd(cmd))
        {
            error(KIO::ERR_CANNOT_RENAME, src.path());
            return;     
        }
    }
    else
        if(::rename(srcPath.data(), destPath.data()))
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

    if(S_ISDIR(buffSrc.st_mode))
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
    KDE_DBUG << "mkdir " << url.path() << endl;
    CHECK_URL(url)

    QCString        realPath=QFile::encodeName(convertUrl(url));
    bool            sys=nonRootSys(url);
    CXConfig        &xcfg=sys ? CGlobal::sysXcfg() : CGlobal::userXcfg();   // for root, syscfg==usercfg
    KDE_struct_stat buff;
    bool            exists=-1!=KDE_stat(realPath.data(), &buff),
                    otherExists,
                    otherHidden;

    if(isSpecialDir(CMisc::getName(url.path()), sys))
        error(KIO::ERR_SLAVE_DEFINED,
                  sys ? i18n("Sorry, you cannot create a folder named either \"CID\" or \"encodings\", as these are special "
                             "system folders. (\"CID\"is for \"CID\" fonts - these are <b>not</b> handled - and "
                             "\"encodings\" is for X11 encoding files.)")
                      : i18n("Sorry, you cannot create a folder named \"kde-override\", as this is a special KDE folder."));
    else
    {
        checkPath(realPath, otherExists, otherHidden);

        if(exists && xcfg.inPath(realPath))
            error(S_ISDIR(buff.st_mode) ? KIO::ERR_DIR_ALREADY_EXIST : KIO::ERR_FILE_ALREADY_EXIST, url.path());
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
                QCString cmd(exists ? "kfontinst adddir " : "kfontinst mkdir ");
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
                if (!exists && 0!=::mkdir(realPath.data(), 0777 /*umask will be applied*/ ))
                    error(EACCES==errno
                                  ? KIO::ERR_ACCESS_DENIED
                                  : ENOSPC==errno
                                        ? KIO::ERR_DISK_FULL
                                        : KIO::ERR_COULD_NOT_MKDIR, url.path() );
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
    KDE_DBUG << "chmod " << url.path() << endl;
    CHECK_URL(url)

    QCString realPath=QFile::encodeName(convertUrl(url));

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
    KDE_DBUG << "del " << url.path() << endl;
    CHECK_URL(url)
    CHECK_ALLOWED(url)

    QString  realPath=convertUrl(url);
    KDE_DBUG << "real " << realPath << endl;

    if(isFile)
    {
        QCString realPathC(QFile::encodeName(realPath));

        if(nonRootSys(url))
        {
            QCString cmd("rm -f "); // "kfontinst rmfont ");
            cmd+=realPathC;

            if(doRootCmd(cmd))
                modifiedDir(CMisc::getDir(realPath), true);
            else
                error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(KIO_FONTS_SYS));
        }
        else 
            if (0!=unlink(realPathC.data()))
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
                modifiedDir(CMisc::getDir(realPath));
            }
    }
    else
    {
        QCString realPathC=QFile::encodeName(realPath);

        if(nonRootSys(url))
        {
            QCString cmd("kfontinst rmdir ");
            cmd+=realPathC;
       
            if(doRootCmd(cmd))
            {
                CGlobal::cfg().storeSysXConfigFileTs();
                deletedDir(realPath, true);
            }
            else
                error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(KIO_FONTS_SYS));
        }
        else
        {
            // 1st may need to remove any fonts.dir, fonts.scale, encodings.dir, XftCache*,
            // fonts.cache*
            CMisc::removeAssociatedFiles(realPath, true);

            if(-1==::rmdir(realPathC.data()))
                if(EACCES==errno || EPERM==errno)
                    error(KIO::ERR_ACCESS_DENIED, url.path());
                else
                {
                    error(KIO::ERR_COULD_NOT_RMDIR, url.path());
                    return;
                }
            else
                deletedDir(realPath);
        }
    }

    finished();
}

//
// This is called from listDir() - and will add any non-kio_fonts created
// dirs to the fontpath (if they contain fonts *NOPE* - see below)
bool CKioFonts::addDir(const QString &ds)
{
    KDE_DBUG << "addDir " << ds << endl;

    if(!CGlobal::userXcfg().inPath(ds))  // Check it's not already in path though!!!
    {
#ifdef ADD_DIR_SHOULD_CHECK_FOR_PRESENCE_OF_FONTS
        QDir                dir(ds);
        const QFileInfoList *files=dir.entryInfoList(QDir::Files);

        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;

            for(; NULL!=(fInfo=it.current()); ++it)
                if(QChar('.')!=fInfo->filePath()[0] &&
                   ( CFontEngine::isAFont(QFile::encodeName(fInfo->fileName())) ||
                     CFontEngine::isA(QFile::encodeName(fInfo->fileName()), "afm") ) )
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
void CKioFonts::cfgDir(const QString &ds)
{
    KDE_DBUG << "cfgDir " << ds << endl;

    //
    // Check that it's not one that we're gonna update later, and that either fonts.dir does not exist, or
    // the fonts.dir is much older than the last mod time of the dir
    if(-1==itsModifiedDirs.findIndex(ds))    // Ensure its not one that we're gonna update later...
    {
        time_t dTs=CMisc::getTimeStamp(ds);
        bool   doTs=false;

        if (!CMisc::fExists(ds+"fonts.dir") || dTs!=CMisc::getTimeStamp(ds+"fonts.dir"))
        {
            infoMessage(i18n("Configuring out of date font folder."));

            KDE_DBUG << "configure out of date x dir" << dTs << " " << CMisc::getTimeStamp(ds+"fonts.dir") << endl;

#ifndef HAVE_FONTCONFIG
            QStringList symFamilies;

            CXConfig::configureDir(ds, symFamilies);
            if(symFamilies.count())
            {
                QStringList::Iterator it;
    
                for(it=symFamilies.begin(); it!=symFamilies.end(); ++it)
                    CGlobal::userXft().addSymbolFamily(*it);
            }
#else
            CXConfig::configureDir(ds);
#endif
            CGlobal::userXcfg().refreshPaths();
            if(CGlobal::userXft().changed())
                CGlobal::userXft().apply();
#ifdef HAVE_FONTCONFIG
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(CGlobal::cfg().getUserFontsDir()));
#else
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(ds));
#endif
            doTs=true;
        }

        if (!CMisc::fExists(ds+"Fontmap") || dTs!=CMisc::getTimeStamp(ds+"Fontmap"))
        {
            infoMessage(i18n("Configuring out of date font folder."));

            KDE_DBUG << "configure out of date fontmap" << dTs << " " << CMisc::getTimeStamp(ds+"Fontmap") << endl;

            CFontmap::createLocal(ds);
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
    CGlobal::userXcfg().addPath(CGlobal::cfg().getUserFontsDir());
    CGlobal::userXft().addDir(CGlobal::cfg().getUserFontsDir());

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
            cfgDir(*it);

    if(inX11NotXft.count())
    {
#ifdef HAVE_FONTCONFIG
        for(it=inX11NotXft.begin(); it!=inX11NotXft.end(); ++it)
            CGlobal::userXft().addDir(*it);
        CGlobal::userXft().apply();
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(CGlobal::cfg().getUserFontsDir()));
        for(it=inX11NotXft.begin(); it!=inX11NotXft.end(); ++it)
            CMisc::setTimeStamps(*it);
#else
        for(it=inX11NotXft.begin(); it!=inX11NotXft.end(); ++it)
            CGlobal::userXft().addDir(*it);
        CGlobal::userXft().apply();
        for(it=inX11NotXft.begin(); it!=inX11NotXft.end(); ++it)
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(*it));
        for(it=inX11NotXft.begin(); it!=inX11NotXft.end(); ++it)
            CMisc::setTimeStamps(*it);
#endif
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
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(CGlobal::cfg().getUserFontsDir()));
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
    KDE_DBUG << "deleted dir " << ds << endl;
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
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(CGlobal::cfg().getUserFontsDir()));
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
    KDE_DBUG << "added dir " << ds << endl;
}

void CKioFonts::modifiedDir(const QString &d, bool sys)
{
    QString ds(CMisc::dirSyntax(d));

    if(sys)
    {
        if(!CGlobal::sysXcfg().inPath(ds) || !CGlobal::sysXft().hasDir(ds)
#ifdef HAVE_FONTCONFIG   // Ensure toop-level is also in
               || !CGlobal::sysXft().hasDir(CGlobal::cfg().getSysFontsDir())
#endif
          )
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
    KDE_DBUG << "modified dir " << ds << endl;
}

void CKioFonts::special(const QByteArray &)
{
    KDE_DBUG << "special" << endl;
    doModifiedDirs();
}

void CKioFonts::doModifiedDirs()
{
    KDE_DBUG << "doModifiedDirs" << endl;

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
        CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(CGlobal::cfg().getUserFontsDir()));
#else
        for(it=itsModifiedDirs.begin(); it!=itsModifiedDirs.end(); ++it)
            CMisc::doCmd(XFT_CACHE_CMD, CMisc::xDirSyntax(*it));
#endif
        for(it=itsModifiedDirs.begin(); it!=itsModifiedDirs.end(); ++it)
            CMisc::setTimeStamps(CMisc::dirSyntax(*it));
        CMisc::setTimeStamps(CGlobal::cfg().getUserFontsDir());
        itsModifiedDirs.clear();
        CGlobal::userXcfg().refreshPaths();
    }
    KDE_DBUG << "finished ModifiedDirs" << endl;
}

#define SYS_USER "root"
QString CKioFonts::getRootPasswd(bool askPasswd)
{
    KDE_DBUG << "getRootPasswd" << endl;
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
            KDE_DBUG << "ATTEMPT : " << attempts << endl;
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
    KDE_DBUG << "doRootCmd " << cmd << endl;

    if(QString::null!=passwd)
    {
        SuProcess proc(SYS_USER);

        if(itsCanStorePasswd)
            itsPasswd=passwd;

        KDE_DBUG << "Try to run command" << endl;
        proc.setCommand(cmd);
        return proc.exec(passwd.local8Bit()) ? false : true;
    }

    return false;
}

bool CKioFonts::confirmUrl(KURL &url)
{
    KDE_DBUG << "confirmUrl " << url.path() << endl;
    if(!CMisc::root())
    {
        QString sect(CMisc::getSect(url.path()));

        if(i18n(KIO_FONTS_USER)!=sect && i18n(KIO_FONTS_SYS)!=sect)
        {
            // No = 2nd button = System
            if(KMessageBox::No==messageBox(QuestionYesNo, i18n("Do you wish to install the font into \"%1\" (in which case the "
                                                               "font will only be usable by you), or \"%2\" (the font will be usable "
                                                               "by all users - but you will need to know the Administrator's password) ?")
                                                              .arg(KIO_FONTS_USER).arg(KIO_FONTS_SYS),
                                           i18n("Where to install..."), i18n(KIO_FONTS_USER), i18n(KIO_FONTS_SYS)))
            {
                switch(CFontEngine::getType(QFile::encodeName(url.path())))
                {
                    case CFontEngine::TRUE_TYPE:
                    case CFontEngine::OPEN_TYPE:
                    case CFontEngine::TT_COLLECTION:
                        url.setPath(QChar('/')+i18n(KIO_FONTS_SYS)+QChar('/')+CGlobal::cfg().getSysTTSubDir()+
                                    CMisc::getFile(url.path()));
                        break;
                    case CFontEngine::TYPE_1:
                    case CFontEngine::TYPE_1_AFM:
                        url.setPath(QChar('/')+i18n(KIO_FONTS_SYS)+QChar('/')+CGlobal::cfg().getSysT1SubDir()+
                                    CMisc::getFile(url.path()));
                        break;
                    default:
                        error(KIO::ERR_SLAVE_DEFINED, i18n("Sorry, to install bitmap (.bdf, .pcf, .snf), or Speedo (.spd) "
                                                           "fonts\n in the \"%1\" folder you must specify a sub-folder.").arg(KIO_FONTS_SYS));
                        return false;
                }
            }
            else
                url.setPath(QChar('/')+i18n(KIO_FONTS_USER)+QChar('/')+CMisc::getFile(url.path()));

            KDE_DBUG << "Changed URL to:" << url.path() << endl;
        }
    }

    return true;
}

QString CKioFonts::convertUrl(const KURL &url)
{
    if(CMisc::root())
        return CGlobal::cfg().getRealTopDir(url.path())+CMisc::getSub(url.path());
    else
    {
        QString sect(CMisc::getSect(url.path()));

        if(i18n(KIO_FONTS_USER)!=sect && i18n(KIO_FONTS_SYS)!=sect)
            return url.path();
        else
            return CGlobal::cfg().getRealTopDir(url.path())+CMisc::getSub(url.path());
    }
}
