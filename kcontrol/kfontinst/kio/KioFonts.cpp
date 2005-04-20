////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CKioFonts
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
// (C) Craig Drummond, 2003, 2004
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
#include <qtextstream.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <qdir.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <kinstance.h>
#include <klargefile.h>
#include <ktempfile.h>
#include <kdesu/su.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kxftconfig.h>
#include <fontconfig/fontconfig.h>
#include "KfiConstants.h"
#include "FcEngine.h"
#include "Misc.h"
#include <X11/Xlib.h>
#include <ctype.h>

//#define KFI_FORCE_DEBUG_TO_STDERR

#ifdef KFI_FORCE_DEBUG_TO_STDERR

#include <qtextstream.h>
QTextOStream ostr(stderr);
#define KFI_DBUG ostr << "[" << (int)(getpid()) << "] "

#else

#define KFI_DBUG kdDebug() << "[" << (int)(getpid()) << "] "

#endif

#define MAX_IPC_SIZE   (1024*32)
#define TIMEOUT        2         // Time between last mod and writing files...
#define MAX_NEW_FONTS  20        // #fonts that can be installed before automatically configuring (related to above)
#define FC_CACHE_CMD  "fc-cache"

extern "C"
{
    KDE_EXPORT int kdemain(int argc, char **argv);
}

int kdemain(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: kio_" KFI_KIO_FONTS_PROTOCOL " protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    KLocale::setMainCatalogue(KFI_CATALOGUE);

    KInstance instance("kio_" KFI_KIO_FONTS_PROTOCOL);
    KFI::CKioFonts slave(argv[2], argv[3]);

    slave.dispatchLoop();

    return 0;
}

inline QString getSect(const QString &f) { return f.section('/', 1, 1); }

static QString modifyName(const QString &fname)
{
    static const char * constSymbols[]={ "-", " ", NULL };

    QString rv(fname);
    int     dotPos=rv.findRev('.');

    if(-1!=dotPos)
    {
        unsigned int rvLen=rv.length();

        for(unsigned int i=dotPos+1; i<rvLen; ++i)
            rv[i]=rv[i].lower();
    }

    for(int s=0; constSymbols[s]; ++s)
        rv=rv.replace(QString::fromLatin1(constSymbols[s]), QString::fromLatin1("_"));

    return rv;
}

static QString getFcString(FcPattern *pat, const char *val)
{
    FcChar8 *fcStr;

    return FcResultMatch==FcPatternGetString(pat, val, 0, &fcStr) ? (char *)fcStr : NULL;
}

static void addAtom(KIO::UDSEntry &entry, unsigned int ID, long l, const QString &s=QString::null)
{
    KIO::UDSAtom atom;
    atom.m_uds = ID;
    atom.m_long = l;
    atom.m_str = s;
    entry.append(atom);
}

static bool createUDSEntry(KIO::UDSEntry &entry, const QString &name, const QString &path, bool sys, bool folder=true)
{
    KFI_DBUG << "createUDSEntry " << name << ' ' << path << ' ' << sys << ' ' << folder << endl;
    KDE_struct_stat buff;
    QCString        cPath(QFile::encodeName(path));

    entry.clear();

    if(-1!=KDE_lstat(cPath, &buff))
    {
        addAtom(entry, KIO::UDS_NAME, 0, name);

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

        addAtom(entry, KIO::UDS_MIME_TYPE, 0, folder
                                                  ? sys
                                                        ? KFI_KIO_FONTS_PROTOCOL"/system-folder" 
                                                        : KFI_KIO_FONTS_PROTOCOL"/folder"
                                                  : KMimeType::findByPath(path, 0, true)->name());

        addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, "application/octet-stream");

        return true;
    }
    else if (folder && sys && !KFI::Misc::root())   // Default system fonts folder does not actually exist yet!
    {
        KFI_DBUG << "Default system folder (" << path << ") does not yet exist, so create dummy entry" << endl;
        addAtom(entry, KIO::UDS_NAME, 0, name);
        addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
        addAtom(entry, KIO::UDS_ACCESS, 0744);
        addAtom(entry, KIO::UDS_USER, 0, "root");
        addAtom(entry, KIO::UDS_GROUP, 0, "root");
        addAtom(entry, KIO::UDS_MIME_TYPE, 0, KFI_KIO_FONTS_PROTOCOL"/system-folder");
        addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, "application/octet-stream");

        return true;
    }


    return false;
}

static int getSize(const QCString &file)
{
    KDE_struct_stat buff;

    if(-1!=KDE_lstat(file, &buff))
    {
        if (S_ISLNK(buff.st_mode))
        {
            char buffer2[1000];
            int n=readlink(file, buffer2, 1000);
            if(n!= -1)
                buffer2[n]='\0';

            if(-1==KDE_stat(file, &buff))
                return -1;
        }
        return buff.st_size;
    }

    return -1;
}

static int getSize(QValueList<FcPattern *> &patterns)
{
    QValueList<FcPattern *>::Iterator it,
                                      end=patterns.end();
    int                               size=0;

    for(it=patterns.begin(); it!=end; ++it)
    {
        int s=getSize(QFile::encodeName(getFcString(*it, FC_FILE)));

        if(s>-1)
            size+=s;
    }

    return size;
}

static bool createFontUDSEntry(KIO::UDSEntry &entry, const QString &name, QValueList<FcPattern *> &patterns)
{
    KFI_DBUG << "createFontUDSEntry " << name << ' ' << patterns.count() << endl;
    if(1==patterns.count())
        return createUDSEntry(entry, name, getFcString(patterns.first(), FC_FILE), false, false);
    else  // This font name is associated with multiple font files - therefore, just return total file size!
    {
        QValueList<FcPattern *>::Iterator it,
                                          end(patterns.end());

        entry.clear();

        addAtom(entry, KIO::UDS_SIZE, getSize(patterns));

        for(it=patterns.begin(); it!=end; ++it)
        {
            QString         path(getFcString(*it, FC_FILE));
            QCString        cPath(QFile::encodeName(path));
            KDE_struct_stat buff;

            if(-1!=KDE_lstat(cPath, &buff))
            {
                addAtom(entry, KIO::UDS_NAME, 0, name);

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
                        goto notype;
                    }
                }

                addAtom(entry, KIO::UDS_FILE_TYPE, buff.st_mode&S_IFMT);
                addAtom(entry, KIO::UDS_ACCESS, buff.st_mode&07777);

                notype:
                addAtom(entry, KIO::UDS_MODIFICATION_TIME, buff.st_mtime);

                struct passwd *user = getpwuid(buff.st_uid);
                addAtom(entry, KIO::UDS_USER, 0, user ? user->pw_name : QString::number(buff.st_uid).latin1());

                struct group *grp = getgrgid(buff.st_gid);
                addAtom(entry, KIO::UDS_GROUP, 0, grp ? grp->gr_name : QString::number(buff.st_gid).latin1());

                addAtom(entry, KIO::UDS_ACCESS_TIME, buff.st_atime);
                addAtom(entry, KIO::UDS_MIME_TYPE, 0, KMimeType::findByPath(path, 0, true)->name());
                addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, "application/octet-stream");

                return true;  // This file was OK, so use its values...
            }
        }
    }
    return false;
}

enum EUrlStatus
{
    BAD_URL,
    URL_OK,
    REDIRECT_URL
};

static KURL getRedirect(const KURL &u)
{
    // Go from fonts:/System to fonts:/

    KURL    redirect(u);
    QString path(u.path()),
            sect(getSect(path));

    path.remove(sect);
    path.replace("//", "/");
    redirect.setPath(path);

    KFI_DBUG << "Redirect from " << u.path() << " to " << redirect.path() << endl;
    return redirect;
}

