////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXConfig
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 05/05/2001
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
// (C) Craig Drummond, 2001, 2002, 2003
////////////////////////////////////////////////////////////////////////////////

#include "XConfig.h"
#include "Global.h"
#include "FontEngine.h"
#include "kxftconfig.h"
#include <fstream>
#include <string.h>
#include <qdir.h>
#include <qregexp.h>
#include <klocale.h>

extern "C" unsigned int kfi_getPid(const char *proc, unsigned int ppid);

static const QCString constFontpaths ("# KFontinst fontpaths file -- DO NOT EDIT");
#define UNSCALED ":unscaled"

using namespace std;

CXConfig::CXConfig(EType type, const QString &file)
        : itsType(type),
          itsFileName(file),
          itsOk(false),
          itsWritable(false)
{
    itsPaths.setAutoDelete(true);
    readConfig();
}

#ifdef HAVE_FONTCONFIG
bool CXConfig::configureDir(const QString &dir)
#else
bool CXConfig::configureDir(const QString &dir, QStringList &symbolFamilies)
#endif
{
#ifdef HAVE_FONTCONFIG
    bool status=createFontsDotDir(dir);
#else
    bool status=createFontsDotDir(dir, symbolFamilies);
#endif

    if(status)
        CGlobal::enc().createEncodingsDotDir(dir);

    return status;
}

bool CXConfig::readConfig()
{
    switch(itsType)
    {
        case XFS:
            itsOk=processXfs(true);
            break;
        case XF86:
            itsOk=processXf86(true);
            break;
        case KFI:
            itsOk=readFontpaths();
            break;
    }

    if(itsOk)
        itsWritable=CMisc::fExists(itsFileName) ? CMisc::fWritable(itsFileName)
                                                : CMisc::dWritable(CMisc::getDir(itsFileName));
    else
        itsWritable=false;

    return itsOk;
}

bool CXConfig::writeConfig()
{
    bool written=false;

    //
    // Check if file has been written since we last read it. If so, then re-read
    // and add any new paths that we've added...
    if(CMisc::fExists(itsFileName) && CMisc::getTimeStamp(itsFileName)!=itsTime)
    {
        CXConfig newConfig(itsType, itsFileName);

        if(newConfig.ok())
        {
            TPath *path;

            for(path=itsPaths.first(); path; path=itsPaths.next())
                if(TPath::DIR==path->type && !path->orig)
                    newConfig.addPath(path->dir, path->unscaled);

            written=newConfig.madeChanges() ? newConfig.writeConfig() : true;
        }
    }
    else
        switch(itsType)
        {
            case XFS:
                written=processXfs(false);
                break;
            case XF86:
                written=processXf86(false);
                break;
            case KFI:
                written=writeFontpaths();
                break;
        }
    if(written)
        readConfig();

    return written;
}

bool CXConfig::madeChanges()
{
    if(itsOk && itsWritable)
    {
        TPath *path;
 
        for(path=itsPaths.first(); path; path=itsPaths.next())
            if(!path->orig || path->toBeRemoved)
                return true;
    }

    return false;
}
 
bool CXConfig::inPath(const QString &dir)
{
    TPath *path=findPath(dir);
 
    return NULL==path || path->toBeRemoved ? false : true;
}

bool CXConfig::subInPath(const QString &dir)
{
    //
    // Look for /a/b/c/d/e/ in
    //    /a/b/c/d/e/f/g/, /a/b/c/, /.....
    // ... /a/b/c/e/f/g/ matches

    TPath   *path=NULL;
    QString ds(CMisc::dirSyntax(dir));


    for(path=itsPaths.first(); path; path=itsPaths.next())
        if(0==path->dir.find(dir))
            return true;

    return false;
}

void CXConfig::addPath(const QString &dir, bool unscaled)
{
    if(itsWritable)
    {
        QString ds(CMisc::dirSyntax(dir));
        TPath   *path=findPath(ds);

        if(NULL==path)
            itsPaths.append(new TPath(ds, unscaled, TPath::DIR, false));
        else
            if(path->toBeRemoved)
                path->toBeRemoved=false;
    }
}

