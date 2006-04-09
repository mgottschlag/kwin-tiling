////////////////////////////////////////////////////////////////////////////////
//
// Namespae      : KFI::Fontmap
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 06/06/2003
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
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "Fontmap.h"
#include "FontEngine.h"
#include "XConfig.h"
#include "FcEngine.h"
#include "KfiConstants.h"
#include <qdir.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <fstream>
#include <unistd.h>

using namespace std;

static char * findSpace(char *str)
{
    while(str && *str!=' ' && *str!='\t')
        str++;

    return str;
}

static bool parseLine(const char *line, QString &ps, QString &fname, bool &isAlias)
{
    static const int constMaxLen     = 127;
    static const int constFileMaxLen = 1023;

    //
    // Format:
    // "/<psname>  (<filename>) ; "
    // "/<psname>  /real ; "

    char a[constMaxLen+1],
         b[constFileMaxLen+1];

    char *slash1=strchr(line, '/'),
         *space1=slash1 ? findSpace(slash1) : NULL, //strchr(slash1, ' ') : NULL,
         *ob=slash1 ? strchr(slash1, '(') : NULL,
         *cb=ob ? strchr(ob, ')') : NULL,
         *slash2=space1 && !ob && !cb ? strchr(space1, '/') : NULL,
         *space2=slash2 ? findSpace(slash2) : NULL, // strchr(slash2, ' ') : NULL,
         *semic=cb || space2 ? strchr(cb ? cb : space2, ';') : NULL;

    if(semic && space1-slash1<constMaxLen)
    {
        slash1++;
        memcpy(a, slash1, space1-slash1);
        a[space1-slash1]='\0';

        if(cb && cb-ob<constFileMaxLen)  // Then found a file entry...
        {
            ob++;
            memcpy(b, ob, cb-ob);
            b[cb-ob]='\0';
            ps=a;
            fname=b;
            isAlias=false;
            return true;
        }
        else if(space2 && space2-slash2<constMaxLen) // Then found an alias...
        {
            slash2++;
            memcpy(b, slash2, space2-slash2);
            b[space2-slash2]='\0';
            ps=a;
            fname=b;
            isAlias=true;
            return true;
        }
    }

    return false;
}

//
// Returns a PS name from an X family name...
//    e.g. "Times New Roman" -> "TimesNewRoman"
static QString createX11PsName(const QString &font)
{
    QString       newName(font);
    int  ch;
    bool          newWord=true;

    newName.replace(QRegExp("\\-"), "_");

    for(ch=0; ch<newName.length(); ++ch)
    {
        if(newName[ch].isSpace())
            newWord=true;
        else
        {
            if(newName[ch]==newName[ch].toUpper())
            {
                if(!newWord)
                    newName[ch]=newName[ch].toLower();
            }
            else
                if(newName[ch]==newName[ch].toLower())
                {
                    if(newWord)
                        newName[ch]=newName[ch].toUpper();
                }
            newWord=false;
        }
    }

    newName.replace(" ", QString());
    return newName;
}

static const char * getItalicStr(KFI::CFontEngine::EItalic it)
{
    switch(it)
    {
        default:
        case KFI::CFontEngine::ITALIC_NONE:
            return NULL;
        case KFI::CFontEngine::ITALIC_ITALIC:
            return "Italic";
        case KFI::CFontEngine::ITALIC_OBLIQUE:
            return "Oblique";
    }
}

//
// Create a full Ps name
static QString createName(const QString &family, const QString &weight, const char *italic)
{
    QString      name;
    QTextStream str(&name);

    str << family;
    if(!weight.isEmpty() || NULL!=italic)
    {
        str << '-';
        if(!weight.isEmpty())
            str << weight;
        if(NULL!=italic)
            str << italic;
    }

    return name;
}

static QString getEntry(QStringList &list, const QString &name)
{
    QStringList::Iterator it(list.begin()),
                          end(list.end());

    for( ; it!=end; ++it)
        if(0==(*it).indexOf('/'+name+' '))
            return *it;

    return QString();
}

inline bool isAlias(const QString &entry)
{
    return -1==entry.lastIndexOf(QRegExp(")\\s*;\\s*$"));
}

static void addEntry(QStringList &list, const QString &name, const QString &file, const QString &fmapDir)
{
    QString existing(getEntry(list, name));
    bool    insert=true;

    if(!existing.isEmpty())
        if(isAlias(existing))
            list.removeAll(existing);
        else
            insert=false;

    if(insert)
    {
        QString      entry;
        QTextOStream str(&entry);

        str << '/' << name << " (";

        if(0==file.indexOf(fmapDir))
            str << file.mid(fmapDir.length());
        else
            str << file;

        str << ") ;";
        list.append(entry);
    }
}

