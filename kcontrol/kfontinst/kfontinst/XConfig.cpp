////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXConfig
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
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
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "XConfig.h"
#include "KfiGlobal.h"
#include "Encodings.h"
#include "FontEngine.h"
#include "BufferedFile.h"
#include "Config.h"
#include <fstream.h>
#include <qdir.h>
#include <qcstring.h>
#include <klocale.h>

static const QCString constFontpaths ("# KFontinst fontpaths file -- DO NOT EDIT");

using namespace std;

CXConfig::CXConfig()
        : QObject(NULL, NULL),
          itsType(NONE)
{
    itsPaths.setAutoDelete(true);
    readConfig();
}

bool CXConfig::go(const QString &dir, QStringList &symbolFamilies)
{
    bool status=createFontsDotDir(dir, symbolFamilies);

    if(status)
    {
        emit step(i18n("Creating encodings.dir"));
        CKfiGlobal::enc().createEncodingsDotDir(dir);
    }

    return status;
}

bool CXConfig::readConfig()
{
    if(readFontpaths())
        itsType=KFONTINST;
    else if(readXF86Config())
        itsType=XF86CONFIG;
    else if(readXfsConfig())
        itsType=XFS;
    else
        itsType=NONE;

    if(NONE!=itsType)
        itsWritable=CMisc::fExists(CKfiGlobal::cfg().getXConfigFile()) ? CMisc::fWritable(CKfiGlobal::cfg().getXConfigFile())
                                                                       : CMisc::dWritable(CMisc::getDir(CKfiGlobal::cfg().getXConfigFile()));
    else
        itsWritable=false;

    return NONE!=itsType;
}

bool CXConfig::writeConfig()
{
    bool written=false;

    switch(itsType)
    {
        case KFONTINST:
            written=writeFontpaths();
            break;
        case XF86CONFIG:
            written=writeXF86Config();
            break;
        case XFS:
            written=writeXfsConfig();
            break;
        default:
            written=false;
    }

    if(written)
        readConfig();

    return written;
}

bool CXConfig::madeChanges()
{
    if(NONE!=itsType && itsWritable)
    {
        TPath *path;
 
        for(path=itsPaths.first(); path; path=itsPaths.next())
            if(!path->orig || path->disabled || path->unscaled!=path->origUnscaled)
                return true;
    }

    return false;
}
 
bool CXConfig::inPath(const QString &dir)
{
    TPath *path=findPath(dir);
 
    return NULL==path || path->disabled ? false : true;
}

bool CXConfig::isUnscaled(const QString &dir)
{
    TPath *path=findPath(dir);
 
    return NULL==path || !path->unscaled ? false : true;
}

void CXConfig::setUnscaled(const QString &dir, bool unscaled) 
{
    TPath *path=findPath(dir);

    if(NULL!=path)
        path->unscaled=unscaled;
}

void CXConfig::addPath(const QString &dir, bool unscaled)
{
    if(itsWritable)
    {
        QString ds(CMisc::dirSyntax(dir));
        TPath   *path=findPath(ds);

        if(NULL==path)
            itsPaths.append(new TPath(ds, unscaled, false, false));
        else
            if(path->disabled)
                path->disabled=false;
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
                path->disabled=true;
            else
                itsPaths.removeRef(path);
    }
}

bool CXConfig::getTTandT1Dirs(QStringList &list)
{
    if(NONE!=itsType)
    {
        TPath *path=NULL;
 
        for(path=itsPaths.first(); path; path=itsPaths.next())
            if(!path->disabled && CMisc::dExists(path->dir) && CMisc::dContainsTTorT1Fonts(path->dir))
                list.append(path->dir);

        return true;
    }
    else
        return false;
}

