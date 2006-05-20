////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CXConfig
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "XConfig.h"
#include "FontEngine.h"
#include "kxftconfig.h"
#include <fstream>
#include <string.h>
#include <QDir>
#include <QRegExp>
#include <klocale.h>
#include <sys/types.h>
#include <signal.h>

#if defined OS_Irix || defined OS_Solaris
extern "C" unsigned int kfi_getPid(const char *proc, pid_t ppid);
#else
extern "C" unsigned int kfi_getPid(const char *proc, unsigned int ppid);
#endif 

#define UNSCALED ":unscaled"

using namespace std;

namespace KFI
{

CXConfig::CXConfig(EType type, const QString &file)
        : itsType(type),
          itsFileName(file),
          itsOk(false),
          itsWritable(false)
{
    itsPaths.setAutoDelete(true);
    readConfig();
}

bool CXConfig::configureDir(const QString &dir, CFontEngine &fe)
{
    bool status=createFontsDotDir(dir, fe);
#ifdef HAVE_FONT_ENC
    if(status)
        status=CEncodings::createEncodingsDotDir(dir);
#endif
    return status;
}

bool CXConfig::readConfig()
{
    itsOk=false;

    switch(itsType)
    {
        case XFS:
            itsOk=processXfs(true);
            break;
        case X11:
            itsOk=processX11(true);
            break;
    }

    if(itsOk)
        itsWritable=Misc::fExists(itsFileName) ? Misc::fWritable(itsFileName)
                                               : Misc::dWritable(Misc::getDir(itsFileName));
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
    if(Misc::fExists(itsFileName) && Misc::getTimeStamp(itsFileName)!=itsTime)
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
            case X11:
                written=processX11(false);
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
            if(!path->orig)
                return true;
    }

    return false;
}
 
void CXConfig::addPath(const QString &dir, bool unscaled)
{
    if(itsWritable)
    {
        QString ds(Misc::dirSyntax(dir));

        if(Misc::dExists(dir))
        {
            TPath *path=findPath(ds);

            if(NULL==path)
                itsPaths.append(new TPath(ds, unscaled, TPath::DIR, false));
        }
    }
}

bool CXConfig::inPath(TPath::EType type)
{
    if(itsOk && X11==itsType)
    {
        TPath *path=NULL;

        for(path=itsPaths.first(); path; path=itsPaths.next())
            if(type==path->type)
                return true;
    }

    return false;
}

void CXConfig::refreshPaths(bool xfs)
{ 
    if(xfs)
    {
        if(Misc::root())
        {
            unsigned int xfsPid=kfi_getPid("xfs", 1);

            if(xfsPid)
            { 
                QString pid;

                kill(xfsPid, SIGUSR1);
            }
        }
    }
    else
        Misc::doCmd("xset", "fp", "rehash");
}