static void addAliasEntry(QStringList &list, const QString &x11Name, const QString &psName)
{
    if(x11Name!=psName)
    {
        QString existing(getEntry(list, x11Name));

        if(existing.isEmpty())
        {
            QString      entry;
            QTextOStream str(&entry);

            str << '/' << x11Name << " /" << psName << " ;";
            list.append(entry);
        }
    }
}

static QString locateFile(const char *dir, const char *file, int level=0)
{
    if(level<5)
    {
        QDir d(dir);

        if(d.isReadable())
        {
            QString               str;
            
            foreach(QFileInfo fInfo, d.entryInfoList())
                if("."!=fInfo.fileName() && ".."!=fInfo.fileName())
                    if(fInfo.isDir())
                    {
                        if(!(str=locateFile(QFile::encodeName(fInfo.filePath()+"/"), file, level+1)).isEmpty())
                            return str;
                    }
                    else
                        if(fInfo.fileName()==file)
                            return fInfo.filePath();
        }
    }

    return QString();
}

static QString locateFile(const char *file, const char **dirs)
{
    int     d;
    QString str;

    for(d=0; dirs[d]; ++d)
        if(!(str=locateFile(dirs[d], file)).isEmpty())
            return str;

    return QString();
}

#define FONTMAP "Fontmap"