void CXConfig::removePath(const QString &dir)
{
    if(itsWritable)
    {
        QString ds(CMisc::dirSyntax(dir));
        TPath   *path=findPath(ds);
 
        if(NULL!=path)
            if(path->orig)
                path->toBeRemoved=true;
            else
                itsPaths.removeRef(path);
    }
}

bool CXConfig::getDirs(QStringList &list)
{
    if(itsOk)
    {
        TPath *path=NULL;
 
        for(path=itsPaths.first(); path; path=itsPaths.next())
            if(!path->toBeRemoved && TPath::DIR==path->type && CMisc::dExists(path->dir))
                list.append(path->dir);

        return true;
    }
    else
        return false;
}

bool CXConfig::xfsInPath()
{
    if(itsOk && XF86==itsType)
    {
        TPath *path=NULL;

        for(path=itsPaths.first(); path; path=itsPaths.next())
            if(TPath::FONT_SERVER==path->type)
                return true;
    }

    return false;
}

void CXConfig::refreshPaths()
{
    KFI_DBUG << "CXConfig::refreshPaths()" << endl;

    if(itsOk && XFS!=itsType)
    {
        TPath *path=NULL;
 
        for(path=itsPaths.first(); path; path=itsPaths.next())
        {
            QString dir(path->unscaled
                        ? CMisc::xDirSyntax(path->dir)+QString(UNSCALED)
                        : CMisc::xDirSyntax(path->dir));

            if(path->orig)
            {
                KFI_DBUG << "xset fp- " << dir << endl;
                CMisc::doCmd("xset", "fp-", dir); // Remove path...
            }
            if(!path->toBeRemoved && CMisc::dExists(path->dir) && CMisc::fExists(path->dir+"fonts.dir"))
            {
                ifstream in(QFile::encodeName(path->dir+"fonts.dir"));

                if(in)
                {
                    int num;

                    in >> num;

                    if(in.good() && num)
                    {
                        KFI_DBUG << "xset fp+ " << dir << endl;
                        CMisc::doCmd("xset", "fp+", dir);   // Add path...
                    }
                }
            }
        }
    }

    if(CMisc::root() && XFS==itsType)
    {
        unsigned int xfsPid=kfi_getPid("xfs", 1);

        if(xfsPid)
        {
            QString pid;

            KFI_DBUG << "kill -SIGUSR1 " << pid << endl;
            CMisc::doCmd("kill", "-SIGUSR1", pid.setNum(xfsPid));
        }
    }
    else
    {
        KFI_DBUG << "xset fp rehash" << endl;
        CMisc::doCmd("xset", "fp", "rehash");
    }
}

CXConfig::TPath * CXConfig::findPath(const QString &dir)
{
    TPath   *path=NULL;
    QString ds(CMisc::dirSyntax(dir));
 
    for(path=itsPaths.first(); path; path=itsPaths.next())
        if(path->dir==ds)
            return path;
 
    return NULL;
}

static void processPath(char *str, QString &path, bool &unscaled)
{
    char *unsc=NULL;

    unscaled=false;
 
    if(NULL!=(unsc=strstr(str, UNSCALED)))
    {
        *unsc='\0';
        unscaled=true;
    }
 
    path=str;

    if(str[strlen(str)-1]!='/')
        path+="/";
}

bool CXConfig::readFontpaths()
{
    //
    // Fontpaths is a custom Kfontinst format, specified as:
    // <paths...>

    bool     status=false;
    ifstream cfg(QFile::encodeName(itsFileName));

    if(cfg)
    {
        itsTime=CMisc::getTimeStamp(itsFileName);

        static const int constMaxLineLen=1024;  // Should be enough for 1 line!
 
        char line[constMaxLineLen];
 
        itsPaths.clear();
        status=true;

        do
        {
            cfg.getline(line, constMaxLineLen);

            if(cfg.good())
            {
                line[constMaxLineLen-1]='\0';

                if('#'!=line[0])
                {
                    QString path;
                    bool    unscaled;

                    processPath(line, path, unscaled);

                    if(NULL==findPath(path))
                        itsPaths.append(new TPath(KXftConfig::expandHome(path), false, TPath::DIR, true));
                }
            }
        }
        while(!cfg.eof());
        cfg.close();
    }
    else
        if(!CMisc::fExists(itsFileName) && CMisc::dWritable(CMisc::getDir(itsFileName)))
            status=true;

    return status;
}

