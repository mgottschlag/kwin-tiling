////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontmap
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////

#include "KfiConfig.h"
#include "Fontmap.h"
#include "Global.h"
#include "FontEngine.h"
#include "XConfig.h"
#include <qdir.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <qregexp.h>
#include <fstream>
#include <unistd.h>

using namespace std;

char * findSpace(char *str)
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
    unsigned int  ch;
    bool          newWord=true;

    newName.replace(QRegExp("\\-"), "_");

    for(ch=0; ch<newName.length(); ++ch)
    {
        if(newName[ch].isSpace())
            newWord=true;
        else
        {
            if(newName[ch]==newName[ch].upper())
            {
                if(!newWord)
                    newName[ch]=newName[ch].lower();
            }
            else
                if(newName[ch]==newName[ch].lower())
                {
                    if(newWord)
                        newName[ch]=newName[ch].upper();
                }
            newWord=false;
        }
    }

    newName.replace(" ", QString::null);
    return newName;
}

static const char * getItalicStr(CFontEngine::EItalic it)
{
    switch(it)
    {
        default:
        case CFontEngine::ITALIC_NONE:
            return NULL;
        case CFontEngine::ITALIC_ITALIC:
            return "Italic";
        case CFontEngine::ITALIC_OBLIQUE:
            return "Oblique";
    }
}

//
// Create a full Ps name
static QString createName(const QString &family, const QString &weight, const char *italic)
{
    QString      name;
    QTextOStream str(&name);

    str << family;
    if(!weight.isNull() || NULL!=italic)
    {
        str << '-';
        if(!weight.isNull())
            str << weight;
        if(NULL!=italic)
            str << italic;
    }

    return name;
}

static void addEntry(QStringList &list, const QString &name, const QString &file)
{
    QString      entry;
    QTextOStream str(&entry);

    str << '/' << name << " (" << file << ") ;";
    if(-1==list.findIndex(entry))
        list.append(entry);
}

static void addAliasEntry(QStringList &list, const QString &x11Name, const QString &psName)
{
    if(x11Name!=psName)
    {
        QString      entry;
        QTextOStream str(&entry);

        str << '/' << x11Name << " /" << psName << " ;";
        if(-1==list.findIndex(entry))
            list.append(entry);
    }
}

//
// Create local may in fact be creating top level Fontmap - if dir==fontmap dir!
void CFontmap::createLocal(const QString &dir)
{
    CFile       old(dir);
    QDir        d(dir);
    QStringList entries;

    if(d.isReadable())
    {
        const QFileInfoList *files=d.entryInfoList();
 
        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;
 
            for(; NULL!=(fInfo=it.current()); ++it)
                if("."!=fInfo->fileName() && ".."!=fInfo->fileName() && !fInfo->isDir() && CFontEngine::hasAfmInfo(QFile::encodeName(fInfo->fileName())))
                {
                    const QStringList *existing=old.getEntries(fInfo->fileName());

                    if(existing && existing->count())
                        entries+=(*existing);
                    else
                    {
                        int face=0,
                            numFaces=0;

                        do
                        {
                            if(CGlobal::fe().openFont(fInfo->filePath(), CFontEngine::NAME|CFontEngine::PROPERTIES, false, face))
                            {
                                numFaces=CGlobal::fe().getNumFaces();  // Only really for TTC files...

                                //
                                // Add real 
                                addEntry(entries, CGlobal::fe().getPsName(), fInfo->fileName());

                                //
                                // Add fake entries for X11 generated names
                                switch(CGlobal::fe().getWeight())
                                {
                                    case CFontEngine::WEIGHT_MEDIUM:
                                    case CFontEngine::WEIGHT_REGULAR:
                                    {
                                        QString x11Ps(createX11PsName(CGlobal::fe().getFamilyName()));

                                        if(CFontEngine::ITALIC_ITALIC!=CGlobal::fe().getItalic() &&
                                           CFontEngine::ITALIC_OBLIQUE!=CGlobal::fe().getItalic())
                                            addAliasEntry(entries,
                                                          createName(x11Ps, "Roman", getItalicStr(CGlobal::fe().getItalic())),
                                                          CGlobal::fe().getPsName());
                                        addAliasEntry(entries,
                                                      createName(x11Ps, NULL, getItalicStr(CGlobal::fe().getItalic())),
                                                      CGlobal::fe().getPsName());
                                        break;
                                    }
                                    case CFontEngine::WEIGHT_UNKNOWN:
                                        break;
                                    default:
                                        addAliasEntry(entries,
                                                      createName(createX11PsName(CGlobal::fe().getFamilyName()),
                                                                 CFontEngine::weightStr(CGlobal::fe().getWeight()),
                                                                 getItalicStr(CGlobal::fe().getItalic())),
                                                      CGlobal::fe().getPsName());
                                }
                                CGlobal::fe().closeFont();
                            }
                        }
                        while(++face<numFaces);
                    }
                }
        }
    }

    unlink(QFile::encodeName(dir+"Fontmap"));

    ofstream out(QFile::encodeName(dir+"Fontmap"));

    if(out)
    {
        QStringList::Iterator it;

        for(it=entries.begin(); it!=entries.end(); ++it)
            out << (*it).latin1() << endl;
    }
}