void CXConfig::refreshPaths()
{
    if(NONE!=itsType)
    {
        TPath *path=NULL;
 
        for(path=itsPaths.first(); path; path=itsPaths.next())
        {
            CMisc::doCmd("xset", "fp-", path->dir); // Remove path...
            if(!path->disabled && CMisc::dExists(path->dir) && CMisc::fExists(path->dir+"fonts.dir"))
                CMisc::doCmd("xset", "fp+", path->dir);   // Add path...
        }
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
 
    if(NULL!=(unsc=strstr(str, ":unscaled")))
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
    // constFontpaths
    // <paths...>

    bool     status=false;
    ifstream cfg(CKfiGlobal::cfg().getXConfigFile().local8Bit());

    if(cfg)
    {
        static const int constMaxLineLen=1024;  // Should be enough for 1 line!
 
        char line[constMaxLineLen];
 
        itsPaths.clear();

        cfg.getline(line, constMaxLineLen);

        if(cfg.good())
            if(NULL!=strstr(line, constFontpaths)) 
            {
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
                                itsPaths.append(new TPath(path, false, false, true));
                        }
                    }
                }
                while(!cfg.eof());
            }
        cfg.close();
    }
    else
        if(!CMisc::fExists(CKfiGlobal::cfg().getXConfigFile()) && CMisc::dWritable(CMisc::getDir(CKfiGlobal::cfg().getXConfigFile())))
            status=true;

    if(status && CKfiGlobal::cfg().firstTime() && 0==itsPaths.count())
    {
        itsWritable=true;
        addPath(CKfiGlobal::cfg().getFontsDir()+CKfiGlobal::cfg().getTTSubDir(), false);
        addPath(CKfiGlobal::cfg().getFontsDir()+CKfiGlobal::cfg().getT1SubDir(), false);
    }

    return status;
}

bool CXConfig::writeFontpaths()
{
    bool     status=false;
    ofstream cfg(CKfiGlobal::cfg().getXConfigFile().local8Bit());
 
    if(cfg)
    {
        TPath *path;

        status=true;
        cfg << constFontpaths << endl;
        for(path=itsPaths.first(); path; path=itsPaths.next())
            if(!path->disabled && CMisc::dExists(path->dir))
                cfg << path->dir.local8Bit() << endl;

        cfg.close();
    }
 
    return status;
}

bool CXConfig::readXF86Config()
{
    bool     status=false;
    ifstream cfg(CKfiGlobal::cfg().getXConfigFile().local8Bit());

    if(cfg)
    {
        static const int constMaxLineLen=1024;  // Should be enough for 1 line!
 
        char line[constMaxLineLen],
             str1[constMaxLineLen],
             str2[constMaxLineLen];
        bool inFiles=false;

        itsPaths.clear();
 
        do
        {
            cfg.getline(line, constMaxLineLen);
 
            if(cfg.good())
            {
                line[constMaxLineLen-1]='\0';
 
                if('#'!=line[0] && sscanf(line, "%s %s", str1, str2)==2)
                {
                    if(!inFiles)
                    {
                        if(strcmp(str1, "Section")==0 && strcmp(str2, "\"Files\"")==0)
                        {
                            itsInsertPos=line;
                            inFiles=status=true;
                        }
                    }
                    else
                        if(strcmp(str1, "FontPath")==0)
                        {
                            if(str2[0]=='\"' && str2[1]=='/' && str2[strlen(str2)-1]=='\"')
                            {
                                QString path;
                                bool    unscaled;

                                str2[strlen(str2)-1]='\0';  // Remove trailing quote

                                processPath(&str2[1], path, unscaled);
 
                                if(NULL==findPath(path))
                                    itsPaths.append(new TPath(path, unscaled, false, true));
                            }
                        }
                }
                else
                    if(inFiles && sscanf(line, "%s", str1)==1 && strcmp(str1, "EndSection")==0)
                        break;
            }
        }
        while(!cfg.eof());
        cfg.close();
    }

    return status;
}