bool CXConfig::writeFontpaths()
{
    bool     status=false;
    ofstream cfg(QFile::encodeName(itsFileName));
 
    if(cfg)
    {
        TPath *path;

        status=true;
        cfg << constFontpaths << endl;
        for(path=itsPaths.first(); path; path=itsPaths.next())
            if(!path->toBeRemoved && CMisc::dExists(path->dir))
                cfg << QFile::encodeName(KXftConfig::contractHome(CMisc::xDirSyntax(path->dir))) << endl;

        cfg.close();
    }
 
    return status;
}

inline bool isWhitespace(char ch)
{
    return (' '==ch || '\t'==ch || '\n'==ch) ? true : false;
}
 
static unsigned int commentChars(char *buffer)
{
    unsigned int num=0;
 
    if(buffer[0]=='#')
        for(num=1; num<strlen(buffer)+1; ++num)
            if(buffer[num]=='\n' || buffer[num]=='\0')
                break;
 
    return num;
}

static bool commentedOut(char *buffer, char *sect)
{
    if(sect!=buffer && '\n'!=*(sect-1))
    {
        char *ch;

        for(ch=sect-1; ch>=buffer; ch--)
            if(*ch=='\n')
                break;
            else if(*ch=='#')
                return true;
    }

    return false;
}

static char * locateSection(char *buffer, const char *section)
{
    const char *sectionMarker   ="Section";
    const int   sectionMarkerLen=7;

    char *s=NULL,
         *buf=buffer;

    do
    {
        s=strstr(buf, sectionMarker);

        if(s)
        {
            bool com=commentedOut(buffer, s);

            buf=s+sectionMarkerLen;
            if(com)
                s=NULL;
            else
            {
                // Skip any whitespace
                for(s+=sectionMarkerLen; s && isWhitespace(*s); s++)
                    ;

                // Now check section type
                if(s && s==strstr(s, section))   // If found, then again skip past whitespace
                    for(s+=strlen(section); s && isWhitespace(*s); s++)
                        ;
                else
                    s=NULL;
            }
        }
        else
            break;
    }
    while(!s);

    return s;
}

static const char *endSectionMarker   ="EndSection";
static const int   endSectionMarkerLen=10;

static char *locateEndSection(char *buffer)
{
    char *s=NULL,
         *buf=buffer;

    do
    {
        s=strstr(buf, endSectionMarker);

        if(s)
        {
            bool com=commentedOut(buffer, s);

            buf=s+endSectionMarkerLen;
            if(com)
                s=NULL;
        }
        else
            break;
    }
    while(!s);

    return s;
}

static char * getItem(char **start, char **end, const char *key, unsigned int &size, bool remove, char *buffer)
{
    static const int constMaxItemLen = 1024;
    static char      item[constMaxItemLen+1];

    unsigned int keyLen=strlen(key);

    char *s=NULL,
         *buf=*start;

    do
    {
        s=strstr(buf, key);

        if(s && s<*end)
        {
            bool com=commentedOut(buf, s);

            buf=s+keyLen;
            if(com)
                s=NULL;
            else
            {
                char *beg=s;
                // Skip any whitespace
                for(s+=keyLen; s && isWhitespace(*s); s++)
                    ;

                if(s && *s=='\"' && s<*end)
                {
                    char *e=strchr(s+1, '\"'),
                         *nl=strchr(s+1, '\n');

                    if(e && e<*end && (!nl || nl>e) && e-s<=constMaxItemLen)
                    {
                        memcpy(item, s+1, (e-s)-1);
                        item[(e-s)-1]='\0';

                        if(remove)
                        {
                            for(beg--; beg>=buffer && *beg!='\n' && *beg !='\"'; beg--)
                                ;
                            if(!nl)
                                nl=e+1;
                            memmove(beg, nl, ((buffer+size)-nl)+1);
                            size-=nl-beg;
                            *end-=nl-beg;
                        }
                        else
                            *start=e+1;

                        return item;
                    }
                    else
                        s=NULL;
                }
                else
                    s=NULL;
            }
        }
        else
            break;
    }
    while(!s);

    return NULL;
}