static bool nonRootSys(const KURL &u)
{
    return !KFI::Misc::root() && i18n(KFI_KIO_FONTS_SYS)==getSect(u.path());
}

static QString getFolder(const QString &defaultDir, const QString &root, QStringList &dirs)
{
    if(dirs.contains(defaultDir))
        return defaultDir;
    else
    {
        QStringList::Iterator it,
                              end=dirs.end();
        bool                  found=false;

        for(it=dirs.begin(); it!=end && !found; ++it)
            if(0==(*it).find(root))
                return *it;
    }

    return QString::null;
}

static void removeChar(char *str, char ch)
{
    unsigned int i=0,
                 len=strlen(str);

    for(i=0; i<len; ++i)
        if(str[i]==ch)
        {
            memmove(&str[i], &str[i+1], len-i);
            return;
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

static bool checkExt(const char *fname, const char *ext)
{
    unsigned int len=strlen(fname);

    return len>4 ? (fname[len-4]=='.' && tolower(fname[len-3])==ext[0] && tolower(fname[len-2])==ext[1] &&
                    tolower(fname[len-1])==ext[2])
                 : false;
}

static bool isAAfm(const QString &fname)
{
    if(checkExt(QFile::encodeName(fname), "afm"))   // CPD? Is this a necessary check?
    {
        QFile file(fname);

        if(file.open(IO_ReadOnly))
        {
            QTextStream stream(&file);
            QString     line;

            for(int lc=0; lc<30 && !stream.atEnd(); ++lc)
            {
                line=stream.readLine();

                if(line.contains("StartFontMetrics"))
                {
                    file.close();
                    return true;
                }
            }

            file.close();
        }
    }

    return false;
}

static bool isAPfm(const QString &fname)
{
    bool ok=false;

    // I know extension checking is bad, but Ghostscript's pf2afm requires the pfm file to
    // have the .pfm extension...
    if(checkExt(QFile::encodeName(fname), "pfm"))
    {
        //
        // OK, the extension matches, so perform a little contents checking...
        FILE *f=fopen(QFile::encodeName(fname).data(), "r");

        if(f)
        {
            static const unsigned long constCopyrightLen =  60;
            static const unsigned long constTypeToExt    =  49;
            static const unsigned long constExtToFname   =  20;
            static const unsigned long constExtLen       =  30;
            static const unsigned long constFontnameMin  =  75;
            static const unsigned long constFontnameMax  = 512;

            unsigned short version=0,
                           type=0,
                           extlen=0;
            unsigned long  length=0,
                           fontname=0,
                           fLength=0;

            fseek(f, 0, SEEK_END);
            fLength=ftell(f);
            fseek(f, 0, SEEK_SET);

            if(2==fread(&version, 1, 2, f) && // Read version
               4==fread(&length, 1, 4, f) &&  // length...
               length==fLength &&
               0==fseek(f, constCopyrightLen, SEEK_CUR) &&   // Skip copyright notice...
               2==fread(&type, 1, 2, f) &&
               0==fseek(f, constTypeToExt, SEEK_CUR) &&
               2==fread(&extlen, 1, 2, f) &&
               extlen==constExtLen &&
               0==fseek(f, constExtToFname, SEEK_CUR) &&
               4==fread(&fontname, 1, 4, f) &&
               fontname>constFontnameMin && fontname<constFontnameMax)
                ok=true;
            fclose(f);
        }
    }

    return ok;
}

//
// This function is *only* used for the generation of AFMs from PFMs.
static bool isAType1(const QString &fname)
{
    static const char *       constStr="%!PS-AdobeFont-";
    static const unsigned int constStrLen=15;
    static const unsigned int constPfbOffset=6;
    static const unsigned int constPfbLen=constStrLen+constPfbOffset;

    QCString name(QFile::encodeName(fname));
    char     buffer[constPfbLen];
    bool     match=false;

    if(checkExt(name, "pfa"))
    {
        FILE *f=fopen(name.data(), "r");

        if(f)
        {
            if(constStrLen==fread(buffer, 1, constStrLen, f))
                match=0==memcmp(buffer, constStr, constStrLen);
            fclose(f);
        }
    }
    else if(checkExt(name, "pfb"))
    {
        static const char constPfbMarker=0x80;

        FILE *f=fopen(name.data(), "r");

        if(f)
        {
            if(constPfbLen==fread(buffer, 1, constPfbLen, f))
                match=buffer[0]==constPfbMarker && 0==memcmp(&buffer[constPfbOffset], constStr, constStrLen);
            fclose(f);
        }
    }

    return match;
}

static QString getMatch(const QString &file, const char *extensions[])
{
    for(int e=0; extensions[e]; ++e)
    {
        QString f(KFI::Misc::changeExt(file, extensions[e]));

        if(KFI::Misc::fExists(f))
            return f;
    }

    return QString::null;
}

inline bool isHidden(const KURL &u)
{
    return QChar('.')==u.fileName()[0];
}

namespace KFI
{

CKioFonts::CKioFonts(const QCString &pool, const QCString &app)
         : KIO::SlaveBase(KFI_KIO_FONTS_PROTOCOL, pool, app),
           itsRoot(Misc::root()),
           itsFontChanges(0),
           itsLastDest(DEST_UNCHANGED),
           itsLastDestTime(0),
           itsFontList(NULL)
{
    KFI_DBUG << "Constructor" << endl;

    // Set core dump size to 0 because we will have
    // root's password in memory.
    struct rlimit rlim;
    rlim.rlim_cur=rlim.rlim_max=0;
    itsCanStorePasswd=setrlimit(RLIMIT_CORE, &rlim) ? false : true;

    //
    // Check with fontconfig for folder locations...
    //
    // 1. Get list of fontconfig dirs
    // 2. For user, look for any starting with $HOME - but prefer $HOME/.fonts
    // 3. For system, look for any starting with /usr/local/share - but prefer /usr/local/share/fonts
    // 4. If either are not found, then add to local.conf / .fonts.conf

    FcStrList   *list=FcConfigGetFontDirs(FcInitLoadConfigAndFonts());
    QStringList dirs;
    FcChar8     *dir;

    while((dir=FcStrListNext(list)))
        dirs.append(Misc::dirSyntax((const char *)dir));

    EFolder mainFolder=FOLDER_SYS;

    if(!itsRoot)
    {
        QString home(Misc::dirSyntax(QDir::homeDirPath())),
                defaultDir(Misc::dirSyntax(QDir::homeDirPath()+"/.fonts/")),
                dir(getFolder(defaultDir, home, dirs));

        if(dir.isEmpty())  // Then no $HOME/ was found in fontconfigs dirs!
        {
            KXftConfig xft(KXftConfig::Dirs, false);
            xft.addDir(defaultDir);
            xft.apply();
            dir=defaultDir;
        }
        mainFolder=FOLDER_USER;
        itsFolders[FOLDER_USER].location=dir;
    }

    QString sysDefault("/usr/local/share/fonts/"),
            sysDir(getFolder(sysDefault, "/usr/local/share/", dirs));
    bool    addToSysFc=false;

    if(sysDir.isEmpty())
    {
        if(itsRoot)
        {
            KXftConfig xft(KXftConfig::Dirs, true);
            xft.addDir(sysDefault);
            xft.apply();
        }
        else
            addToSysFc=true;

        sysDir=sysDefault;
    }

    itsFolders[FOLDER_SYS].location=sysDir;

    //
    // Ensure exists
    if(!Misc::dExists(itsFolders[mainFolder].location))
        Misc::createDir(itsFolders[mainFolder].location);

    //
    // Work out best params to send to kfontinst

    // ...determine if X already knows about the system font path...
    bool    usingFcFpe=false,
            usingXfsFpe=false,
            hasSys=false;
    Display *xDisplay=XOpenDisplay(NULL);

    if(xDisplay)
    {
        int  numPaths=0;
        char **paths=XGetFontPath(xDisplay, &numPaths);

        if(numPaths>0)
            for(int path=0; path<numPaths && !usingFcFpe; ++path)
                if(paths[path][0]=='/')
                {
                    if(Misc::dirSyntax(paths[path])==itsFolders[FOLDER_SYS].location)
                        hasSys=true;
                }
                else
                {
                    QString str(paths[path]);

                    str.replace(QRegExp("\\s*"), "");

                    if(0==str.find("unix/:"))
                        usingXfsFpe=true;
                    else if("fontconfig"==str)
                        usingFcFpe=true;
                }
        XFreeFontPath(paths);
        XCloseDisplay(xDisplay);
    }

    strcpy(itsKfiParams, "-g");
    if(itsRoot)
    {
        if(!usingFcFpe)
        {
            strcat(itsKfiParams, usingXfsFpe ? "s" : "r");
            strcat(itsKfiParams, "x");
            if(!hasSys)
                strcat(itsKfiParams, "a");
        }
    }
    else
    {
        strcpy(itsNrsKfiParams, addToSysFc ? "-gf" : "-g");
        if(!usingFcFpe)
        {
            strcat(itsNrsKfiParams, usingXfsFpe ? "sx" : "x");   // Can't get root to refresh X, only xfs!
            if(!hasSys)
                strcat(itsNrsKfiParams, "a");
            strcat(itsKfiParams, "xr");
        }
        memcpy(itsNrsNonMainKfiParams, itsNrsKfiParams, KFI_PARAMS);
        removeChar(itsNrsNonMainKfiParams, 'a');
        removeChar(itsNrsNonMainKfiParams, 'f');
    }
    reinitFc();
}

CKioFonts::~CKioFonts()
{
    KFI_DBUG << "Destructor" << endl;
    doModified();
}

void CKioFonts::listDir(const KURL &url)
{
    KFI_DBUG << "listDir " << url.path() << endl;

    if(updateFontList() && checkUrl(url, true))
    {
        KIO::UDSEntry entry;
        int           size=0;

        if(itsRoot || QStringList::split('/', url.path(), false).count()!=0)
        {
            EFolder folder=itsRoot || i18n(KFI_KIO_FONTS_SYS)==getSect(url.path()) ? FOLDER_SYS : FOLDER_USER;

            totalSize(itsFolders[folder].fontMap.count());
            if(itsFolders[folder].fontMap.count())
            {
                QMap<QString, QValueList<FcPattern *> >::Iterator it=itsFolders[folder].fontMap.begin(),
                                                                  end=itsFolders[folder].fontMap.end();

                for ( ; it != end; ++it)
                {
                    entry.clear();
                    createFontUDSEntry(entry, it.key(), it.data());
                    listEntry(entry, false);
                }
            }
        }
        else
        {
            size=2;
            totalSize(size);
            createUDSEntry(entry, i18n(KFI_KIO_FONTS_USER), itsFolders[FOLDER_USER].location, false);
            listEntry(entry, false);
            createUDSEntry(entry, i18n(KFI_KIO_FONTS_SYS), itsFolders[FOLDER_SYS].location, true);
            listEntry(entry, false);
        }

        listEntry(size ? entry : KIO::UDSEntry(), true);
        finished();
    }

    KFI_DBUG << "listDir - finished!" << endl;
}

void CKioFonts::stat(const KURL &url)
{
    KFI_DBUG << "stat " << url.prettyURL() << endl;

    if(updateFontList() && checkUrl(url, true))
    {
        QString path(url.path(-1));

        if(path.isEmpty())
        {
            error(KIO::ERR_COULD_NOT_STAT, url.prettyURL());
            return;
        }

        QStringList   pathList(QStringList::split('/', path, false));
        KIO::UDSEntry entry;
        bool          err=false;

        switch(pathList.count())
        {
            case 0:
                err=!createUDSEntry(entry, i18n("Fonts"), itsFolders[itsRoot ? FOLDER_SYS : FOLDER_USER].location, false);
                break;
            case 1:
                if(itsRoot)
                    err=!createStatEntry(entry, url, FOLDER_SYS);
                else
                    if(i18n(KFI_KIO_FONTS_USER)==pathList[0] || KFI_KIO_FONTS_USER==pathList[0])
                        err=!createUDSEntry(entry, i18n(KFI_KIO_FONTS_USER), itsFolders[FOLDER_USER].location, false);
                    else if(i18n(KFI_KIO_FONTS_SYS)==pathList[0] || KFI_KIO_FONTS_SYS==pathList[0])
                        err=!createUDSEntry(entry, i18n(KFI_KIO_FONTS_SYS), itsFolders[FOLDER_USER].location, true);
                    else
                    {
                        error(KIO::ERR_SLAVE_DEFINED,
                              i18n("Please specify \"%1\" or \"%2\".").arg(i18n(KFI_KIO_FONTS_USER)).arg(i18n(KFI_KIO_FONTS_SYS)));
                        return;
                    }
                break;
            default:
                err=!createStatEntry(entry, url, i18n(KFI_KIO_FONTS_SYS)==getSect(url.path()) ? FOLDER_SYS : FOLDER_USER);
        }

        if(err)
        {
            error(KIO::ERR_DOES_NOT_EXIST, url.prettyURL());
            return;
        }

        statEntry(entry);
        finished();
    }
}

bool CKioFonts::createStatEntry(KIO::UDSEntry &entry, const KURL &url, EFolder folder)
{
    KFI_DBUG << "createStatEntry " << url.path() << endl;

    QMap<QString, QValueList<FcPattern *> >::Iterator it=itsFolders[folder].fontMap.find(url.fileName());

    if(it!=itsFolders[folder].fontMap.end())
        return createFontUDSEntry(entry, it.key(), it.data());
    else
    {
        QString   name(url.fileName());
        FcPattern *pat=getEntry(folder, name);

        if(!pat) // OK, try the modified version of the name...
        {
            name=modifyName(name);
            pat=getEntry(folder, name);
        }

        if(pat)
            return createUDSEntry(entry, url.fileName(), getFcString(pat, FC_FILE), false, false);
    }
    return false;
}

//
// CPD: TODO? How to get a particular file in the case where font name matches multiple files?
//      e.g. bitmap fonts are supplied in multiple .pcf.gz files...
//
//      For the moment, just return 1st in list...
//
void CKioFonts::get(const KURL &url)
{
    KFI_DBUG << "get " << url.path() << " query:" << url.query() << endl;

    QValueList<FcPattern *> *entries;
    FcPattern               *entry;

    if(updateFontList() && checkUrl(url) && (entries=getEntries(url)) && (entry=entries->first()))
    {
        //
        // The thumbnail job always donwloads non-local files to /tmp/... and passes this file name to the thumbnail
        // creator. However, in the case of fonts which are split among many files, this wont work. Therefore, when the
        // thumbnail code asks for the font to donwload, just return the URL used. This way the font-thumbnail creator can
        // read this and just ask Xft/fontconfig for the font data.
        if(hasMetaData("thumbnail") && "1"==metaData("thumbnail"))
        {
            QByteArray   array;
            QTextOStream stream(array);

            emit mimeType("text/pain");

            KFI_DBUG << "hasMetaData(\"thumbnail\"), so return: " << url.prettyURL() << endl;

            stream << url.prettyURL();
            totalSize(array.size());
            data(array);
            processedSize(array.size());
            data(QByteArray());
            processedSize(array.size());
            finished();
            return;
        }

        QString         realPath(getFcString(entry, FC_FILE));
        QCString        realPathC(QFile::encodeName(realPath));
        KDE_struct_stat buff;
    
        KFI_DBUG << "real: " << realPathC << endl;
    
        if (-2==KDE_stat(realPathC.data(), &buff))
            error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST, url.prettyURL());
        else if (S_ISDIR(buff.st_mode))
            error(KIO::ERR_IS_DIRECTORY, url.prettyURL());
        else if (!S_ISREG(buff.st_mode))
            error(KIO::ERR_CANNOT_OPEN_FOR_READING, url.prettyURL());
        else
        {
            int fd = KDE_open(realPathC.data(), O_RDONLY);

            if (fd < 0)
                error(KIO::ERR_CANNOT_OPEN_FOR_READING, url.prettyURL());
            else
            {
                // Determine the mimetype of the file to be retrieved, and emit it.
                // This is mandatory in all slaves (for KRun/BrowserRun to work).
                emit mimeType(KMimeType::findByPath(realPathC, buff.st_mode, true)->name());
    
                totalSize(buff.st_size);
    
                KIO::filesize_t processed=0;
                char            buffer[MAX_IPC_SIZE];
                QByteArray      array;
    
                while(1)
                {
                    int n=::read(fd, buffer, MAX_IPC_SIZE);
                    if (-1==n && EINTR!=errno)
                    {
                        error(KIO::ERR_COULD_NOT_READ, url.prettyURL());
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
}

void CKioFonts::put(const KURL &u, int mode, bool overwrite, bool resume)
{
    KFI_DBUG << "put " << u.path() << endl;

    if(isHidden(u))
    {
        error(KIO::ERR_WRITE_ACCESS_DENIED, u.prettyURL());
        return;
    }

    updateFontList();

    //checkUrl(u) // CPD: Don't need to check URL, as the call to "confirmUrl()" below will sort out any probs!

    KURL            url(u);
    bool            changed=confirmUrl(url),
                    nrs=nonRootSys(url);
    EFolder         destFolder=itsRoot || i18n(KFI_KIO_FONTS_SYS)==getSect(url.path()) ? FOLDER_SYS : FOLDER_USER;
    QString         dest=itsFolders[destFolder].location+modifyName(url.fileName()),
                    passwd;
    QCString        destC=QFile::encodeName(dest);
    KDE_struct_stat buffDest;
    bool            destExists=(KDE_lstat(destC.data(), &buffDest)!= -1);

    if (destExists && !overwrite && !resume)
    {
        error(KIO::ERR_FILE_ALREADY_EXIST, url.prettyURL());
        return;
    }

    if(nrs) // Need to check can get root passwd before start download...
    { 
        passwd=getRootPasswd(); 
 
        if(passwd.isEmpty())
        {
            error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(i18n(KFI_KIO_FONTS_SYS)));
            return;
        }
    }

    //
    // As we don't get passed a mime-type the following needs to happen:
    //
    //    1. Download to a temporary file
    //    2. Check with FreeType that the file is a font, or that it is
    //       an AFM or PFM file
    //    3. If its OK, then get the fonts "name" from 
    KTempFile tmpFile;
    QCString  tmpFileC(QFile::encodeName(tmpFile.name()));

    tmpFile.setAutoDelete(true);

    if(putReal(tmpFile.name(), tmpFileC, destExists, mode, resume))
    {
        if(!checkFile(tmpFile.name()))  // error logged in checkFile
            return;
   
        if(nrs)  // Ask root to copy the font...
        {
            QCString cmd;

            if(!Misc::dExists(itsFolders[destFolder].location))
            {
                cmd+="mkdir ";
                cmd+=QFile::encodeName(KProcess::quote(itsFolders[destFolder].location));
                cmd+=" && chmod 0755 ";
                cmd+=QFile::encodeName(KProcess::quote(itsFolders[destFolder].location));
                cmd+=" && ";
            }
            cmd+="cp -f ";
            cmd+=QFile::encodeName(KProcess::quote(tmpFileC));
            cmd+=" ";
            cmd+=QFile::encodeName(KProcess::quote(destC));
            cmd+=" && chmod 0644 ";
            cmd+=destC;

            if(!itsCanStorePasswd)
                createRootRefreshCmd(cmd);

            // Get root to move this to fonts folder...
            if(doRootCmd(cmd, passwd))
            {
                modified(FOLDER_SYS);
                createAfm(dest, true, passwd);
            }
            else
            {
                error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(i18n(KFI_KIO_FONTS_SYS)));
                return;
            }
        }
        else // Move it to our font folder...
        {
            tmpFile.setAutoDelete(false);
            if(Misc::doCmd("mv", "-f", tmpFileC, destC))
            {
                ::chmod(destC.data(), Misc::FILE_PERMS);
                modified(FOLDER_USER);
                createAfm(dest);
            }
            else
            {
                error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(i18n(KFI_KIO_FONTS_USER)));
                return;
            }
        }

        finished();

        if(changed)
            itsLastDestTime=time(NULL);
    }
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
        {
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
    }
    while(result>0);

    if (result<0)
    {
        close(fd);
        if (-1==result)
           ::remove(destC.data());
        else if (markPartial)
        {
           KDE_struct_stat buff;

           if ((-1==KDE_stat(destC.data(), &buff)) || 
               (buff.st_size<config()->readNumEntry("MinimumKeepSize", DEFAULT_MINIMUM_KEEP_SIZE)))
               ::remove(destC.data());
        }
        ::exit(255);
    }

    if (-1==fd) // we got nothing to write out, so we never opened the file
    {
        finished();
        return false;
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

    return true;
}

void CKioFonts::copy(const KURL &src, const KURL &d, int mode, bool overwrite)
{
    //
    // Support:
    //    Copying to fonts:/
    //    Copying from fonts:/ and file:/
    //
    KFI_DBUG << "copy " << src.prettyURL() << " - " << d.prettyURL() << endl;

    if(isHidden(d))
    {
        error(KIO::ERR_WRITE_ACCESS_DENIED, d.prettyURL());
        return;
    }

    if(updateFontList() && checkUrl(src) && checkAllowed(src))
    {
        //checkUrl(u) // CPD as per comment in ::put()

        QStringList srcFiles;

        if(getSourceFiles(src, srcFiles))  // Any error will be logged in getSourceFiles
        {
            KURL    dest(d);
            bool    changed=confirmUrl(dest);
            EFolder destFolder=itsRoot || i18n(KFI_KIO_FONTS_SYS)==getSect(dest.path()) ? FOLDER_SYS : FOLDER_USER;

            if(confirmMultiple(src, srcFiles, FOLDER_SYS==destFolder ? FOLDER_USER : FOLDER_SYS, OP_COPY) &&
               checkDestFiles(src, srcFiles, dest, destFolder, overwrite))
            {
                if(nonRootSys(dest))
                {
                    QCString              cmd;
                    QStringList::Iterator it,
                                          end=srcFiles.end();
                    int                   size=0;

                    if(!Misc::dExists(itsFolders[destFolder].location))
                    {
                        cmd+="mkdir ";
                        cmd+=QFile::encodeName(KProcess::quote(itsFolders[destFolder].location));
                        cmd+=" && chmod 0755 ";
                        cmd+=QFile::encodeName(KProcess::quote(itsFolders[destFolder].location));
                        cmd+=" && ";
                    }
                    cmd+="cp -f ";

                    for(it=srcFiles.begin(); it!=end; ++it) 
                    {
                        cmd+=QFile::encodeName(KProcess::quote(modifyName(*it)));
                        cmd+=" ";
                        int s=getSize(QFile::encodeName(*it));
                        if(s>0)
                            size+=s;
                    }
                    cmd+=QFile::encodeName(KProcess::quote(itsFolders[destFolder].location));
    
                    if(!itsCanStorePasswd)
                        createRootRefreshCmd(cmd);
    
                    totalSize(size);
  
                    QString passwd=getRootPasswd();

                    if(doRootCmd(cmd, passwd))
                    {
                        modified(destFolder);
                        processedSize(size);
                        if(src.isLocalFile() && 1==srcFiles.count())
                            createAfm(itsFolders[destFolder].location+Misc::getFile(*(srcFiles.begin())), true, passwd);
                    }
                    else
                    {
                        error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(i18n(KFI_KIO_FONTS_SYS)));
                        return;
                    }
                }
                else
                {
                    QStringList::Iterator it,
                                          end=srcFiles.end();

                    for(it=srcFiles.begin(); it!=end; ++it)
                    {
                        QCString        realSrc(QFile::encodeName(*it)),
                                        realDest(QFile::encodeName(itsFolders[destFolder].location+modifyName(Misc::getFile(*it))));
                        KDE_struct_stat buffSrc;

                        if(-1==KDE_stat(realSrc.data(), &buffSrc))
                        {
                            error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST, src.prettyURL());
                            return;
                        }

                        int srcFd=KDE_open(realSrc.data(), O_RDONLY);
    
                        if (srcFd<0)
                        {
                            error(KIO::ERR_CANNOT_OPEN_FOR_READING, src.prettyURL());
                            return;
                        }
    
                        if(!Misc::dExists(itsFolders[destFolder].location))
                            Misc::createDir(itsFolders[destFolder].location);
    
                        // WABA: Make sure that we keep writing permissions ourselves,
                        // otherwise we can be in for a surprise on NFS.
                        int destFd=KDE_open(realDest.data(), O_CREAT | O_TRUNC | O_WRONLY, -1==mode ? 0666 : mode | S_IWUSR);
    
                        if (destFd<0)
                        {
                            error(EACCES==errno ? KIO::ERR_WRITE_ACCESS_DENIED : KIO::ERR_CANNOT_OPEN_FOR_WRITING, dest.prettyURL());
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
                                error(KIO::ERR_COULD_NOT_READ, src.prettyURL());
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
                                    error(KIO::ERR_DISK_FULL, dest.prettyURL());
                                    remove(realDest.data());
                                }
                                else
                                    error(KIO::ERR_COULD_NOT_WRITE, dest.prettyURL());
                                return;
                            }
    
                            processed += n;
                            processedSize(processed);
                        }
    
                        close(srcFd);
    
                        if(close(destFd))
                        {
                            error(KIO::ERR_COULD_NOT_WRITE, dest.prettyURL());
                            return;
                        }
    
                        ::chmod(realDest.data(), Misc::FILE_PERMS);
    
                        // copy access and modification time
                        struct utimbuf ut;
    
                        ut.actime = buffSrc.st_atime;
                        ut.modtime = buffSrc.st_mtime;
                        ::utime(realDest.data(), &ut);
    
                        processedSize(buffSrc.st_size);
                        modified(destFolder);
                    }

                    if(src.isLocalFile() && 1==srcFiles.count())
                        createAfm(itsFolders[destFolder].location+Misc::getFile(*(srcFiles.begin())));
                }
    
                finished();
    
                if(changed)
                    itsLastDestTime=time(NULL);
            }
        }
    }
}

void CKioFonts::rename(const KURL &src, const KURL &d, bool overwrite)
{
    if(src.directory()==d.directory())
        error(KIO::ERR_SLAVE_DEFINED, i18n("Sorry, fonts cannot be renamed."));
    else if(itsRoot) // Should never happen...
        error(KIO::ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, KIO::CMD_RENAME));
    else
    {
        //
        // Can't rename from/to file:/ -> therefore rename can only be from fonts:/System to fonts:/Personal,
        // or vice versa.

        QStringList srcFiles;

        if(getSourceFiles(src, srcFiles))   // Any error will be logged in getSourceFiles
        {
            KURL    dest(d);
            bool    changed=confirmUrl(dest);
            EFolder destFolder=i18n(KFI_KIO_FONTS_SYS)==getSect(dest.path()) ? FOLDER_SYS : FOLDER_USER;

            if(confirmMultiple(src, srcFiles, FOLDER_SYS==destFolder ? FOLDER_USER : FOLDER_SYS, OP_MOVE) &&
               checkDestFiles(src, srcFiles, dest, destFolder, overwrite))
            {
                QStringList::Iterator it,
                                      end=srcFiles.end();
                bool                  askPasswd=true,
                                      toSys=FOLDER_SYS==destFolder;
                QCString              userId,
                                      groupId,
                                      destDir(QFile::encodeName(KProcess::quote(itsFolders[destFolder].location)));

                userId.setNum(toSys ? 0 : getuid());
                groupId.setNum(toSys ? 0 : getgid());

                for(it=srcFiles.begin(); it!=end; ++it)
                {
                    QCString cmd,
                             destFile(QFile::encodeName(KProcess::quote(itsFolders[destFolder].location+
                                                        Misc::getFile(*it))));

                    if(toSys && !Misc::dExists(itsFolders[destFolder].location))
                    {
                        cmd+="mkdir ";
                        cmd+=destDir;
                        cmd+=" && ";
                    }

                    cmd+="mv -f ";
                    cmd+=QFile::encodeName(KProcess::quote(*it));
                    cmd+=" ";
                    cmd+=destDir;
                    cmd+=" && chmod -f 0644 ";
                    cmd+=destFile;
                    cmd+=" && chown -f ";
                    cmd+=userId;
                    cmd+=":";
                    cmd+=groupId;
                    cmd+=" ";
                    cmd+=destFile;

                    QString sysDir,
                            userDir;

                    if(FOLDER_SYS==destFolder)
                    {
                        sysDir=itsFolders[destFolder].location;
                        userDir=Misc::getDir(*it);
                    }
                    else
                    {
                        userDir=itsFolders[destFolder].location;
                        sysDir=Misc::getDir(*it);
                    }

                    if(!itsCanStorePasswd)
                        createRootRefreshCmd(cmd, sysDir);

                    if(doRootCmd(cmd, askPasswd))
                    {
                        modified(FOLDER_SYS, sysDir);
                        modified(FOLDER_USER, userDir);
                        askPasswd=false;  // Don't keep on asking for password...
                    }
                    else
                    {
                        error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(i18n(KFI_KIO_FONTS_SYS)));
                        return;
                    }
                }
                if(changed)
                    itsLastDestTime=time(NULL);
            }
        }
    }
}