CXConfig::TPath * CXConfig::findPath(const QString &dir)
{
    TPath   *path=NULL;
    QString ds(Misc::dirSyntax(dir));
 
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
        path+='/';
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

bool CXConfig::processX11(bool read)
{
    ifstream x11(QFile::encodeName(itsFileName));
    bool     ok=false;

    if(x11)
    {
        itsTime=Misc::getTimeStamp(itsFileName);

        bool closed=false;

        x11.seekg(0, ios::end);
        unsigned int size=(streamoff) x11.tellg();

        if(read)
            itsPaths.clear();

        if(size<65536) // Just incase...
        {
            char *buffer=new char [size+1];

            if(buffer)
            {
                x11.seekg(0, ios::beg);
                x11.read(buffer, size);

                if(x11.good())
                {
                    char *filesStart=NULL,
                         *filesEnd=NULL;

                    closed=true;
                    x11.close();
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
                                    itsPaths.append(new TPath(path, unscaled, TPath::getType(path)));

                            }

                        if(read)
                            ok=true;
                        else
                        {
                            Misc::createBackup(itsFileName);

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
                                    if(TPath::DIR!=path->type || Misc::dExists(path->dir))
                                    {
                                        of << "    FontPath \t\"";
                                        of << QFile::encodeName(Misc::xDirSyntax(path->dir)).constData();
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
            x11.close();
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
        itsTime=Misc::getTimeStamp(itsFileName);

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
                                                                itsPaths.append(new TPath(str, unscaled));
                                                        }
 
                                                    if(!read) // then must be write...
                                                    {
                                                        Misc::createBackup(itsFileName);

                                                        ofstream of(QFile::encodeName(itsFileName));
 
                                                        if(of)
                                                        {
                                                            bool  first=true;
                                                            TPath *p=NULL;

                                                            of.write(buffer, cat-buffer);
                                                            of << ' ';
                                                            for(p=itsPaths.first(); p; p=itsPaths.next())
                                                                if(Misc::dExists(p->dir))
                                                                {
                                                                    if(!first)
                                                                    {
                                                                        of << ',';
                                                                        of << endl;
                                                                    }
                                                                    of << QFile::encodeName(Misc::xDirSyntax(p->dir)).constData();
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

bool CXConfig::createFontsDotDir(const QString &dir, CFontEngine &fe)
{
    QString ds(Misc::dirSyntax(dir));
    QDir    d(ds);
    bool    status=true;

    if(d.isReadable())
    {
        CFontsFile    fd(QFile::encodeName(QString(ds+"fonts.dir"))),
                      fs(QFile::encodeName(QString(ds+"fonts.scale")));
        QStringList   fdir,
                      fscale;
        bool          addedFd=false,
                      addedFs=false;
 
        foreach(QFileInfo fInfo, d.entryInfoList())
        {
            QByteArray        cName(QFile::encodeName(fInfo.fileName()));
            const QStringList *origfd=NULL,
                              *origfs=NULL;

            if("."!=fInfo.fileName() && ".."!=fInfo.fileName())
            {
                origfd=fd.getXlfds(fInfo.fileName());
                origfs=fs.getXlfds(fInfo.fileName());

                if(origfd)
                    fdir+=*origfd;
                else if(origfs)
                    fdir+=*origfs;

                if(origfs)
                    fscale+=*origfs;

                if(!origfd && !origfs)
                {
                    int face=0,
                        numFaces=0;

                    do
                    {
                        if(fe.openFont(fInfo.filePath(), face))
                        {
                            if(CFontEngine::BITMAP!=fe.getType())
                            {
                                 QStringList encodings=fe.getEncodings();

                                 numFaces=fe.getNumFaces();   // Only really for TTC files..

                                 if(!encodings.isEmpty())
                                 {
                                     QByteArray xlfd;
                                     QString    family=fe.getFamilyName();

                                     if(face>0)  // Again, only really for TTC files...
                                     {
                                         xlfd+=':';
                                         xlfd+=QByteArray::number(face);
                                         xlfd+=':';
                                     }

                                     xlfd+=QFile::encodeName(fInfo.fileName());
                                     xlfd+=" -";
                                     xlfd+=fe.getFoundry().toLatin1();
                                     xlfd+='-';
                                     xlfd+=family.toLatin1();
                                     xlfd+='-';
                                     xlfd+=CFontEngine::weightStr(fe.getWeight()).toLatin1();
                                     xlfd+='-';
                                     xlfd+=CFontEngine::italicStr(fe.getItalic()).toLatin1();
                                     xlfd+='-';
                                     xlfd+=CFontEngine::widthStr(fe.getWidth()).toLatin1();
                                     xlfd+='-';
                                     if(!fe.getAddStyle().isEmpty())
                                         xlfd+=fe.getAddStyle().toLatin1();
                                     //CPD: TODO Bitmap only ttfs!!!
                                     xlfd+="-0-0-0-0-";

                                     foreach(QString encoding, encodings)
                                     {
                                         QByteArray entry(xlfd);

                                         // Taken from ttmkfdir...
                                         if(encoding.indexOf("jisx")!=-1 || encoding.indexOf("gb2312")!=-1 ||
                                            encoding.indexOf("big5")!=-1 || encoding.indexOf("ksc")!=-1)
                                             entry+='c';
                                         else
                                             entry+=CFontEngine::spacingStr(fe.getSpacing()).toLatin1();

                                         entry+="-0-";
                                         entry+=encoding.toLatin1();

                                         if(-1==fscale.indexOf(entry))
                                             fscale.append(QString(entry));
                                         if(-1==fdir.indexOf(entry))
                                             fdir.append(QString(entry));
                                         addedFd=addedFs=true;
#ifndef HAVE_FONTCONFIG
                                         if(CFontEngine::isATtf(QFile::encodeName(fInfo->fileName())) &&
                                            CEncodings::constTTSymbol==encoding &&
                                            !symbolFamilies.contains(family))
                                             symbolFamilies.append(family);
#endif
                                     }
                                 }
                            }
                            else
                            {
                                QByteArray entry(QFile::encodeName(fInfo.fileName()));

                                entry+=' ';
                                entry+=fe.getXlfdBmp().toLatin1();
                                fdir.append(QString(entry));
                                addedFd=true;
                            }
                            fe.closeFont();
                         }
                     }
                     while(++face<numFaces);
                  }
            }
        }

        //
        // Only output if we have added something, or the enumber of Xlfds is different (would mean a font was
        // removed)
        if(addedFd || fdir.count()!=fd.xlfdCount() || (0==fdir.count() && 0!=fd.xlfdCount()))
        {
            ofstream fontsDotDir(QFile::encodeName(QString(ds+"fonts.dir")));

            if(fontsDotDir)
            {
                QStringList::Iterator sIt;

                fontsDotDir << fdir.count() << endl;
                for(sIt=fdir.begin(); sIt!=fdir.end(); ++sIt)
                    fontsDotDir << (*sIt).toLocal8Bit().constData() << endl;

                fontsDotDir.close();
            }
            else
                status=false;
        }

        if(status && (addedFs || fscale.count()!=fs.xlfdCount()))
        {
            ofstream fontsDotScale(QFile::encodeName(QString(ds+"fonts.scale")));

            if(fontsDotScale)
            {
                QStringList::Iterator sIt;

                fontsDotScale << fscale.count() << endl;
                for(sIt=fscale.begin(); sIt!=fscale.end(); ++sIt)
                    fontsDotScale << (*sIt).toLocal8Bit().constData() << endl;

                fontsDotScale.close();
            }
            else
                status=false;
        }
    }

    return status;
}

CXConfig::TPath::EType CXConfig::TPath::getType(const QString &d)
{
    QString str(d);

    str.replace(QRegExp("\\s*"), "");

    return 0==str.indexOf("unix/:")
               ? FONT_SERVER
               : "fontconfig"==str
                   ? FONT_CONFIG
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
                char *dash=strstr(line, " -");

                if(!dash)
                    dash=strstr(line, "\t-");

                if(dash)
                {
                    itsXlfdCount++;

                    QString xlfd(&dash[1]);

                    *dash='\0';

                    //
                    // Check for ttc files, and ttcap entries...
                    QString fname(QString(line).trimmed()), 
                            mod;
                    int     c1=fname.lastIndexOf(':');

                    if(-1!=c1)
                    {
                        mod=fname.left(c1+1);
                        fname.remove(0, c1+1);
                    }

                    TEntry *entry=getEntry(&current, fname);

                    if(entry)
                        if(mod.isEmpty())
                            entry->xlfds.append(entry->filename+' '+xlfd);
                        else
                            entry->xlfds.append(mod+entry->filename+' '+xlfd);
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

}