void CFontmap::createTopLevel()
{
    //              
    // Cat each sub-folders top-level fontmap file to top-level one...

    QStringList xDirs;

    CGlobal::userXcfg().getDirs(xDirs);

    //
    // If we're not root, and the 1 and only fonts dir is the GS dir - then just create fontmap if it
    // does not exist.
    if(!CMisc::root() && 1==xDirs.count() && xDirs.first()==CGlobal::cfg().getFontmapDir())
    {
        if(!CMisc::fExists(CGlobal::cfg().getFontmapDir()+"Fontmap"))
            createLocal(CGlobal::cfg().getFontmapDir());
    }
    else
    {
        //
        // Need to combine local fontmaps into 1 top-level one
        QStringList           entries;
        QStringList::Iterator it;

        for(it=xDirs.begin(); it!=xDirs.end(); ++it)
        {
            if(!CMisc::fExists((*it)+"Fontmap"))
                createLocal(*it);

            ifstream f(QFile::encodeName((*it)+"Fontmap"));

            if(f)
            {
                static const int constMaxLine=512;

                char    line[constMaxLine+1];
                QString lastPsName;


                while(!f.eof())
                {
                    QString ps,
                            fname;
                    bool    isAlias;

                    f.getline(line, constMaxLine);

                    if(!f.eof() && parseLine(line, ps, fname, isAlias) && !fname.contains('/'))
                        if(isAlias)
                        {
                            if(!lastPsName.isNull() && fname==lastPsName)
                                addAliasEntry(entries, ps, fname);  // fname => real Ps name
                        }
                        else
                        {
                            //
                            // Check if file path contains top-level fontmap dir - if so remove
                            QString ffile((*it)+fname);

                            if(0==ffile.find(CGlobal::cfg().getFontmapDir()))
                                addEntry(entries, ps, ffile.mid(CGlobal::cfg().getFontmapDir().length()));
                            else
                                addEntry(entries, ps, ffile);
                            lastPsName=ps;
                        }
                }
                f.close();
            }
        }

        ofstream of(QFile::encodeName(CGlobal::cfg().getFontmapDir()+"Fontmap"));

        if(of)
            for(it=entries.begin(); it!=entries.end(); ++it)
                of << (*it).latin1() << endl;
        of.close();

        CMisc::setTimeStamps(CGlobal::cfg().getFontmapDir());

        if(CMisc::root() && !CGlobal::cfg().getGhostscriptFile().isNull())  // Now ensure GS's Fontmap file .runlibfile's our Fontmap file!
        {
            const int constMaxLineLen=1024;
            const char *constRLF=".runlibfile";

            char     line[constMaxLineLen];
            ifstream in(QFile::encodeName(CGlobal::cfg().getGhostscriptFile()));

            if(in)
            {
                QCString fmap(QFile::encodeName(CGlobal::cfg().getFontmapDir()+"Fontmap"));
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
                        else if(strstr(line, "Fontmap.GS")!=NULL && strstr(line, constRLF)!=NULL)
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
                            ofstream out(QFile::encodeName(CGlobal::cfg().getGhostscriptFile()));

                            if(out)
                                out << buffer;
                        }
                        delete [] buffer;
                    }
                }
            }
        }
    }
}

CFontmap::CFile::CFile(const QString &dir)
{
    ifstream f(QFile::encodeName(dir+"Fontmap"));

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
                    QString d(CMisc::getDir(fname));

                    if(d==dir)
                        fname=CMisc::getFile(fname);

                    TEntry *entry=getEntry(&current, fname, isAlias);

                    if(!isAlias && entry->psName.isNull())
                        entry->psName=ps;

                    if(entry)
                        entry->entries.append(line);
                }
            }
        }
        f.close();
    }
}

const QStringList * CFontmap::CFile::getEntries(const QString &fname)
{
    TEntry *entry=findEntry(fname, false);

    return entry ? &entry->entries : NULL;
}

CFontmap::CFile::TEntry * CFontmap::CFile::findEntry(const QString &fname, bool isAlias)
{
    TEntry *entry=NULL;

    for(entry=itsEntries.first(); entry; entry=itsEntries.next())
        if(isAlias ? entry->psName==fname : entry->filename==fname)
            break;

    return entry;
}

CFontmap::CFile::TEntry * CFontmap::CFile::getEntry(TEntry **current, const QString &fname, bool isAlias)
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