bool CXConfig::writeXF86Config()
{
    bool status=false,
         changed=madeChanges();

    if(changed)
    {
        CMisc::createBackup(CKfiGlobal::cfg().getXConfigFile().local8Bit());

        CBufferedFile cfg(CKfiGlobal::cfg().getXConfigFile().local8Bit(), "FontPath", itsInsertPos.latin1(), false, false, true);
       
        if(cfg)
        {
            TPath *path;

            status=true;

            for(path=itsPaths.first(); path; path=itsPaths.next())
                if(!path->disabled && CMisc::dExists(path->dir))
                {
                    QCString str("  FontPath  \t\"");
                    QString  dir(path->dir); 

                    dir.remove(dir.length()-1, 1); // Remove trailing /
                    str+=dir.local8Bit();
                    if(path->unscaled)
                        str+=":unscaled";
                    str+="\"";
                    cfg.writeNoGuard(str);
                }
            cfg.close();
        } 
    }
    else
        status=true;

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

bool CXConfig::processXfs(const QString &fname, bool read)
{
    ifstream xfs(fname.local8Bit());
    bool     ok=false;
 
    if(xfs)
    {
        bool closed=false;
 
        xfs.seekg(0, ios::end);
        unsigned int size=xfs.tellg();

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
 
                                bool comentedOut=false;
 
                                if(cat!=buffer && '\n'!=*(cat-1))
                                {
                                    char *ch;
 
                                    for(ch=cat-1; ch>=buffer && !comentedOut; ch--)
                                        if(*ch=='\n')
                                            break;
                                        if(*ch=='#')
                                            comentedOut=true;
                                }
                                if(!comentedOut)
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
                                                                itsPaths.append(new TPath(str, unscaled, false, true));
                                                        }
 
                                                    if(!read) // then must be write...
                                                    {
                                                        CMisc::createBackup(fname);

                                                        ofstream of(fname.local8Bit());
 
                                                        if(of)
                                                        {
                                                            bool  first=true;
                                                            TPath *p=NULL;

                                                            of.write(buffer, cat-buffer);
                                                            of << ' ';
                                                            for(p=itsPaths.first(); p; p=itsPaths.next())
                                                                if(!p->disabled && CMisc::dExists(p->dir))
                                                                {
                                                                    if(!first)
                                                                    {
                                                                        of << ',';
                                                                        of << endl;
                                                                    }
                                                                    of << p->dir.local8Bit();
                                                                    if(p->unscaled)
                                                                        of << ":unscaled";
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

bool CXConfig::readXfsConfig()
{
    return processXfs(CKfiGlobal::cfg().getXConfigFile(), true);
}

bool CXConfig::writeXfsConfig()
{
    return processXfs(CKfiGlobal::cfg().getXConfigFile(), false);
}

static bool find(const QStringList &list, const QString &val)
{
    QStringList::Iterator it;

    for(it=((QStringList &)list).begin(); it!=((QStringList &)list).end(); ++it)
        if(0==strcmp((*it).latin1(), val.latin1()))
            return true;

    return false;
}

bool CXConfig::createFontsDotDir(const QString &dir, QStringList &symbolFamilies)
{
    bool status=false;
    QDir d(dir);
 
    if(d.isReadable())
    {
        ofstream fontsDotDir(QString(dir+"fonts.dir").local8Bit());

        if(fontsDotDir)
        {
            const QFileInfoList *files=d.entryInfoList();
            QStringList         bitmapFonts,
                                scalableFonts;
 
            if(files)
            {
                QFileInfoListIterator it(*files);
                QFileInfo             *fInfo;
 
                for(; NULL!=(fInfo=it.current()); ++it)
                    if("."!=fInfo->fileName() && ".."!=fInfo->fileName() && CFontEngine::isAFont(fInfo->fileName().local8Bit()))
                    {
                       bool bitmap=CFontEngine::isABitmap(fInfo->fileName().local8Bit());

                       emit step(i18n("Adding %1 to X").arg(fInfo->filePath()));

                       if(!bitmap)
                       {
                           if(CKfiGlobal::fe().openFont(fInfo->filePath(), CFontEngine::NAME|CFontEngine::XLFD|CFontEngine::PROPERTIES))
                           {
                                QStringList encodings=CKfiGlobal::fe().getEncodings();

                                if(encodings.count())
                                {
                                    QCString xlfd(fInfo->fileName().local8Bit());
                                    QString  family=CKfiGlobal::fe().getFamilyName();

                                    xlfd+=" -";
                                    xlfd+=CKfiGlobal::fe().getFoundry().latin1();
                                    xlfd+="-";
                                    xlfd+=family.latin1();
                                    xlfd+="-";
                                    xlfd+=CFontEngine::weightStr(CKfiGlobal::fe().getWeight()).latin1();
                                    xlfd+="-";
                                    xlfd+=CFontEngine::italicStr(CKfiGlobal::fe().getItalic()).latin1();
                                    xlfd+="-";
                                    xlfd+=CFontEngine::widthStr(CKfiGlobal::fe().getWidth()).latin1();
                                    xlfd+="--";
                                    xlfd+="0";
                                    xlfd+="-";
                                    xlfd+="0";
                                    xlfd+="-";
                                    xlfd+="0";
                                    xlfd+="-";
                                    xlfd+="0";
                                    xlfd+="-";

                                    QStringList::Iterator it;

                                    for(it=encodings.begin(); it!=encodings.end(); ++it)
                                    {
                                        QCString entry(xlfd);

                                        // Taken from ttmkfdir...
                                        if((*it).find("jisx")!=-1 || (*it).find("gb2312")!=-1 || (*it).find("big5")!=-1 || (*it).find("ksc")!=-1)
                                            entry+='c';
                                        else
                                            entry+=CFontEngine::spacingStr(CKfiGlobal::fe().getSpacing()).latin1();

                                        entry+="-";
                                        entry+="0";
                                        entry+="-";
                                        if(CEncodings::constUnicodeStr==*it)
                                            entry+="iso10646-1";
                                        else
                                            entry+=(*it).latin1();
                                        scalableFonts.append(entry);

                                        if(CFontEngine::isATtf(fInfo->fileName().local8Bit()) &&
                                           CEncodings::constTTSymbol==*it &&
                                           !find(symbolFamilies, family))
                                            symbolFamilies.append(family);
                                    }
                                }
                                CKfiGlobal::fe().closeFont();
                            }
                        }
                        else
                            if(CKfiGlobal::fe().openFont(fInfo->filePath(), CFontEngine::XLFD)) // Bitmap fonts contain Xlfd embedded within...
                            {
                                QCString entry(fInfo->fileName().local8Bit());

                                entry+=" ";
                                entry+=CKfiGlobal::fe().getXlfdBmp().latin1();
                                bitmapFonts.append(entry);
                                CKfiGlobal::fe().closeFont();
                            }
                    }
            }

            ofstream fontsDotScale;

            if(scalableFonts.count())
                fontsDotScale.open(QString(dir+"fonts.scale").local8Bit());

            fontsDotDir << bitmapFonts.count()+scalableFonts.count() << endl;

            if(fontsDotScale)
                fontsDotScale << scalableFonts.count() << endl;

            QStringList::Iterator sIt;
 
            for(sIt=scalableFonts.begin(); sIt!=scalableFonts.end(); ++sIt)
            {
                fontsDotDir << (*sIt).local8Bit() << endl;
                if(fontsDotScale)
                    fontsDotScale << (*sIt).local8Bit() << endl;
            }

            if(fontsDotScale)
                fontsDotScale.close();

            for(sIt=bitmapFonts.begin(); sIt!=bitmapFonts.end(); ++sIt)
                fontsDotDir << (*sIt).local8Bit() << endl;

            status=true;
            fontsDotDir.close();
        }
    }
 
    return status;
}
#include "XConfig.moc"