namespace KFI
{

namespace Fontmap
{

bool create(const QString &dir, CFontEngine &fe)
{
    bool        root(Misc::root()),
                added=false;
    QString     fmapDir(Misc::dirSyntax(root ? KFI_ROOT_CFG_DIR : dir));
    CFile       old(fmapDir);
    QStringList entries;
    int         i;
    FcPattern   *pat = FcPatternCreate();
    FcObjectSet *os = FcObjectSetBuild(FC_FILE, FC_SCALABLE, (void*)0);
    FcFontSet   *fs = FcFontList(0, pat, os);

    FcPatternDestroy(pat);
    FcObjectSetDestroy(os);

    for (i = 0; i<fs->nfont; i++)
    {
        QString fName(Misc::fileSyntax(CFcEngine::getFcString(fs->fonts[i], FC_FILE)));
        FcBool  scalable=FcFalse;

        if(!fName.isEmpty() && (root || dir.isEmpty() || 0==fName.indexOf(dir)) &&
           FcResultMatch==FcPatternGetBool(fs->fonts[i], FC_SCALABLE, 0, &scalable) && scalable)
        {
            const QStringList *existing=old.getEntries(fName);

            if(existing && existing->count())
                entries+=(*existing);
            else
            {
                int face=0,
                    numFaces=0;

                do
                {
                    if(fe.openFont(fName, face))
                    {
                        if(fe.hasPsInfo())
                        {
                            if(0==numFaces)
                                numFaces=fe.getNumFaces();  // Only really for TTC files...

                            //
                            // Add real
                            addEntry(entries, fe.getPsName(), fName, fmapDir);
                            added=true;

                            //
                            // Add fake entries for X11 generated names
                            switch(fe.getWeight())
                            {
                                case CFontEngine::WEIGHT_MEDIUM:
                                case CFontEngine::WEIGHT_REGULAR:
                                {
                                    QString x11Ps(createX11PsName(fe.getFamilyName()));

                                    if(CFontEngine::ITALIC_ITALIC!=fe.getItalic() &&
                                       CFontEngine::ITALIC_OBLIQUE!=fe.getItalic())
                                        addAliasEntry(entries,
                                                      createName(x11Ps, "Roman",
                                                      getItalicStr(fe.getItalic())),
                                                      fe.getPsName());
                                    addAliasEntry(entries,
                                                  createName(x11Ps, NULL, getItalicStr(fe.getItalic())),
                                                  fe.getPsName());
                                    break;
                                }
                                case CFontEngine::WEIGHT_UNKNOWN:
                                    break;
                                default:
                                    addAliasEntry(entries,
                                                  createName(createX11PsName(fe.getFamilyName()),
                                                             CFontEngine::weightStr(fe.getWeight()),
                                                             getItalicStr(fe.getItalic())),
                                                  fe.getPsName());
                            }
                        }
                        fe.closeFont();
                    }
                }
                while(++face<numFaces);
            }
        }
    }

    bool status=true;

    if(added || entries.count()!=old.getLineCount())
    {
        ofstream out(QFile::encodeName(fmapDir+FONTMAP));

        if(out)
        {
            QStringList::Iterator it;

            for(it=entries.begin(); it!=entries.end(); ++it)
                out << (*it).latin1() << endl;
        }
        else
            status=false;
    }

    //
    // Ensure GS's main Fontmap references our file...
    if(root && status)
    {
        static const char * constGhostscriptDirs[]=
        {
            "/usr/share/ghostscript/",
            "/usr/local/share/ghostscript/",
            NULL
        };

        QString gsFile=locateFile(FONTMAP, constGhostscriptDirs);

        if(!gsFile.isEmpty())
        {
            const int constMaxLineLen=1024;
            const char *constRLF=".runlibfile";

            char     line[constMaxLineLen];
            ifstream in(QFile::encodeName(gsFile));

            if(in)
            {
                QByteArray fmap(QFile::encodeName(fmapDir+FONTMAP));
                int      lineNum=0,
                         kfiLine=-1,
                         gsLine=-1,
                         ncLine=-1;

                do
                {
                    in.getline(line, constMaxLineLen);

                    if(in.good())
                    {
                        line[constMaxLineLen-1]='\0';

                        if(strstr(line, fmap.data())!=NULL && strstr(line, constRLF)!=NULL)
                            kfiLine=lineNum;
                        else if(strstr(line, FONTMAP".GS")!=NULL && strstr(line, constRLF)!=NULL)
                            gsLine=lineNum;
                        if(-1==ncLine && '%'!=line[0])
                            ncLine=lineNum;
                        lineNum++;
                    }
                }
                while(!in.eof() && (-1==kfiLine || -1==gsLine));

                //
                // If the file doesn't already say to use our Fontmap file, then tell it to!
                // Also, ensure ours is .runlibfile'd before the main GS one - else problems can occur
                if(-1==kfiLine || kfiLine>gsLine)
                {
                    in.clear();
                    in.seekg(0, ios::end);
                    int size= (streamoff) in.tellg();
                    in.seekg(0, ios::beg);

                    char *buffer=new char[size+strlen(fmap)+strlen(constRLF)+5];

                    if(buffer)
                    {
                        bool added=false;

                        buffer[0]='\0';
                        lineNum=0;

                        do
                        {
                            in.getline(line, constMaxLineLen);

                            if(in.good())
                            {
                                line[constMaxLineLen-1]='\0';

                                if(lineNum>=ncLine && !added)
                                {
                                    strcat(buffer, "(");
                                    strcat(buffer, fmap);
                                    strcat(buffer, ") ");
                                    strcat(buffer, constRLF);
                                    strcat(buffer, "\n");
                                    added=true;
                                }

                                if(lineNum!=kfiLine)
                                {
                                    strcat(buffer, line);
                                    strcat(buffer, "\n");
                                }
                                lineNum++;
                            }
                        }
                        while(!in.eof());

                        in.close();

                        if(added) // Don't re-write GS's Fontmap unless we've actually added something...
                        {
                            ofstream out(QFile::encodeName(gsFile));

                            if(out)
                                out << buffer;
                        }
                        delete [] buffer;
                    }
                }
            }
        }
    }

    return status;
}

CFile::CFile(const QString &dir)
     : itsDir(dir),
       itsLineCount(0)
{
    ifstream f(QFile::encodeName(dir+FONTMAP));

    itsEntries.setAutoDelete(true);

    if(f)
    {
        static const int constMaxLine=512;

        char   line[constMaxLine+1];
        TEntry *current=NULL;

        while(!f.eof())
        {
            f.getline(line, constMaxLine);

            if(!f.eof())
            {
                QString ps,
                        fname;
                bool    isAlias;

                if(parseLine(line, ps, fname, isAlias))
                {
                    itsLineCount++;

                    TEntry *entry=getEntry(&current, fname, isAlias);

                    if(!isAlias && entry && entry->psName.isEmpty())
                        entry->psName=ps;

                    if(entry)
                        entry->entries.append(line);
                }
            }
        }
        f.close();
    }
}

const QStringList * CFile::getEntries(const QString &fname)
{
    TEntry *entry=findEntry(0==fname.indexOf(itsDir) ? fname.mid(itsDir.length()) : fname, false);

    return entry ? &entry->entries : NULL;
}

CFile::TEntry * CFile::findEntry(const QString &fname, bool isAlias)
{
    TEntry *entry=NULL;

    for(entry=itsEntries.first(); entry; entry=itsEntries.next())
        if(isAlias ? entry->psName==fname : entry->filename==fname)
            break;

    return entry;
}

CFile::TEntry * CFile::getEntry(TEntry **current, const QString &fname, bool isAlias)
{
    //
    // See if its the current one...
    if(*current && (isAlias ? (*current)->psName==fname : (*current)->filename==fname))
        return *current;

    //
    // See if its already known...
    TEntry *entry=findEntry(fname, isAlias);

    //
    // If not found, then create a new entry
    if(!entry)
    {
        entry=new TEntry(fname);
        itsEntries.append(entry);
    }

    *current=entry;
    return entry;
}

}

}