bool CXConfig::processXf86(bool read)
{
    ifstream xf86(QFile::encodeName(itsFileName));
    bool     ok=false;

    if(xf86)
    {
        itsTime=CMisc::getTimeStamp(itsFileName);

        bool closed=false;

        xf86.seekg(0, ios::end);
        unsigned int size=(streamoff) xf86.tellg();

        if(read)
            itsPaths.clear();

        if(size<65536) // Just incase...
        {
            char *buffer=new char [size+1];

            if(buffer)
            {
                xf86.seekg(0, ios::beg);
                xf86.read(buffer, size);

                if(xf86.good())
                {
                    char *filesStart=NULL,
                         *filesEnd=NULL;

                    closed=true;
                    xf86.close();
                    buffer[size]='\0';

                    if(NULL!=(filesStart=locateSection(buffer, "\"Files\"")) && NULL!=(filesEnd=locateEndSection(filesStart)))
                    {
                        char *pos=filesStart,
                             *item;

                        while(NULL!=(item=getItem(&pos, &filesEnd, "FontPath", size, !read, buffer)))
                            if(read) // Then save paths...
                            {
                                QString path;
                                bool    unscaled;

                                processPath(item, path, unscaled);

                                if(NULL==findPath(path))
                                    itsPaths.append(new TPath(path, unscaled, TPath::getType(path), true));

                            }

                        if(read)
                            ok=true;
                        else
                        {
                            CMisc::createBackup(itsFileName);

                            ofstream of(QFile::encodeName(itsFileName));

                            if(of)
                            {
                                char  *from=buffer,
                                      *modStart=NULL,
                                      *modEnd=NULL;
                                bool  foundFt=false;
                                TPath *path;

                                // Check if "freetype" OR "xtt" is loaded for usage of TTF's
                                if(NULL!=(modStart=locateSection(buffer, "\"Module\"")) && NULL!=(modEnd=locateEndSection(modStart)))
                                { 
                                    pos=modStart;

                                    while(NULL!=(item=getItem(&pos, &modEnd, "Load", size, false, buffer)) && !foundFt)
                                        if(0==strcmp(item, "freetype") || 0==strcmp(item, "xtt"))
                                            foundFt=true;
                                }

                                if(!foundFt && modStart && modEnd && modStart<filesStart) // Then write mod section first...
                                {
                                    of.write(from, modEnd-from);
                                    if(!foundFt)
                                        of << "    Load \"freetype\"\n";    // CPD TODO: Which is better xtt of freetype? Perhaps check locale?
                                    of.write(modEnd, endSectionMarkerLen);
                                    from=modEnd+endSectionMarkerLen;
                                }

                                of.write(from, filesEnd-from);

                                for(path=itsPaths.first(); path; path=itsPaths.next())
                                    if(!path->toBeRemoved && (TPath::DIR!=path->type || CMisc::dExists(path->dir)))
                                    {
                                        of << "    FontPath \t\"";
                                        of << QFile::encodeName(CMisc::xDirSyntax(path->dir));
                                        if(path->unscaled)
                                            of << UNSCALED;
                                        of << "\"\n";
                                    }

                                of.write(filesEnd, endSectionMarkerLen);
                                from=filesEnd+endSectionMarkerLen;

                                if(!foundFt && modStart && modEnd && modStart>filesStart) // Then write mod section last...
                                {
                                    of.write(from, modEnd-from);
                                    if(!foundFt)
                                        of << "    Load \"freetype\"\n";
                                    of.write(modEnd, endSectionMarkerLen);
                                    from=modEnd+endSectionMarkerLen;
                                }
                                if(((unsigned int)(from-buffer))<size)
                                    of.write(from, size-(from-buffer));
                                of.close();
                                ok=true;
                            }
                        }
                    }
                }
                delete [] buffer;
            }
        }
        if(!closed)
            xf86.close();
    }

    return ok;
}
 
