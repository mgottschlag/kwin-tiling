/*
   Copyright (c) 2002 Craig Drummond <craig@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kxftconfig.h"
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <qregexp.h>
#include <qfile.h>
#include <qpaintdevice.h>
//Added by qt3to4:
#include <QByteArray>
#include <Q3PtrList>
#include <klocale.h>
#include <kde_file.h>
#include <qdir.h>

#ifdef HAVE_FONTCONFIG
#include <stdarg.h>
#include <stdio.h>
#include <fontconfig/fontconfig.h>
#include <kdebug.h>
#endif

using namespace std;

QString KXftConfig::contractHome(QString path)
{
    if (!path.isEmpty() && '/'==path[0])
    {
        QString home(QDir::homeDirPath());

        if(path.startsWith(home))
        {
            int len = home.length();

            if(path.length() == len || path[len] == '/')
                return path.replace(0, len, QLatin1String("~"));
        }
    }

    return path;
}

QString KXftConfig::expandHome(QString path)
{
    if(!path.isEmpty() && '~'==path[0])
        return 1==path.length() ? QDir::homeDirPath() : path.replace(0, 1, QDir::homeDirPath());

    return path;
}

static int point2Pixel(double point)
{
    return (int)(((point*QPaintDevice::x11AppDpiY())/72.0)+0.5);
}

static int pixel2Point(double pixel)
{
    return (int)(((pixel*72.0)/(double)QPaintDevice::x11AppDpiY())+0.5);
}

static bool equal(double d1, double d2)
{
    return (fabs(d1 - d2) < 0.0001);
}

static QString dirSyntax(const QString &d)
{
    if(!d.isNull())
    {
        QString ds(d);

        ds.replace("//", "/");

        int slashPos=ds.findRev('/');

        if(slashPos!=(((int)ds.length())-1))
            ds.append('/');

        return ds;
    }

    return d;
}

static QString xDirSyntax(const QString &d)
{
    if(!d.isNull())
    {
        QString ds(d);
        int     slashPos=ds.findRev('/');
 
        if(slashPos==(((int)ds.length())-1))
            ds.remove(slashPos, 1);
        return ds;
    }

    return d;
}

static bool check(const QString &path, unsigned int fmt, bool checkW=false)
{
    KDE_struct_stat info;
    QByteArray        pathC(QFile::encodeName(path));

    return 0==KDE_lstat(pathC, &info) && (info.st_mode&S_IFMT)==fmt && (!checkW || 0==::access(pathC, W_OK));
}

inline bool fExists(const QString &p)
{
    return check(p, S_IFREG, false);
}

inline bool dWritable(const QString &p)
{
    return check(p, S_IFDIR, true);
}

inline bool dExists(const QString &p)
{
    return check(p, S_IFDIR, false);
}

static QString getDir(const QString &f)
{
    QString d(f);

    int slashPos=d.findRev('/');

    if(-1!=slashPos)
        d.remove(slashPos+1, d.length());

    return dirSyntax(d);
}

static time_t getTimeStamp(const QString &item)
{
    KDE_struct_stat info;

    return !item.isNull() && 0==KDE_lstat(QFile::encodeName(item), &info) ? info.st_mtime : 0;
}

#ifdef HAVE_FONTCONFIG

inline QString fileSyntax(const QString &f) { return xDirSyntax(f); }
//
// Obtain location of config file to use.
//
// For system, prefer the following:
//
//     <...>/config.d/00kde   = preferred method from FontConfig >= 2.3
//     <...>/local.conf
//
// Non-system, prefer:
//
//     $HOME/<...>/.fonts.conf
//     $HOME/<...>/fonts.conf
//
QString getConfigFile(bool system)
{
#if (FC_VERSION>=20300)
    static const char * constKdeRootFcFile="00kde";
#endif

    FcStrList   *list=FcConfigGetConfigFiles(FcConfigGetCurrent());
    QStringList files;
    FcChar8     *file;
    QString     home(dirSyntax(QDir::homeDirPath()));

    while((file=FcStrListNext(list)))
    {
        QString f((const char *)file);

        if(fExists(f))
        {
            if(system || 0==fileSyntax(f).find(home)) // For nonsystem, only consider file within $HOME
                files.append(f);
        }
#if (FC_VERSION>=20300)
        else if(system && dExists(f) && -1!=f.find(QRegExp("/conf\\.d/?$")))
            return dirSyntax(f)+constKdeRootFcFile;   // This ones good enough for me!
#endif
    }

    //
    // Go through list of files, looking for the preferred one...
    if(files.count())
    {
        QStringList::Iterator it(files.begin()),
                              end(files.end());

        for(; it!=end; ++it)
            if(-1!=(*it).find(QRegExp(system ? "/local\\.conf$" : "/\\.?fonts\\.conf$")))
                return *it;
        return files.front();  // Just return the 1st one...
    }
    else
        return system ? "/etc/fonts/local.conf" : fileSyntax(home+"/.fonts.conf"); // Hmmm... no known files?
}

static QString getEntry(QDomElement element, const char *type, unsigned int numAttributes, ...)
{
    if(numAttributes==element.attributes().length())
    {
        va_list      args;
        unsigned int arg;
        bool         ok=true;

        va_start(args, numAttributes);

        for(arg=0; arg<numAttributes && ok; ++arg)
        {
            const char *attr=va_arg(args, const char *);
            const char *val =va_arg(args, const char *);

            if(!attr || !val || val!=element.attribute(attr))
                ok=false;
        }

        va_end(args);

        if(ok)
        {
            QDomNode n=element.firstChild();

            if(!n.isNull())
            {
                QDomElement e = n.toElement();

                if(!e.isNull() && type==e.tagName())
                    return e.text();
            }
        }
    }

    return QString::null;
}

static KXftConfig::SubPixel::Type strToType(const char *str)
{
    if(0==strcmp(str, "\0"))
        return KXftConfig::SubPixel::Greyscale;
    else if(0==strcmp(str, "rgb"))
        return KXftConfig::SubPixel::Rgb;
    else if(0==strcmp(str, "bgr")) 
        return KXftConfig::SubPixel::Bgr;
    else if(0==strcmp(str, "vrgb"))
        return KXftConfig::SubPixel::Vrgb;
    else if(0==strcmp(str, "vbgr"))
        return KXftConfig::SubPixel::Vbgr;
    else
        return KXftConfig::SubPixel::None;
}

static KXftConfig::Hint::Style strToStyle(const char *str)
{
    if(0==strcmp(str, "hintslight"))
        return KXftConfig::Hint::Slight;
    else if(0==strcmp(str, "hintmedium"))
        return KXftConfig::Hint::Medium;
    else if(0==strcmp(str, "hintfull"))
        return KXftConfig::Hint::Full;
    else
        return KXftConfig::Hint::None;
}

#else
static bool strToType(const char *str, KXftConfig::SubPixel::Type &type)
{   
    if(0==memcmp(str, "\0", 1))
        type=KXftConfig::SubPixel::Greyscale;
    else if(0==memcmp(str, "rgb", 3))
        type=KXftConfig::SubPixel::Rgb;
    else if(0==memcmp(str, "bgr", 3))
        type=KXftConfig::SubPixel::Bgr;
    else if(0==memcmp(str, "vrgb", 4))
        type=KXftConfig::SubPixel::Vrgb;
    else if(0==memcmp(str, "vbgr", 4))
        type=KXftConfig::SubPixel::Vbgr;
    else if(0==memcmp(str, "none", 4))
        type=KXftConfig::SubPixel::None;
    else
        return false;
    return true;
}

static inline bool isWhiteSpace(char c)
{
    return c==' ' || c=='\n' || c== '\t';
}

static bool ok(char *data, char *entry)
{
    char *e=entry;

    for(;;)
    {
        e--;
        if(e==data || *e=='\n')
            return true;
        else
            if(!isWhiteSpace(*e))
                return false;
    }
    return false;
}

static char * getKey(char *data, const char *key)
{
    char *entry,
         *start=data;

    while(start &&start &&  (entry=strstr(start, key)))
        if(entry==data || ok(data, entry) && isWhiteSpace(entry[strlen(key)]))
            return entry;
        else
            start=entry+strlen(key);

    return NULL;
}

static bool skipToken(char **ptr, const char *token)
{
    while(isWhiteSpace(**ptr))
        (*ptr)++;
    if(0!=memcmp(*ptr, token, strlen(token)))
        return false;
    (*ptr)+=strlen(token);
    return true;
}

static bool readNum(char **ptr, double *num)
{
    static const int constMaxNumLen=64;

    char n[constMaxNumLen+1];
    bool foundNum=false,
         foundPoint=false,
         foundE=false;
    int  numChars=0;

    while(isWhiteSpace(**ptr))
        (*ptr)++;

    while(numChars<constMaxNumLen && (isdigit(**ptr) ||
          ('.'==**ptr && foundNum && !foundPoint && !foundE) || ('e'==**ptr && foundNum && !foundE)))
    {
        n[numChars++]=**ptr;
        if('.'==**ptr)
            foundPoint=true;
        else if('e'==**ptr)
            foundE=true;
        else
            foundNum=true;
        (*ptr)++;
    }

    if(numChars)
    {
        n[numChars]='\0';
        *num=atof(n);
        return true;
    }

    return false;
}

static KXftConfig::ListItem * getFirstItem(Q3PtrList<KXftConfig::ListItem> &list)
{
    KXftConfig::ListItem *cur;

    for(cur=list.first(); cur; cur=list.next())
        if(!cur->added())
            return cur;
    return NULL;
}
#endif

static KXftConfig::ListItem * getLastItem(Q3PtrList<KXftConfig::ListItem> &list)
{
    KXftConfig::ListItem *cur;

    for(cur=list.last(); cur; cur=list.prev())
        if(!cur->added())
            return cur;
    return NULL;
}

#ifndef HAVE_FONTCONFIG
static const QString defaultPath("/usr/X11R6/lib/X11/XftConfig");
static const QString defaultUserFile(".xftconfig");
static const char *  constSymEnc="\"glyphs-fontspecific\"";

static const QString constConfigFiles[]=
{
    defaultPath,

    "/etc/X11/XftConfig",
    QString::null
};
#endif

KXftConfig::KXftConfig(int required, bool system)
          : m_required(required),
#ifdef HAVE_FONTCONFIG
            m_doc("fontconfig"),
#else
            m_size(0),
            m_data(NULL),
#endif
            m_system(system)
{
#ifdef HAVE_FONTCONFIG
    m_file=getConfigFile(system);
    kdDebug(1208) << "Using fontconfig file:" << m_file << endl;
#else
    if(system) 
    {
        int f;

        for(f=0; !constConfigFiles[f].isNull(); ++f)
            if(fExists(constConfigFiles[f]))
                m_file=constConfigFiles[f];

        if(m_file.isNull())
            m_file=defaultPath;
    }
    else
        m_file= QString(QDir::homeDirPath()+"/"+defaultUserFile);
#endif
#ifndef HAVE_FONTCONFIG
    m_symbolFamilies.setAutoDelete(true);
#endif
    m_dirs.setAutoDelete(true);
    reset();
}

KXftConfig::~KXftConfig()
{
#ifndef HAVE_FONTCONFIG
    delete [] m_data;
    m_data=NULL;
#endif
}

bool KXftConfig::reset()
{
    bool ok=false;

    m_madeChanges=false;
#ifdef HAVE_FONTCONFIG
    m_hint.reset();
    m_hinting.reset();
#else
    m_symbolFamilies.clear();
#endif
    m_dirs.clear();
    m_excludeRange.reset();
    m_excludePixelRange.reset();
    m_subPixel.reset();

#ifdef HAVE_FONTCONFIG
    QFile f(m_file);

    if(f.open(QIODevice::ReadOnly))
    {
        m_time=getTimeStamp(m_file);
        ok=true;
        m_doc.clear();

        if(m_doc.setContent(&f))
            readContents();
        f.close();
    }
    else
        ok=!fExists(m_file) && dWritable(getDir(m_file));

    if(m_doc.documentElement().isNull())
        m_doc.appendChild(m_doc.createElement("fontconfig"));
#else
    QFile f(m_file);

    m_size=0;
    delete [] m_data;
    m_data=NULL;

    if(f.open(QIODevice::Unbuffered|QIODevice::ReadOnly))
    {
        m_time=getTimeStamp(m_file);
        m_size=f.size();
        ok=true;

        if(m_size>0)
        {
            m_data=new char [m_size+1];

            if(m_data)
            {
                f.readBlock(m_data, m_size);
                m_data[m_size]='\0';
                readContents();
            }
        }
        f.close();
    }
    else
        ok=!fExists(m_file) && dWritable(getDir(m_file));
#endif

    if(ok && m_required&ExcludeRange)
    {
        //
        // Check exclude range values - i.e. size and pixel size...
        if(!equal(0, m_excludeRange.from) || !equal(0, m_excludeRange.to))    // If "size" range is set, ensure "pixelsize" matches...
        {
            double pFrom=(double)point2Pixel(m_excludeRange.from),
                   pTo=(double)point2Pixel(m_excludeRange.to);

            if(!equal(pFrom, m_excludePixelRange.from) || !equal(pTo, m_excludePixelRange.to))
            {
                m_excludePixelRange.from=pFrom;
                m_excludePixelRange.to=pTo;
                m_madeChanges=true;
                apply();
            }
        }
        else if(!equal(0, m_excludePixelRange.from) || !equal(0, m_excludePixelRange.to))   // "pixelsize" set, but not "size" !!!
        {
            m_excludeRange.from=(int)pixel2Point(m_excludePixelRange.from);
            m_excludeRange.to=(int)pixel2Point(m_excludePixelRange.to);
            m_madeChanges=true;
            apply();
        }
    }

    return ok;
}

bool KXftConfig::apply()
{
    bool ok=true;

    if(m_madeChanges)
    {
        //
        // Check if file has been written since we last read it. If it has, then re-read and add any
        // of our changes...
        if(fExists(m_file) && getTimeStamp(m_file)!=m_time)
        {
            KXftConfig            newConfig(m_required, m_system);
            QStringList           list;
            QStringList::Iterator it;

            if(m_required&Dirs)
            {
                list=getDirs();
                for(it=list.begin(); it!=list.end(); ++it)
                    newConfig.addDir(*it);
            }

            if(m_required&ExcludeRange)
                newConfig.setExcludeRange(m_excludeRange.from, m_excludeRange.to);
            if(m_required&SubPixelType)
                newConfig.setSubPixelType(m_subPixel.type);
#ifdef HAVE_FONTCONFIG
            if(m_required&HintStyle)
                newConfig.setHintStyle(m_hint.style);
#else
            if(m_required&SymbolFamilies)
            {
                list=getSymbolFamilies();
                for(it=list.begin(); it!=list.end(); ++it)
                    newConfig.addSymbolFamily(*it);
            }
#endif

            ok=newConfig.changed() ? newConfig.apply() : true;
            if(ok)
                reset();
            else
                m_time=getTimeStamp(m_file);
        }
        else
        {
            if(m_required&ExcludeRange)
            {
                // Ensure these are always equal...
                m_excludePixelRange.from=(int)point2Pixel(m_excludeRange.from);
                m_excludePixelRange.to=(int)point2Pixel(m_excludeRange.to);
            }
    
#ifdef HAVE_FONTCONFIG
            FcAtomic *atomic=FcAtomicCreate((const unsigned char *)((const char *)(QFile::encodeName(m_file))));
    
            ok=false;
            if(atomic)
            {
                if(FcAtomicLock(atomic))
                {
                    FILE *f=fopen((char *)FcAtomicNewFile(atomic), "w");
    
                    if(f)
                    {
                        if(m_required&Dirs)
                        {
                            applyDirs();
                            removeItems(m_dirs);
                        }
                        if(m_required&SubPixelType)
                            applySubPixelType();
                        if(m_required&HintStyle)
                            applyHintStyle();
                        if(m_required&ExcludeRange)
                        {
                            applyExcludeRange(false);
                            applyExcludeRange(true);
                        }
    
                        //
                        // Check document syntax...
                        static const char * qtXmlHeader   = "<?xml version = '1.0'?>";
                        static const char * xmlHeader     = "<?xml version=\"1.0\"?>";
                        static const char * qtDocTypeLine = "<!DOCTYPE fontconfig>";
                        static const char * docTypeLine   = "<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">";
    
                        QString str(m_doc.toString());
                        int     idx;
    
                        if(0!=str.find("<?xml"))
                            str.insert(0, xmlHeader);
                        else if(0==str.find(qtXmlHeader))
                            str.replace(0, strlen(qtXmlHeader), xmlHeader);
    
                        if(-1!=(idx=str.find(qtDocTypeLine)))
                            str.replace(idx, strlen(qtDocTypeLine), docTypeLine);
    
                        //
                        // Write to file...
                        fputs(str.utf8(), f);
                        fclose(f);
    
                        if(FcAtomicReplaceOrig(atomic))
                        {
                            ok=true;
                            reset(); // Re-read contents..
                        }
                        else
                            FcAtomicDeleteNew(atomic);
                    }
                    FcAtomicUnlock(atomic);
                }
                FcAtomicDestroy(atomic);
            }
#else
            std::ofstream f(QFile::encodeName(m_file));
    
            if(f)
            {
                ListItem *ldi=m_required&Dirs ? getLastItem(m_dirs) : NULL,
                        *lfi=m_required&SymbolFamilies ? getLastItem(m_symbolFamilies) : NULL;
                char     *pos=m_data;
                bool     finished=false,
                        pixel=false;
    
                while(!finished)
                {
                    int      type=0;
                    ListItem *fdi=NULL,
                            *ffi=NULL;
                    Item     *first=NULL;
    
                    if(m_required&Dirs && NULL!=(fdi=getFirstItem(m_dirs)) && (NULL==first || fdi->start < first->start))
                    {
                        first=fdi;
                        type=Dirs;
                    }
                    if(m_required&SymbolFamilies && NULL!=(ffi=getFirstItem(m_symbolFamilies)) && (NULL==first || ffi->start < first->start))
                    {
                        first=ffi;
                        type=SymbolFamilies;
                    }
                    if(m_required&SubPixelType && NULL!=m_subPixel.start && (NULL==first || m_subPixel.start < first->start))
                    {
                        first=&m_subPixel;
                        type=SubPixelType;
                    }
                    if(m_required&ExcludeRange)
                        if(NULL!=m_excludeRange.start && (NULL==first || m_excludeRange.start < first->start))
                        {
                            first=&m_excludeRange;
                            type=ExcludeRange;
                            pixel=false;
                        }
                        else if(NULL!=m_excludePixelRange.start && (NULL==first || m_excludePixelRange.start < first->start))
                        {
                            first=&m_excludePixelRange;
                            type=ExcludeRange;
                            pixel=true;
                        }
    
                    if(first && first->start!=pos)
                        f.write(pos, first->start-pos);
    
                    if(0!=type)
                        pos=first->end+1;
    
                    switch(type)
                    {
                        case Dirs:
                            if(!first->toBeRemoved)
                                outputDir(f, fdi->str);
                            m_dirs.remove(fdi);
                            if(fdi==ldi)
                                outputNewDirs(f);
                            break;
                        case SymbolFamilies:
                            if(!first->toBeRemoved)
                                outputSymbolFamily(f, ffi->str);
                            m_symbolFamilies.remove(ffi);
                            if(ffi==lfi)
                                outputNewSymbolFamilies(f);
                            break;
                        case SubPixelType:
                            if(!first->toBeRemoved)
                                outputSubPixelType(f, false);
                            m_subPixel.start=NULL;
                            break;
                        case ExcludeRange:
                            if(!first->toBeRemoved)
                                outputExcludeRange(f, false, pixel);
                            m_excludeRange.start=NULL;
                            break;
                        case 0: // 0 => All read in entries written...
                            if(m_size && (pos < m_data+m_size))
                                f.write(pos, (m_data+m_size)-pos);
                        default:
                            finished=true;
                            break;
                    }
                };
    
                outputNewDirs(f);
                outputNewSymbolFamilies(f);
                outputSubPixelType(f, true);
                outputExcludeRange(f, true, false);
                outputExcludeRange(f, true, true);
                f.close();
                reset(); // Re-read contents...
            }
            else
                ok=false;
#endif
        }
    }

    return ok;
}

bool KXftConfig::getSubPixelType(SubPixel::Type &type)
{
    if(SubPixel::None!=m_subPixel.type && !m_subPixel.toBeRemoved)
    {
        type=m_subPixel.type;
        return true;
    }
    else
        return false;
}

void KXftConfig::setSubPixelType(SubPixel::Type type)
{
    if((SubPixel::None==type && SubPixel::None!=m_subPixel.type && !m_subPixel.toBeRemoved) ||
       (SubPixel::None!=type && (type!=m_subPixel.type || m_subPixel.toBeRemoved)) )
    {
        m_subPixel.toBeRemoved=(SubPixel::None==type);
        m_subPixel.type=type;
        m_madeChanges=true;
    }
}

#ifdef HAVE_FONTCONFIG
bool KXftConfig::getHintStyle(Hint::Style &style)
{
    if(Hint::NotSet!=m_hint.style && !m_hint.toBeRemoved)
    {
        style=m_hint.style;
        return true;
    }
    else
        return false;
}

void KXftConfig::setHintStyle(Hint::Style style)
{
    if((Hint::NotSet==style && Hint::NotSet!=m_hint.style && !m_hint.toBeRemoved) ||
       (Hint::NotSet!=style && (style!=m_hint.style || m_hint.toBeRemoved)) )
    {
        m_hint.toBeRemoved=(Hint::NotSet==style);
        m_hint.style=style;
        m_madeChanges=true;
    }

    if(Hint::NotSet!=style)
        setHinting(Hint::None!=m_hint.style);
}

void KXftConfig::setHinting(bool set)
{
    if(set!=m_hinting.set)
    {
        m_hinting.set=set;
        m_madeChanges=true;
    }
}
#endif

bool KXftConfig::getExcludeRange(double &from, double &to)
{
    if(!equal(0, m_excludeRange.from) || !equal(0,m_excludeRange.to))
    {
        from=m_excludeRange.from;
        to=m_excludeRange.to;
        return true;
    }
    else
        return false;
}


void KXftConfig::setExcludeRange(double from, double to)
{
    double f=from<to ? from : to,
           t=from<to ? to   : from;

    if(!equal(f, m_excludeRange.from) || !equal(t,m_excludeRange.to))
    {
        m_excludeRange.from=f;
        m_excludeRange.to=t;
        m_madeChanges=true;
    }
}

void KXftConfig::addDir(const QString &d)
{
    QString dir(dirSyntax(d));

    if(dExists(dir) && !hasDir(dir))
        addItem(m_dirs, dir);
}

void KXftConfig::removeDir(const QString &d)
{
    QString dir(dirSyntax(d));

    removeItem(m_dirs, dir);
}

QString KXftConfig::description(SubPixel::Type t)
{
    switch(t)
    {
        default:
        case SubPixel::None:
            return i18n("None");
        case SubPixel::Greyscale:
            return i18n("Grayscale");
        case SubPixel::Rgb:
            return i18n("RGB");
        case SubPixel::Bgr:
            return i18n("BGR");
        case SubPixel::Vrgb:
            return i18n("Vertical RGB");
        case SubPixel::Vbgr:
            return i18n("Vertical BGR");
    }
}

const char * KXftConfig::toStr(SubPixel::Type t)
{
    switch(t)
    {
        default:
        case SubPixel::None:
            return "none";
        case SubPixel::Greyscale:
            return "";
        case SubPixel::Rgb:
            return "rgb";
        case SubPixel::Bgr:
            return "bgr";
        case SubPixel::Vrgb:
            return "vrgb";
        case SubPixel::Vbgr:
            return "vbgr";
    }
}

#ifdef HAVE_FONTCONFIG
QString KXftConfig::description(Hint::Style s)
{
    switch(s)
    {
        default:
        case Hint::Medium:
            return i18n("Medium");
        case Hint::NotSet:
            return "";
        case Hint::None:
            return i18n("None");
        case Hint::Slight:
            return i18n("Slight");
        case Hint::Full:
            return i18n("Full");
    }
}

const char * KXftConfig::toStr(Hint::Style s)
{
    switch(s)
    {
        default:
        case Hint::Medium:
            return "hintmedium";
        case Hint::None:
            return "hintnone";
        case Hint::Slight:
            return "hintslight";
        case Hint::Full:
            return "hintfull";
    }
}
#endif

bool KXftConfig::hasDir(const QString &d)
{
    QString dir(dirSyntax(d));

#ifdef HAVE_FONTCONFIG
    ListItem *item;

    for(item=m_dirs.first(); item; item=m_dirs.next())
        if(0==dir.find(item->str))
            return true;

    return false;
#else
    return NULL!=findItem(m_dirs, dir);
#endif
}

KXftConfig::ListItem * KXftConfig::findItem(Q3PtrList<ListItem> &list, const QString &i)
{   
    ListItem *item;

    for(item=list.first(); item; item=list.next())
        if(item->str==i)
            break;

    return item;
}

void KXftConfig::clearList(Q3PtrList<ListItem> &list)
{
    ListItem *item;

    for(item=list.first(); item; item=list.next())
        removeItem(list, item);
}

QStringList KXftConfig::getList(Q3PtrList<ListItem> &list)
{
    QStringList res;
    ListItem    *item;

    for(item=list.first(); item; item=list.next())
        if(!item->toBeRemoved)
            res.append(item->str);

    return res;
}

void KXftConfig::addItem(Q3PtrList<ListItem> &list, const QString &i)
{
    ListItem *item=findItem(list, i);

    if(!item)
    {
        list.append(new ListItem(i
#ifndef HAVE_FONTCONFIG
                                 , NULL, NULL
#endif
                                ));
        m_madeChanges=true;
    }
    else
        item->toBeRemoved=false;
}

void KXftConfig::removeItem(Q3PtrList<ListItem> &list, ListItem *item)
{
    if(item)
    {
        if(item->added())
            list.remove(item);
        else
            item->toBeRemoved=true;
        m_madeChanges=true;
    }
}

void KXftConfig::readContents()
{
#ifdef HAVE_FONTCONFIG
    QDomNode n = m_doc.documentElement().firstChild();

    while(!n.isNull())
    {
        QDomElement e = n.toElement();

        if(!e.isNull())
            if("dir"==e.tagName())
            {
                if(m_required&Dirs)
                    m_dirs.append(new ListItem(expandHome(dirSyntax(e.text())), n));
            }
            else if("match"==e.tagName())
            {
                QString str;

                switch(e.childNodes().count())
                {
                    case 1:
                        if(m_required&SubPixelType && "font"==e.attribute("target"))
                        {
                            QDomElement ene=e.firstChild().toElement();

                            if(!ene.isNull() && "edit"==ene.tagName())
                                if(!(str=getEntry(ene, "const", 2, "name", "rgba", "mode", "assign")).isNull())
                                {
                                    m_subPixel.node=n;
                                    m_subPixel.type=strToType(str.latin1());
                                }
                                else if(!(str=getEntry(ene, "const", 2, "name", "hintstyle", "mode", "assign")).isNull())
                                {
                                    m_hint.node=n;
                                    m_hint.style=strToStyle(str.latin1());
                                }
                                else if(!(str=getEntry(ene, "bool", 2, "name", "hinting", "mode", "assign")).isNull())
                                {
                                    m_hinting.node=n;
                                    m_hinting.set=str.lower()!="false";
                                }
                        }
                        break;
                    case 3:
                        if(m_required&ExcludeRange && "font"==e.attribute("target"))  // CPD: Is target "font" or "pattern" ????
                        {
                            bool     foundFalse=false;
                            QDomNode en=e.firstChild();
                            QString  family;
                            double   from=-1.0,
                                     to=-1.0,
                                     pixelFrom=-1.0,
                                     pixelTo=-1.0;

                            while(!en.isNull())
                            {
                                QDomElement ene=en.toElement();

                                if(!ene.isNull())
                                    if("test"==ene.tagName())
                                    {
                                        if(!(str=getEntry(ene, "double", 3, "qual", "any", "name", "size", "compare", "more")).isNull())
                                            from=str.toDouble();
                                        else if(!(str=getEntry(ene, "double", 3, "qual", "any", "name", "size", "compare", "less")).isNull())
                                            to=str.toDouble();
                                        else if(!(str=getEntry(ene, "double", 3, "qual", "any", "name", "pixelsize", "compare", "more")).isNull())
                                            pixelFrom=str.toDouble();
                                        else if(!(str=getEntry(ene, "double", 3, "qual", "any", "name", "pixelsize", "compare", "less")).isNull())
                                            pixelTo=str.toDouble();
                                    }
                                    else if("edit"==ene.tagName() && "false"==getEntry(ene, "bool", 2, "name", "antialias", "mode", "assign"))
                                        foundFalse=true;

                                en=en.nextSibling();
                            }

                            if((from>=0 || to>=0) && foundFalse)
                            {
                                m_excludeRange.from=from < to ? from : to;
                                m_excludeRange.to  =from < to ? to   : from;
                                m_excludeRange.node=n;
                            }
                            else if((pixelFrom>=0 || pixelTo>=0) && foundFalse)
                            {
                                m_excludePixelRange.from=pixelFrom < pixelTo ? pixelFrom : pixelTo;
                                m_excludePixelRange.to  =pixelFrom < pixelTo ? pixelTo   : pixelFrom;
                                m_excludePixelRange.node=n;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        n=n.nextSibling();
    }
#else
    static const int constMaxDataLen=2048;

    char *from=NULL,
         *ptr=m_data,
         *eostr=NULL,
         data[constMaxDataLen];

    if(m_required&&Dirs)
        while((ptr=getKey(ptr, "dir")))
        {
            from=ptr;
            ptr+=4;
            while(isWhiteSpace(*ptr))
                ptr++;

            if(*ptr=='\"')
            {
                ptr++;
                if(NULL!=(eostr=strchr(ptr, '\"')) && eostr-ptr<constMaxDataLen)
                {
                    memcpy(data, ptr, eostr-ptr);
                    data[eostr-ptr]='\0';
                    if(NULL==strchr(data, '\n'))
                    {
                        ptr=eostr+1;

                        while(*ptr!='\n' && *ptr!='\0' && isWhiteSpace(*ptr))
                            ptr++;
                        m_dirs.append(new ListItem(expandHome(dirSyntax(data)), from, ptr));
                    }
               }
            }
        }

    if(m_required&SymbolFamilies || m_required&SubPixelType || m_required&ExcludeRange)
    {
        double efrom,
               eto;

        ptr=m_data;

        while((ptr=getKey(ptr, "match")))
        {
            from=ptr;
            ptr+=6;
            if((m_required&SymbolFamilies || m_required&ExcludeRange) && skipToken(&ptr, "any"))
            {
                if(m_required&SymbolFamilies && skipToken(&ptr, "family") && skipToken(&ptr, "=="))
                {
                    while(isWhiteSpace(*ptr))
                        ptr++;
                    if(*ptr=='\"')
                    {
                        ptr++;
                        if(NULL!=(eostr=strchr(ptr, '\"')) && eostr-ptr<constMaxDataLen)
                        {
                            memcpy(data, ptr, eostr-ptr);
                            data[eostr-ptr]='\0';
                            if(NULL==strchr(data, '\n'))
                            {
                                ptr=eostr+1;

                                if(skipToken(&ptr, "edit") && skipToken(&ptr, "encoding") && skipToken(&ptr, "=") && 
                                   skipToken(&ptr, constSymEnc) && skipToken(&ptr, ";"))
                                {
                                    while(*ptr!='\n' && *ptr!='\0' && isWhiteSpace(*ptr))
                                        ptr++;

                                    m_symbolFamilies.append(new ListItem(data, from, ptr));
                                }
                            }
                        }
                    }
                }
                else if(m_required&ExcludeRange && skipToken(&ptr, "size") && (skipToken(&ptr, ">")||skipToken(&ptr, "<")) && 
                        readNum(&ptr, &efrom) && skipToken(&ptr, "any") && skipToken(&ptr, "size") &&
                        (skipToken(&ptr, "<")||skipToken(&ptr, ">")) && readNum(&ptr, &eto) && skipToken(&ptr, "edit") &&
                        skipToken(&ptr, "antialias") && skipToken(&ptr, "=") && skipToken(&ptr, "false") && skipToken(&ptr, ";"))
                {
                    while(*ptr!='\n' && *ptr!='\0' && isWhiteSpace(*ptr))
                        ptr++;
                    m_excludeRange.from=efrom<eto ? efrom : eto;
                    m_excludeRange.to=efrom<eto ?   eto   : efrom;
                    m_excludeRange.start=from;
                    m_excludeRange.end=ptr;
                }
                else if(m_required&ExcludeRange && skipToken(&ptr, "pixelsize") && (skipToken(&ptr, ">")||skipToken(&ptr, "<")) && 
                        readNum(&ptr, &efrom) && skipToken(&ptr, "any") && skipToken(&ptr, "pixelsize") &&
                        (skipToken(&ptr, "<")||skipToken(&ptr, ">")) && readNum(&ptr, &eto) && skipToken(&ptr, "edit") &&
                        skipToken(&ptr, "antialias") && skipToken(&ptr, "=") && skipToken(&ptr, "false") && skipToken(&ptr, ";"))
                {
                    while(*ptr!='\n' && *ptr!='\0' && isWhiteSpace(*ptr))
                        ptr++;
                    m_excludePixelRange.from=efrom<eto ? efrom : eto;
                    m_excludePixelRange.to=efrom<eto ?   eto   : efrom;
                    m_excludePixelRange.start=from;
                    m_excludePixelRange.end=ptr;
                }
            }
            else if(m_required&SubPixelType && skipToken(&ptr, "edit") && skipToken(&ptr, "rgba") && skipToken(&ptr, "="))
            {
                SubPixel::Type type=SubPixel::None;

                while(isWhiteSpace(*ptr))
                    ptr++;

                if(!strToType(ptr, type))
                    continue;

                ptr+=SubPixel::Rgb==type || SubPixel::Bgr==type ? 3 : 4;

                if(skipToken(&ptr, ";"))
                {
                    while(*ptr!='\n' && *ptr!='\0' && isWhiteSpace(*ptr))
                        ptr++;
                    m_subPixel.type=type;
                    m_subPixel.start=from;
                    m_subPixel.end=ptr;
                }
            }
        }
    }
#endif
}

#ifdef HAVE_FONTCONFIG
void KXftConfig::applyDirs()
{
    ListItem *item,
             *last=getLastItem(m_dirs);

    for(item=m_dirs.first(); item; item=m_dirs.next())
        if(!item->toBeRemoved && item->node.isNull())
        {
            QDomElement newNode = m_doc.createElement("dir");
            QDomText    text    = m_doc.createTextNode(contractHome(xDirSyntax(item->str)));

            newNode.appendChild(text);

            if(last)
                m_doc.documentElement().insertAfter(newNode, last->node);
            else
                m_doc.documentElement().appendChild(newNode);
        }
}

void KXftConfig::applySubPixelType()
{
    if(SubPixel::None==m_subPixel.type || m_subPixel.toBeRemoved)
    {
        if(!m_subPixel.node.isNull())
        {
            m_doc.documentElement().removeChild(m_subPixel.node);
            m_subPixel.node.clear();
        }
    }
    else
    {
        QDomElement matchNode = m_doc.createElement("match"),
                    typeNode  = m_doc.createElement("const"),
                    editNode  = m_doc.createElement("edit");
        QDomText    typeText  = m_doc.createTextNode(toStr(m_subPixel.type));

        matchNode.setAttribute("target", "font");
        editNode.setAttribute("mode", "assign");
        editNode.setAttribute("name", "rgba");
        editNode.appendChild(typeNode);
        typeNode.appendChild(typeText);
        matchNode.appendChild(editNode);
        if(m_subPixel.node.isNull())
            m_doc.documentElement().appendChild(matchNode);
        else
            m_doc.documentElement().replaceChild(matchNode, m_subPixel.node);
        m_subPixel.node=matchNode;
    }
}

void KXftConfig::applyHintStyle()
{
    applyHinting();

    if(Hint::NotSet==m_hint.style || m_hint.toBeRemoved)
    {
        if(!m_hint.node.isNull())
        {
            m_doc.documentElement().removeChild(m_hint.node);
            m_hint.node.clear();
        }
    }
    else
    {
        QDomElement matchNode = m_doc.createElement("match"),
                    typeNode  = m_doc.createElement("const"),
                    editNode  = m_doc.createElement("edit");
        QDomText    typeText  = m_doc.createTextNode(toStr(m_hint.style));

        matchNode.setAttribute("target", "font");
        editNode.setAttribute("mode", "assign");
        editNode.setAttribute("name", "hintstyle");
        editNode.appendChild(typeNode);
        typeNode.appendChild(typeText);
        matchNode.appendChild(editNode);
        if(m_hint.node.isNull())
            m_doc.documentElement().appendChild(matchNode);
        else
            m_doc.documentElement().replaceChild(matchNode, m_hint.node);
        m_hint.node=matchNode;
    }
}

void KXftConfig::applyHinting()
{
    QDomElement matchNode = m_doc.createElement("match"),
                typeNode  = m_doc.createElement("bool"),
                editNode  = m_doc.createElement("edit");
    QDomText    typeText  = m_doc.createTextNode(m_hinting.set ? "true" : "false");

    matchNode.setAttribute("target", "font");
    editNode.setAttribute("mode", "assign");
    editNode.setAttribute("name", "hinting");
    editNode.appendChild(typeNode);
    typeNode.appendChild(typeText);
    matchNode.appendChild(editNode);
    if(m_hinting.node.isNull())
        m_doc.documentElement().appendChild(matchNode);
    else
        m_doc.documentElement().replaceChild(matchNode, m_hinting.node);
    m_hinting.node=matchNode;
}

void KXftConfig::applyExcludeRange(bool pixel)
{
    Exclude &range=pixel ? m_excludePixelRange : m_excludeRange;

    if(equal(range.from, 0) && equal(range.to, 0))
    {
        if(!range.node.isNull())
        {
            m_doc.documentElement().removeChild(range.node);
            range.node.clear();
        }
    }
    else
    {
        QString     fromString,
                    toString;

        fromString.setNum(range.from);
        toString.setNum(range.to);

        QDomElement matchNode    = m_doc.createElement("match"),
                    fromTestNode = m_doc.createElement("test"),
                    fromNode     = m_doc.createElement("double"),
                    toTestNode   = m_doc.createElement("test"),
                    toNode       = m_doc.createElement("double"),
                    editNode     = m_doc.createElement("edit"),
                    boolNode     = m_doc.createElement("bool");
        QDomText    fromText     = m_doc.createTextNode(fromString),
                    toText       = m_doc.createTextNode(toString),
                    boolText     = m_doc.createTextNode("false");

        matchNode.setAttribute("target", "font");   // CPD: Is target "font" or "pattern" ????
        fromTestNode.setAttribute("qual", "any");
        fromTestNode.setAttribute("name", pixel ? "pixelsize" : "size");
        fromTestNode.setAttribute("compare", "more");
        fromTestNode.appendChild(fromNode);
        fromNode.appendChild(fromText);
        toTestNode.setAttribute("qual", "any");
        toTestNode.setAttribute("name", pixel ? "pixelsize" : "size");
        toTestNode.setAttribute("compare", "less");
        toTestNode.appendChild(toNode);
        toNode.appendChild(toText);
        editNode.setAttribute("mode", "assign");
        editNode.setAttribute("name", "antialias");
        editNode.appendChild(boolNode);
        boolNode.appendChild(boolText);
        matchNode.appendChild(fromTestNode);
        matchNode.appendChild(toTestNode);
        matchNode.appendChild(editNode);
        if(range.node.isNull())
            m_doc.documentElement().appendChild(matchNode);
        else
            m_doc.documentElement().replaceChild(matchNode, range.node);
        range.node=matchNode;
    }
}

void KXftConfig::removeItems(Q3PtrList<ListItem> &list)
{
    ListItem    *item;
    QDomElement docElem = m_doc.documentElement();

    for(item=list.first(); item; item=list.next())
        if(item->toBeRemoved && !item->node.isNull())
            docElem.removeChild(item->node);
}
#else
void KXftConfig::outputDir(std::ofstream &f, const QString &str)
{
    f << "dir \"" << contractHome(xDirSyntax(str)).toLocal8Bit().data() << "\"" << endl;
}

void KXftConfig::outputNewDirs(std::ofstream &f)
{
    ListItem *item;

    for(item=m_dirs.first(); item; item=m_dirs.next())
        if(!item->toBeRemoved && NULL==item->start)
            outputDir(f, item->str);
    m_dirs.clear();
}

void KXftConfig::outputSymbolFamily(std::ofstream &f, const QString &str)
{
    f << "match any family == \"" << str.toLocal8Bit().data() << "\" edit encoding = " << constSymEnc << ';' << endl;
}

void KXftConfig::outputNewSymbolFamilies(std::ofstream &f)
{
    ListItem *item;

    for(item=m_symbolFamilies.first(); item; item=m_symbolFamilies.next())
        if(!item->toBeRemoved && NULL==item->start)
            outputSymbolFamily(f, item->str);
    m_symbolFamilies.clear();
}

void KXftConfig::outputSubPixelType(std::ofstream &f, bool ifNew)
{
    if(!m_subPixel.toBeRemoved && ((ifNew && NULL==m_subPixel.end) || (!ifNew && NULL!=m_subPixel.end)) && SubPixel::None!=m_subPixel.type)
        f << "match edit rgba = " << toStr(m_subPixel.type) << ';' << endl;
}

void KXftConfig::outputExcludeRange(std::ofstream &f, bool ifNew, bool pixel)
{
    Exclude &range=pixel ? m_excludePixelRange : m_excludeRange;

    if(((ifNew && NULL==range.end) || (!ifNew && NULL!=range.end)) &&
       (!equal(range.from,0) || !equal(range.to,0)))
    {
        if(pixel)
            f << "match any pixelsize > ";
        else
            f << "match any size > ";

        f << range.from;
        if(pixel)
            f << " any pixelsize < ";
        else
            f << " any size < ";
        f << range.to << " edit antialias = false;" << endl;
}
}
#endif