void CKioFonts::del(const KURL &url, bool)
{
    KFI_DBUG << "del " << url.path() << endl;

    QValueList<FcPattern *> *entries;

    if(updateFontList() && checkUrl(url) && checkAllowed(url) && (entries=getEntries(url)) && entries->count() &&
       confirmMultiple(url, entries, itsRoot || i18n(KFI_KIO_FONTS_SYS)==getSect(url.path()) ? FOLDER_SYS : FOLDER_USER,
                       OP_DELETE))
    {
        QValueList<FcPattern *>::Iterator it,
                                          end=entries->end();
        CDirList                          modifiedDirs;

        if(nonRootSys(url))
        {
            QCString cmd("rm -f");
 
            for(it=entries->begin(); it!=end; ++it)
            {
                QString file(getFcString(*it, FC_FILE));

                modifiedDirs.add(Misc::getDir(file));
                cmd+=" ";
                cmd+=QFile::encodeName(KProcess::quote(file));

                KURL::List urls;

                Misc::getAssociatedUrls(KURL(file), urls);

                if(urls.count())
                {
                    KURL::List::Iterator uIt,
                                         uEnd=urls.end();

                    for(uIt=urls.begin(); uIt!=uEnd; ++uIt)
                    {
                        cmd+=" ";
                        cmd+=QFile::encodeName(KProcess::quote((*uIt).path()));
                    }
                }
            }
    
            if(!itsCanStorePasswd)
                createRootRefreshCmd(cmd, modifiedDirs);
    
            if(doRootCmd(cmd))
                modified(FOLDER_SYS, modifiedDirs);
            else
                error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\" folder.").arg(i18n(KFI_KIO_FONTS_SYS)));
        }
        else
        {
            for(it=entries->begin(); it!=end; ++it)
            {
                QString file(getFcString(*it, FC_FILE));

                if (0!=unlink(QFile::encodeName(file).data()))
                    error(EACCES==errno || EPERM==errno
                            ? KIO::ERR_ACCESS_DENIED
                            : EISDIR==errno
                                    ? KIO::ERR_IS_DIRECTORY
                                    : KIO::ERR_CANNOT_DELETE,
                          file);
                else
                {
                    modifiedDirs.add(Misc::getDir(file));

                    KURL::List urls;

                    Misc::getAssociatedUrls(KURL(file), urls);

                    if(urls.count())
                    {
                        KURL::List::Iterator uIt,
                                             uEnd=urls.end();

                        for(uIt=urls.begin(); uIt!=uEnd; ++uIt)
                            unlink(QFile::encodeName((*uIt).path()).data());
                    }
                }
            }
            modified(itsRoot ? FOLDER_SYS : FOLDER_USER, modifiedDirs);
        }
        finished();
    }
}