static bool isXfsKey(const char *str)
{
    static const char *constKeys[]=
    {
        "alternate-servers",
        "cache-balance",
        "cache-hi-mark",
        "cache-low-mark",
        "catalogue",
        "client-limit",
        "clone-self",
        "default-point-size",
        "default-resolutions",
        "deferglyphs",
        "error-file",
        "no-listen",
        "port",
        "server-number",
        "snf-format",
        "trusted-clients",
        "use-syslog",
        NULL
    };
 
    for(unsigned int key=0; NULL!=constKeys[key]; ++key)
        if(strstr(str, constKeys[key])==str)
        {
            unsigned int sLen=strlen(str),
                         kLen=strlen(constKeys[key]);
 
            if(sLen>kLen && isWhitespace(str[kLen]) || '\0'==str[kLen] || '#'==str[kLen] || '='==str[kLen])
                return true;
        }
 
    return false;
}

static char * getXfsPath(char *buffer, unsigned int &totalSize, unsigned int offsetSize)
{
    // Remove & return a path from the buffer
    const unsigned int constMaxPathLen=8192;
 
    static char path[constMaxPathLen];
    bool        found=false;

    if(offsetSize<totalSize) // Just to make sure soething hasn't gone horribly wrong!
    {
        unsigned int i;
 
        for(i=0; i<offsetSize && !found; i++)
            if(!isWhitespace(buffer[i]) && ','!=buffer[i])
            {
                unsigned int comChars=commentChars(&buffer[i]);
 
                if(comChars)
                    i+=comChars;
                else
                    if(isXfsKey(&buffer[i]))
                        break;
                    else
                    {
                        // A path is terminated by either a comma, another key, or eof...
 
                        unsigned int j=0;
 
                        for(j=1; j<offsetSize-i && !found; j++)
                            if(buffer[i+j]==',' || buffer[i+j]=='\n' || buffer[i+j]=='\0' || isXfsKey(&buffer[i+j]))
                            {
                                if(j>0 && j<constMaxPathLen)
                                {
                                    memcpy(path, &buffer[i], j);
                                    path[j]='\0';
                                    if(buffer[i+j]==',')
                                        j++;
                                    memmove(buffer, &buffer[i+j], (offsetSize-(i+j))+1);
                                    totalSize-=(i+j);
                                    found=true;
                                }
                            }
                    }
            }
    }

    return found ? path : NULL;
}

bool CXConfig::processXfs(bool read)
{
    ifstream xfs(QFile::encodeName(itsFileName));
    bool     ok=false;
 
    if(xfs)
    {
        itsTime=CMisc::getTimeStamp(itsFileName);

        bool closed=false;
 
        xfs.seekg(0, ios::end);
        unsigned int size= (streamoff) xfs.tellg();

        if(read)
            itsPaths.clear();
 
        if(size<32768) // Just incase...
        {
            char *buffer=new char [size+1];
 
            if(buffer)
            {
                xfs.seekg(0, ios::beg);
                xfs.read(buffer, size);
 
                if(xfs.good())
                {
                    const char *constCatalogueStr="catalogue";
                    char *cat=NULL;
                    bool found=false,
                         formatError=false;
 
                    closed=true;
                    xfs.close();
                    buffer[size]='\0';
 
                    // Now remove the directory lists from the buffer...
                    do
                        if(NULL!=(cat=strstr(buffer, constCatalogueStr)))
                        {
                            cat+=strlen(constCatalogueStr);
 
                            if(!isWhitespace(*(cat-1)))
                            {
                                // Check it's not been commented out - by searching back until we get to the start of the buffer,
                                // a carriage-return, or a hash...
 
                                if(!commentedOut(buffer, cat))
                                {
                                    // Look for '='
                                    unsigned int i;
 
                                    for(i=1; i<size-(cat-buffer) && !found && !formatError; ++i)
                                        if(!isWhitespace(cat[i]))
                                        {
                                            unsigned int comChars=commentChars(&cat[i]);
 
                                            if(comChars)
                                                i+=comChars;
                                            else
                                                if(cat[i]!='=' || i+1>=size-(cat-buffer))
                                                    formatError=true;
                                                else
                                                {
                                                    char *path;
 
                                                    cat=&cat[i+1]; // skip equals sign
                                                    while(NULL!=(path=getXfsPath(cat, size, size-(cat-buffer))))
                                                        if(read)
                                                        {
                                                            QString str;
                                                            bool    unscaled;
                                                            processPath(path, str, unscaled);
 
                                                            if(NULL==findPath(path))
                                                                itsPaths.append(new TPath(str, unscaled, TPath::DIR, true));
                                                        }
 
                                                    if(!read) // then must be write...
                                                    {
                                                        CMisc::createBackup(itsFileName);

                                                        ofstream of(QFile::encodeName(itsFileName));
 
                                                        if(of)
                                                        {
                                                            bool  first=true;
                                                            TPath *p=NULL;

                                                            of.write(buffer, cat-buffer);
                                                            of << ' ';
                                                            for(p=itsPaths.first(); p; p=itsPaths.next())
                                                                if(!p->toBeRemoved && CMisc::dExists(p->dir))
                                                                {
                                                                    if(!first)
                                                                    {
                                                                        of << ',';
                                                                        of << endl;
                                                                    }
                                                                    of << QFile::encodeName(CMisc::xDirSyntax(p->dir));
                                                                    if(p->unscaled)
                                                                        of << UNSCALED;
                                                                    first=false;
                                                                }
                                                            of.write(cat, size-(cat-buffer));
                                                            of.close();
                                                            ok=true;
                                                        }
                                                    }
                                                    else
                                                        ok=true;
 
                                                    found=true;
                                                }
                                        }
                                }
                            }
                        }
                    while(NULL!=cat && !found && !formatError);
                }
                delete [] buffer;
            }
        }
        if(!closed)
            xfs.close();
    }
 
    return ok;
}