void CKioFonts::modified(EFolder folder, const CDirList &dirs)
{
    KFI_DBUG << "modified(" << (int)folder << ")\n";

    if(FOLDER_SYS!=folder || itsCanStorePasswd || itsRoot)
    {
        if(dirs.count())
        {
            CDirList::ConstIterator it(dirs.begin()),
                                    end(dirs.end());

            for(; it!=end; ++it)
                itsFolders[folder].modified.add(*it);
        }
        else
            itsFolders[folder].modified.add(itsFolders[folder].location);

        //reinitFc();

        if(++itsFontChanges>MAX_NEW_FONTS)
        {
            setTimeoutSpecialCommand(0); // Cancel timer
            doModified();
        } 
        else
            setTimeoutSpecialCommand(TIMEOUT);
    }

    //
    // Remove any kfontinst params possible...
    if(FOLDER_SYS==folder && !itsRoot && !itsCanStorePasswd)
    {
        // If we modified sys, we're not root, and  couldn't store the passwd, then kfontinst has already been called
        // so no need to ask it to add folder to fontconfig and X's config files...
        removeChar(itsNrsKfiParams, 'f');
        removeChar(itsNrsKfiParams, 'a');
    }

    reinitFc();
}

void CKioFonts::special(const QByteArray &)
{
    KFI_DBUG << "special" << endl;
    doModified();
}