#ifdef HAVE_FONTCONFIG
bool CXConfig::createFontsDotDir(const QString &dir)
#else
bool CXConfig::createFontsDotDir(const QString &dir, QStringList &symbolFamilies)
#endif
{
    KFI_DBUG << "CXConfig::createFontsDotDir(" << dir << ')' << endl;

    bool status=false;
    QDir d(dir);
 
    if(d.isReadable())
    {
        CFontsFile          fd(QFile::encodeName(QString(dir+"fonts.dir"))),
                            fs(QFile::encodeName(QString(dir+"fonts.scale")));
        const QFileInfoList *files=d.entryInfoList();
        QStringList         fdir,
                            fscale;
        const QStringList   *origfd=NULL,
                            *origfs=NULL;
        bool                added=false;
 
        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;
 
            for(; NULL!=(fInfo=it.current()); ++it)
            {
                QCString cName(QFile::encodeName(fInfo->fileName()));

                if("."!=fInfo->fileName() && ".."!=fInfo->fileName() && CFontEngine::isAFont(cName))
                {
                    bool bitmap=CFontEngine::isABitmap(cName);

                    origfd=fd.getXlfds(fInfo->fileName());
                    origfs=fs.getXlfds(fInfo->fileName());

                    if(origfd)
                    {
                        KFI_DBUG << "Use origfd entry for fdir for " << fInfo->fileName() << endl;
                        fdir+=*origfd;
                    }
                    else if(origfs)
                    {
                        KFI_DBUG << "Use origfs entry for fdir for " << fInfo->fileName() << endl;
                        fdir+=*origfs;
                    }

                    if(origfs)
                    {
                        KFI_DBUG << "Use origfs entry for fscale for " << fInfo->fileName() << endl;
                        fscale+=*origfs;
                    }
                    else if(origfd && !bitmap)
                    {
                        KFI_DBUG << "Use origfd entry for fscale for " << fInfo->fileName() << endl;
                        fscale+=*origfd;
                    }

                    if(!origfd && !origfs)
                    {
                        KFI_DBUG << "Need to create entries for " << fInfo->fileName() << endl;
                        if(!bitmap)
                        {
                            int face=0,
                                numFaces=0;

                            do
                            {
                                if(CGlobal::fe().openFont(fInfo->filePath(), CFontEngine::NAME|CFontEngine::XLFD|CFontEngine::PROPERTIES, false, face))
                                {
                                     QStringList encodings=CGlobal::fe().getEncodings();

                                     numFaces=CGlobal::fe().getNumFaces();   // Only really for TTC files..

                                     if(encodings.count())
                                     {
                                         QCString xlfd;
                                         QString  family=CGlobal::fe().getFamilyName();

                                         if(face>0)  // Again, only really for TTC files...
                                         {
                                             QCString fc;
                                             fc.setNum(face);
                                             xlfd+=':';
                                             xlfd+=fc;
                                             xlfd+=':';
                                         }

                                         xlfd+=QFile::encodeName(fInfo->fileName());
                                         xlfd+=" -";
                                         xlfd+=CGlobal::fe().getFoundry().latin1();
                                         xlfd+="-";
                                         xlfd+=family.latin1();
                                         xlfd+="-";
                                         xlfd+=CFontEngine::weightStr(CGlobal::fe().getWeight()).latin1();
                                         xlfd+="-";
                                         xlfd+=CFontEngine::italicStr(CGlobal::fe().getItalic()).latin1();
                                         xlfd+="-";
                                         xlfd+=CFontEngine::widthStr(CGlobal::fe().getWidth()).latin1();
                                         xlfd+="-";
                                         if(!CGlobal::fe().getAddStyle().isEmpty())
                                             xlfd+=CGlobal::fe().getAddStyle().latin1();
                                         xlfd+="-0-0-0-0-";

                                         QStringList::Iterator it;

                                         for(it=encodings.begin(); it!=encodings.end(); ++it)
                                         {
                                             QCString entry(xlfd);

                                             // Taken from ttmkfdir...
                                             if((*it).find("jisx")!=-1 || (*it).find("gb2312")!=-1 || (*it).find("big5")!=-1 || (*it).find("ksc")!=-1)
                                                 entry+='c';
                                             else
                                                 entry+=CFontEngine::spacingStr(CGlobal::fe().getSpacing()).latin1();

                                             entry+="-0-";
                                             entry+=(*it).latin1();

                                             if(-1==fscale.findIndex(entry))
                                                 fscale.append(entry);
                                             if(-1==fdir.findIndex(entry))
                                                 fdir.append(entry);
                                             added=true;
#ifndef HAVE_FONTCONFIG
                                             if(CFontEngine::isATtf(QFile::encodeName(fInfo->fileName())) &&
                                                CEncodings::constTTSymbol==*it &&
                                                !symbolFamilies.contains(family))
                                                 symbolFamilies.append(family);
#endif
                                         }
                                     }
                                     CGlobal::fe().closeFont();
                                 }
                             }
                             while(++face<numFaces);
                         }
                         else
                             if(CGlobal::fe().openFont(fInfo->filePath(), CFontEngine::XLFD)) // Bitmap fonts contain Xlfd embedded within...
                             {
                                 QCString entry(QFile::encodeName(fInfo->fileName()));

                                 entry+=" ";
                                 entry+=CGlobal::fe().getXlfdBmp().latin1();
                                 fdir.append(entry);
                                 CGlobal::fe().closeFont();
                                 added=true;
                             }
                      }
                }
            }
        }

        //
        // Only output if we have added something, or the enumber of Xlfds is different (would mean a font was
        // removed)
        if(added || fdir.count()!=fd.xlfdCount())
        {
            KFI_DBUG << "Ouput fonts.dir, " << added << ' ' << fdir.count() << ' ' << fd.xlfdCount() << endl;

            ofstream fontsDotDir(QFile::encodeName(QString(dir+"fonts.dir")));

            if(fontsDotDir)
            {
                QStringList::Iterator sIt;

                fontsDotDir << fdir.count() << endl;
                for(sIt=fdir.begin(); sIt!=fdir.end(); ++sIt)
                    fontsDotDir << (*sIt).local8Bit() << endl;

                fontsDotDir.close();
            }
            status=true;
        }

        if(added || fscale.count()!=fs.xlfdCount())
        {
            KFI_DBUG << "Ouput fonts.scale, " << added << ' ' << fscale.count() << ' ' << fs.xlfdCount() << endl;

            ofstream fontsDotScale(QFile::encodeName(QString(dir+"fonts.scale")));

            if(fontsDotScale)
            {
                QStringList::Iterator sIt;

                fontsDotScale << fscale.count() << endl;
                for(sIt=fscale.begin(); sIt!=fscale.end(); ++sIt)
                    fontsDotScale << (*sIt).local8Bit() << endl;

                fontsDotScale.close();
            }
        }
    }

    return status;
}