void CKioFonts::createRootRefreshCmd(QCString &cmd, const CDirList &dirs)
{
    if(!cmd.isEmpty())
        cmd+=" && ";

    cmd+=FC_CACHE_CMD;

    if(dirs.count())
    {
        CDirList::ConstIterator it(dirs.begin()),
                                end(dirs.end());

        for(; it!=end; ++it)
        {
            cmd+=" && kfontinst ";
            if(*it==itsFolders[FOLDER_SYS].location)
                cmd+=itsNrsKfiParams;
            else
                cmd+=itsNrsNonMainKfiParams;
            cmd+=" ";
            cmd+=QFile::encodeName(KProcess::quote(*it));
        }
    }
    else
    {
        cmd+=" && kfontinst ";
        cmd+=itsNrsKfiParams;
        cmd+=" ";
        cmd+=QFile::encodeName(KProcess::quote(itsFolders[FOLDER_SYS].location));
    }
}

void CKioFonts::doModified()
{
    KFI_DBUG << "doModified" << endl;

    itsFontChanges=0;
    if(itsFolders[FOLDER_SYS].modified.count())
    {
        if(itsRoot)
        {
            Misc::doCmd(FC_CACHE_CMD);
            KFI_DBUG << "RUN(root): " << FC_CACHE_CMD << endl;

            CDirList::ConstIterator it(itsFolders[FOLDER_SYS].modified.begin()),
                                    end(itsFolders[FOLDER_SYS].modified.end());

            for(; it!=end; ++it)
            {
                Misc::doCmd("kfontinst", itsKfiParams, QFile::encodeName(*it));
                KFI_DBUG << "RUN(root): kfontinst " << itsKfiParams << ' ' << *it << endl;
            }

            if(itsFolders[FOLDER_SYS].modified.contains(itsFolders[FOLDER_SYS].location))
            {
                removeChar(itsKfiParams, 'a');
                removeChar(itsKfiParams, 'f');
            }
        }
        else
        {
            QCString cmd;

            createRootRefreshCmd(cmd, itsFolders[FOLDER_SYS].modified);
            if(doRootCmd(cmd, false) && itsFolders[FOLDER_SYS].modified.contains(itsFolders[FOLDER_SYS].location))
            {
                removeChar(itsNrsKfiParams, 'f');
                removeChar(itsNrsKfiParams, 'a');
            }
            Misc::doCmd("xset", "fp", "rehash");  // doRootCmd can only refresh if xfs is being used, so try here anyway...
        }
        itsFolders[FOLDER_SYS].modified.clear();
    }

    if(!itsRoot && itsFolders[FOLDER_USER].modified.count())
    {
        Misc::doCmd(FC_CACHE_CMD);
        KFI_DBUG << "RUN(non-root): " << FC_CACHE_CMD << endl;

        CDirList::ConstIterator it(itsFolders[FOLDER_USER].modified.begin()),
                                end(itsFolders[FOLDER_USER].modified.end());

        for(; it!=end; ++it)
        {
            Misc::doCmd("kfontinst", itsKfiParams, QFile::encodeName(*it));
            KFI_DBUG << "RUN(non-root): kfontinst " << itsKfiParams << ' ' << *it << endl;
        }
        itsFolders[FOLDER_USER].modified.clear();
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

    authInfo.url=KURL(KFI_KIO_FONTS_PROTOCOL ":///");
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

    if(!passwd.isEmpty())
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
    if(!itsRoot)
    {
        QString sect(getSect(url.path()));

        if(i18n(KFI_KIO_FONTS_USER)!=sect && i18n(KFI_KIO_FONTS_SYS)!=sect && 
           KFI_KIO_FONTS_USER!=sect && KFI_KIO_FONTS_SYS!=sect)
        {
            bool changeToSystem=false;

            if(DEST_UNCHANGED!=itsLastDest && itsLastDestTime && (abs(time(NULL)-itsLastDestTime) < 5))
                changeToSystem=DEST_SYS==itsLastDest;
            else
                changeToSystem=KMessageBox::No==messageBox(QuestionYesNo,
                                                           i18n("Do you wish to install the font into \"%1\" (in which "
                                                                "case the font will only be usable by you), or \"%2\" ("
                                                                "the font will be usable by all users - but you will "
                                                                "need to know the administrator's password)?")
                                                               .arg(i18n(KFI_KIO_FONTS_USER)).arg(i18n(KFI_KIO_FONTS_SYS)),
                                                           i18n("Where to Install"), i18n(KFI_KIO_FONTS_USER),
                                                           i18n(KFI_KIO_FONTS_SYS));

            if(changeToSystem)
            {
                itsLastDest=DEST_SYS;
                url.setPath(QChar('/')+i18n(KFI_KIO_FONTS_SYS)+QChar('/')+url.fileName());
            }
            else
            {
                itsLastDest=DEST_USER;
                url.setPath(QChar('/')+i18n(KFI_KIO_FONTS_USER)+QChar('/')+url.fileName());
            }

            KFI_DBUG << "Changed URL to:" << url.path() << endl;
            return true;
        }
    }

    return false;
}

void CKioFonts::reinitFc()
{
    KFI_DBUG << "reinitFc()" << endl;

    FcInitReinitialize();
    if(itsFontList)
        FcFontSetDestroy(itsFontList);
    itsFontList=NULL;
    itsFolders[FOLDER_SYS].fontMap.clear();
    itsFolders[FOLDER_USER].fontMap.clear();
}

bool CKioFonts::updateFontList()
{
    KFI_DBUG << "updateFontList" << endl;

//    if (itsFontList && !FcConfigUptoDate(0))
//        reinitFc();

    if(!itsFontList)
    {
        FcPattern   *pat = FcPatternCreate();
        FcObjectSet *os  = FcObjectSetBuild(FC_FILE, FC_FAMILY, FC_WEIGHT,
#ifdef KFI_FC_HAS_WIDTHS
                                            FC_WIDTH,
#endif
                                            FC_SLANT, (void*)0);

        itsFontList=FcFontList(0, pat, os);

        FcPatternDestroy(pat);
        FcObjectSetDestroy(os);

        if (itsFontList)
        {
            QString home(Misc::dirSyntax(QDir::homeDirPath()));

            for (int i = 0; i < itsFontList->nfont; i++)
            {
                EFolder folder=FOLDER_SYS;
                QString file(Misc::fileSyntax(getFcString(itsFontList->fonts[i], FC_FILE)));

                if(!file.isEmpty())
                {
                    if(!itsRoot && 0==file.find(home))
                        folder=FOLDER_USER;

                    QValueList<FcPattern *> &patterns=
                                                itsFolders[folder].fontMap[CFcEngine::createName(itsFontList->fonts[i])];
                    bool                    use=true;

                    if(patterns.count()) // Check for duplicates...
                    {
                        QValueList<FcPattern *>::Iterator it,
                                                          end=patterns.end();

                        for(it=patterns.begin(); use && it!=end; ++it)
                            if(file==(Misc::fileSyntax(getFcString(*it, FC_FILE))))
                                use=false;
                    }
                    if(use)
                        patterns.append(itsFontList->fonts[i]);
                }
            }
        }
    }

    if(NULL==itsFontList)
    {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Internal fontconfig error."));
        return false;
    }

    return true;
}

QValueList<FcPattern *> * CKioFonts::getEntries(const KURL &url)
{
    EFolder folder=itsRoot || i18n(KFI_KIO_FONTS_SYS)==getSect(url.path()) ? FOLDER_SYS : FOLDER_USER;

    QMap<QString, QValueList<FcPattern *> >::Iterator it=itsFolders[folder].fontMap.find(url.fileName());

    if(it!=itsFolders[folder].fontMap.end())
        return &(it.data());

    error(KIO::ERR_SLAVE_DEFINED, i18n("Could not access \"%1\".").arg(url.prettyURL()));
    return NULL;
}

FcPattern * CKioFonts::getEntry(EFolder folder, const QString &file, bool full)
{
    QMap<QString, QValueList<FcPattern *> >::Iterator it,
                                                      end=itsFolders[folder].fontMap.end();

    for(it=itsFolders[folder].fontMap.begin(); it!=end; ++it)
    {
        QValueList<FcPattern *>::Iterator patIt,
                                          patEnd=it.data().end();

        for(patIt=it.data().begin(); patIt!=patEnd; ++patIt)
            if( (full && getFcString(*patIt, FC_FILE)==file) ||
                (!full && Misc::getFile(getFcString(*patIt, FC_FILE))==file))
                return *patIt;
    }

    return NULL;
}

bool CKioFonts::checkFile(const QString &file)
{
    int       count=0;
    FcPattern *pat=FcFreeTypeQuery((const FcChar8 *)(QFile::encodeName(file).data()), 0, NULL, &count);

    if(!pat && !isAAfm(file) && !isAPfm(file))
    {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Only fonts may be installed."));
        return false;
    }

    if(pat)
        FcPatternDestroy(pat);

    return true;
}

bool CKioFonts::getSourceFiles(const KURL &src, QStringList &files)
{
    if(KFI_KIO_FONTS_PROTOCOL==src.protocol())
    {
        QValueList<FcPattern *> *entries=getEntries(src);

        if(entries && entries->count())
        {
            QValueList<FcPattern *>::Iterator it,
                                              end=entries->end();

            for(it=entries->begin(); it!=end; ++it)
                files.append(getFcString(*it, FC_FILE));
        }

        if(files.count())
        {
            QStringList::Iterator sIt,
                                  sEnd=files.end();

            for(sIt=files.begin(); sIt!=files.end(); ++sIt)
            {
                KURL::List urls;

                Misc::getAssociatedUrls(KURL(*sIt), urls);

                if(urls.count())
                {
                    KURL::List::Iterator uIt,
                                         uEnd=urls.end();

                    for(uIt=urls.begin(); uIt!=uEnd; ++uIt)
                        if(-1==files.findIndex((*uIt).path()))
                            files.append((*uIt).path());
                }
           }
        }
    }
    else
        if(src.isLocalFile())
            if(checkFile(src.path()))
                files.append(src.path());
            else
                return false;  // error logged in checkFile...

    if(files.count())
    {
        QStringList::Iterator it,
                              end=files.end();

        for(it=files.begin(); it!=end; ++it)
        {
            QCString        realSrc=QFile::encodeName(*it);
            KDE_struct_stat buffSrc;

            if (-1==KDE_stat(realSrc.data(), &buffSrc))
            {
                error(EACCES==errno ? KIO::ERR_ACCESS_DENIED : KIO::ERR_DOES_NOT_EXIST, src.prettyURL());
                return false;
            }
            if(S_ISDIR(buffSrc.st_mode))
            {
                error(KIO::ERR_IS_DIRECTORY, src.prettyURL());
                return false;
            }
            if(S_ISFIFO(buffSrc.st_mode) || S_ISSOCK(buffSrc.st_mode))
            {
                error(KIO::ERR_CANNOT_OPEN_FOR_READING, src.prettyURL());
                return false;
            }
        }
    }
    else
    {
        error(KIO::ERR_DOES_NOT_EXIST, src.prettyURL());
        return false;
    }

    return true;
}

bool CKioFonts::checkDestFiles(const KURL &src, QStringList &srcFiles, const KURL &dest, EFolder destFolder, bool overwrite)
{
    //
    // Check whether files exist at destination...
    //
    if(!itsRoot && (KFI_KIO_FONTS_PROTOCOL==dest.protocol() &&
                    KFI_KIO_FONTS_PROTOCOL==src.protocol())) // Then check on "Font Name"
    {    
        if(dest.directory()==src.directory())  // Check whether confirmUrl changed a "cp fonts:/System fonts:/"
                                               // to "cp fonts:/System fonts:/System"
        {
            error(KIO::ERR_FILE_ALREADY_EXIST, dest.prettyURL());
            return false;
        }
        else if(!overwrite)
        {
            QMap<QString, QValueList<FcPattern *> >::Iterator it=itsFolders[destFolder].fontMap.find(src.fileName());

            if(it!=itsFolders[destFolder].fontMap.end())
            {
                error(KIO::ERR_FILE_ALREADY_EXIST, dest.prettyURL());
                return false;
            }
        }
    }
    else // We're copying/moving from file, so just stat on the filename(s)...
    {
        if(!overwrite)
        {
            QStringList::Iterator it,
                                  end=srcFiles.end();

            for(it=srcFiles.begin(); it!=end; ++it)
            {
                QString file(Misc::getFile(*it));

                //
                // Check both original, and modified version of filename
                if(NULL!=getEntry(destFolder, file) || NULL!=getEntry(destFolder, modifyName(file)))
                {
                    error(KIO::ERR_FILE_ALREADY_EXIST, dest.prettyURL());
                    return false;
                }
            }
        }
    }

    return true;
}

//
// Gather the number and names of the font faces located in "files". If there is more than 1 face
// (such as there would be for a TTC font), then ask the user for confirmation of the action.
bool CKioFonts::confirmMultiple(const KURL &url, const QStringList &files, EFolder folder, EOp op)
{
    if(KFI_KIO_FONTS_PROTOCOL!=url.protocol())
        return true;

    QStringList::ConstIterator it,
                               end=files.end();
    QStringList                fonts;

    for(it=files.begin(); it!=files.end(); ++it)
    {
        FcPattern *pat=getEntry(folder, *it, false);

        if(pat)
        {
            QString name(CFcEngine::createName(pat));

            if(-1==fonts.findIndex(name))
                fonts.append(name);
        }
    }
    
    if(fonts.count()>1)
    {
        QString               out;
        QStringList::Iterator it,
                              end=fonts.end();

        for(it=fonts.begin(); it!=end; ++it)
            out+=QString("<li>")+*it+QString("</li>");

        if(KMessageBox::No==messageBox(QuestionYesNo,
                                              OP_MOVE==op
                                                ? i18n("<p>This font is located in a file alongside other fonts; in order "
                                                       "to proceed with the moving they will all have to be moved. "
                                                       "The other affected fonts are:</p><ul>%1</ul><p>\n Do you wish to "
                                                       "move all of these?</p>").arg(out)
                                            : OP_COPY==op
                                                ? i18n("<p>This font is located in a file alongside other fonts; in order "
                                                       "to proceed with the copying they will all have to be copied. "
                                                       "The other affected fonts are:</p><ul>%1</ul><p>\n Do you wish to "
                                                       "copy all of these?</p>").arg(out)
                                                : i18n("<p>This font is located in a file alongside other fonts; in order "
                                                       "to proceed with the deleting they will all have to be deleted. "
                                                       "The other affected fonts are:</p><ul>%1</ul><p>\n Do you wish to "
                                                       "delete all of these?</p>").arg(out)))
        {
            error(KIO::ERR_USER_CANCELED, url.prettyURL());
            return false;
        }
    }

    return true;
}

bool CKioFonts::confirmMultiple(const KURL &url, QValueList<FcPattern *> *patterns, EFolder folder, EOp op)
{
    if(KFI_KIO_FONTS_PROTOCOL!=url.protocol())
        return true;

    QStringList files;

    if(patterns && patterns->count())
    {
        QValueList<FcPattern *>::Iterator it,
                                          end=patterns->end();

        for(it=patterns->begin(); it!=end; ++it)
            files.append(getFcString(*it, FC_FILE));
    }

    return confirmMultiple(url, files, folder, op);
}

bool CKioFonts::checkUrl(const KURL &u, bool rootOk)
{
    if(KFI_KIO_FONTS_PROTOCOL==u.protocol() && (!rootOk || (rootOk && "/"!=u.path())))
    {
        QString sect(getSect(u.path()));

        if(itsRoot)
        {
            if (KFI_KIO_FONTS_USER==sect)
                 sect = i18n(KFI_KIO_FONTS_USER);
            if (KFI_KIO_FONTS_SYS==sect)
                 sect = i18n(KFI_KIO_FONTS_SYS);
                 
            if((i18n(KFI_KIO_FONTS_USER)==sect || i18n(KFI_KIO_FONTS_SYS)==sect) &&
               (itsFolders[FOLDER_SYS].fontMap.end()==itsFolders[FOLDER_SYS].fontMap.find(sect)))
//CPD: TODO: || it has a font specified! e.g. fonts:/System/Times -> even in have a fonts:/System font, redirect
//should still happen
            {
                 redirection(getRedirect(u));
                 finished();
                 return false;
            }
        }
        else
            if(i18n(KFI_KIO_FONTS_USER)!=sect && i18n(KFI_KIO_FONTS_SYS)!=sect &&
               KFI_KIO_FONTS_USER!=sect && KFI_KIO_FONTS_SYS!=sect)
            {
                error(KIO::ERR_SLAVE_DEFINED, i18n("Please specify \"%1\" or \"%2\".")
                      .arg(i18n(KFI_KIO_FONTS_USER)).arg(i18n(KFI_KIO_FONTS_SYS)));
                return false;
            }
    }

    return true;
}

bool CKioFonts::checkAllowed(const KURL &u)
{
    if (KFI_KIO_FONTS_PROTOCOL==u.protocol())
    {
        QString ds(Misc::dirSyntax(u.path()));

        if(ds==QString(QChar('/')+i18n(KFI_KIO_FONTS_USER)+QChar('/')) ||
           ds==QString(QChar('/')+i18n(KFI_KIO_FONTS_SYS)+QChar('/')) ||
           ds==QString(QChar('/')+QString::fromLatin1(KFI_KIO_FONTS_USER)+QChar('/')) ||
           ds==QString(QChar('/')+QString::fromLatin1(KFI_KIO_FONTS_SYS)+QChar('/')))
        {
            error(KIO::ERR_SLAVE_DEFINED, i18n("Sorry, you cannot rename, move, copy, or delete either \"%1\" or \"%2\".")
                  .arg(i18n(KFI_KIO_FONTS_USER)).arg(i18n(KFI_KIO_FONTS_SYS))); \
            return false;
        }
    }

    return true;
}

//
// Create an AFM from a Type 1 (pfa/pfb) font and its PFM file...
void CKioFonts::createAfm(const QString &file, bool nrs, const QString &passwd)
{
    if(nrs && passwd.isEmpty())
        return;

    static const char * afmExt[]={ "afm", "AFM", "Afm", "AFm", "AfM", "aFM", "aFm", "afM", NULL };
    static const char * pfmExt[]={ "pfm", "PFM", "Pfm", "PFm", "PfM", "pFM", "pFm", "pfM", NULL };
    static const char * t1Ext[]= { "pfa", "PFA", "Pfa", "PFa", "PfA", "pFA", "pFa", "pfA",
                                   "pfb", "PFB", "Pfb", "PFb", "PfB", "pFB", "pFb", "pfB", NULL };

    bool type1=isAType1(file),
         pfm=!type1 && isAPfm(file);  // No point checking if is pfm if its a type1

    if(type1 || pfm)
    {
        QString afm=getMatch(file, afmExt);

        if(afm.isEmpty())  // No point creating if AFM already exists!
        {
            QString pfm,
                    t1;

            if(type1)      // Its a Type1, so look for existing PFM
            {
                pfm=getMatch(file, pfmExt);
                t1=file;
            }
            else           // Its a PFM, so look for existing Type1
            {
                t1=getMatch(file, t1Ext);
                pfm=file;
            }

            if(!t1.isEmpty() && !pfm.isEmpty())         // Do we have both Type1 and PFM?
            {
                QString name(t1.left(t1.length()-4));   // pf2afm wants name without extension...

                if(nrs)
                {
                    QCString cmd("pf2afm ");
                    cmd+=QFile::encodeName(KProcess::quote(name));
                    doRootCmd(cmd, passwd);
                }
                else
                    Misc::doCmd("pf2afm", QFile::encodeName(name));
            }
        }
    }
}

}