CXConfig::TPath::EType CXConfig::TPath::getType(const QString &d)
{
    QString str(d);

    return str.replace(QRegExp("\\s*"), "").find("unix/:")==0
               ? FONT_SERVER
               : DIR;
}

CXConfig::CFontsFile::CFontsFile(const char *file)
                    : itsXlfdCount(0)
{
    ifstream f(file);

    itsEntries.setAutoDelete(true);

    if(f)
    {
        const int constMaxLine=512;

        char   line[constMaxLine+1];
        TEntry *current=NULL;

        f.getline(line, constMaxLine); // Ignore # of entries

        while(!f.eof())
        {
            f.getline(line, constMaxLine);
            if(!f.eof())
            {
                // Assume entries are:
                //    <file>.<ext>   -<xlfd>
                char *dot=strchr(line, '.'),
                     *dash=dot ? strchr(dot, '-') : NULL;

                if(dash)
                {
                    itsXlfdCount++;

                    QString xlfd(dash);

                    *dash='\0';

                    //
                    // Check for ttc files...
                    QString fname(QString(line).stripWhiteSpace()), 
                            mod;
                    int     c1=fname.find(':'),
                            c2=-1!=c1 ? fname.findRev(':') : -1;

                    if(-1!=c2 && c1!=c2)
                    {
                        mod=fname.mid(c1+1, c2-1);
                        fname.remove(0, c2+1);
                    }

                    TEntry *entry=getEntry(&current, fname);

                    if(entry)
                        if(mod.isEmpty())
                            entry->xlfds.append(entry->filename+" "+xlfd);
                        else
                            entry->xlfds.append(mod+entry->filename+" "+xlfd);
                }
            }
        }
        f.close();
    }
}

const QStringList * CXConfig::CFontsFile::getXlfds(const QString &fname)
{
    TEntry *entry=findEntry(fname);

    return entry ? &entry->xlfds : NULL;
}

CXConfig::CFontsFile::TEntry * CXConfig::CFontsFile::findEntry(const QString &fname)
{
    TEntry *entry=NULL;

    for(entry=itsEntries.first(); entry; entry=itsEntries.next())
        if(entry->filename==fname)
            break;

    return entry;
}

CXConfig::CFontsFile::TEntry * CXConfig::CFontsFile::getEntry(TEntry **current, const QString &filename)
{
    //
    // See if its the current one...
    if(*current && (*current)->filename==filename)
        return *current;

    //
    // See if its already known...
    TEntry *entry=findEntry(filename);

    //
    // If not found, then create a new entry
    if(!entry)
    {
        entry=new TEntry(filename);
        itsEntries.append(entry);
    }

    *current=entry;
    return entry;
}

