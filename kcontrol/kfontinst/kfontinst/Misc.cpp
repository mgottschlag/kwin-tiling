////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CMisc
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 01/05/2001
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

#include "Misc.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "FontEngine.h"
#include <kprocess.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <ctype.h>

bool CMisc::dExists(const QString &d)
{
    QDir dir(d);

    return dir.isReadable();
}

bool CMisc::fExists(const QString &f)
{
    QFile file(f);

    return file.exists();
}

bool CMisc::fWritable(const QString &f)
{
    QFileInfo inf(f);
 
    return inf.isWritable();
}

bool CMisc::dWritable(const QString &d)
{
    QFileInfo inf(d);
 
    return inf.isWritable();
}

bool CMisc::dHasSubDirs(const QString &d)
{
    QDir dir(d);
 
    if(dir.isReadable())
    {
        const QFileInfoList *files=dir.entryInfoList();
 
        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;
 
            for(; NULL!=(fInfo=it.current()); ++it)
                if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                    if(fInfo->isDir())
                        return true;
        }
    }

    return false;
}

bool CMisc::dContainsTTorT1Fonts(const QString &d)
{
    QDir dir(d);
 
    if(dir.isReadable())
    {
        const QFileInfoList *files=dir.entryInfoList();
 
        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;
 
            for(; NULL!=(fInfo=it.current()); ++it)
                if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                    if(CFontEngine::isATtf(fInfo->fileName().local8Bit())||CFontEngine::isAType1(fInfo->fileName().local8Bit()))
                        return true;
        }
    }
 
    return false;
}

QString CMisc::getDir(const QString &f)
{
    QString d(f);

    int slashPos=d.findRev('/');
 
    if(slashPos!=-1)
        d.remove(slashPos+1, d.length());

    return d;
}

unsigned int CMisc::getNumItems(const QString &d)
{
    QDir dir(d);
 
    if(dir.isReadable())
        return dir.count()>=2 ? dir.count()-2 : dir.count();
 
    return 0;
}

unsigned int CMisc::countFonts(const QString &d)
{
    QDir         dir(d);
    unsigned int num=0;
 
    if(dir.isReadable())
    {
        const QFileInfoList *files=dir.entryInfoList();
 
        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;
 
            for(; NULL!=(fInfo=it.current()); ++it)
                if("."!=fInfo->fileName() && ".."!=fInfo->fileName())
                    if(!fInfo->isDir() && CFontEngine::isAFont(fInfo->fileName().local8Bit()))
                        num++;
        }
    }

    return num;
}

bool CMisc::doCmd(const QString &cmd, const QString &p1, const QString &p2, const QString &p3)
{
    KProcess proc;

    proc << cmd;

    if(QString::null!=p1)
        proc << p1;
    if(QString::null!=p2)
        proc << p2;
    if(QString::null!=p3)
        proc << p3;

    proc.start(KProcess::Block);

    return proc.normalExit() && proc.exitStatus()==0;
}

bool CMisc::doCmdStr(const QString &cmdStr)  // Execute commands contained in a string (e.g. "ls ; ls /tmp ")
{                                            // This function is only really used by the custom X refresh command
    KProcess *proc=new KProcess;
    bool     status=true,
             addedCmd=false;
    int      newPos,
             oldPos=0;

    while(status && (newPos=cmdStr.find(QRegExp("[ ;\\t]"), oldPos))!=-1)
    {
        if(newPos>oldPos)
        {
            *proc << cmdStr.mid(oldPos, newPos-oldPos);
            addedCmd=true;
        }

        if(cmdStr[newPos]==';' && addedCmd)
        {
            proc->start(KProcess::Block);
            status=proc->normalExit() && proc->exitStatus()==0;
            if(status)
            {
                delete proc;
                proc=new KProcess;
                addedCmd=false;
            }
        }

        oldPos=newPos+1;
    }

    if(status && oldPos<(int)cmdStr.length())
    {
        *proc << cmdStr.mid(oldPos, cmdStr.length()-oldPos);
        proc->start(KProcess::Block);
        status=proc->normalExit() && proc->exitStatus()==0;
    }

    delete proc;

    return status;
}

QString CMisc::changeExt(const QString &f, const QString &newExt)
{
    QString newStr(f);
    int dotPos=newStr.findRev('.');

    if(dotPos!=-1)
    {
        newStr.remove(dotPos+1, newStr.length());
        newStr+=newExt;
    }
    return newStr;
}

QString CMisc::removeSymbols(const QString &str)
{
    //
    // Remove any non-alphanumeric (except ' ' & '_') chars from str
    //
 
    static const QString constOk(" _");

    QString      modified; 
    unsigned int i,
                 offset=0,
                 slen=str.length();
 
    for(i=0; i<str.length()+1; ++i)
        if(str[i].isLetterOrNumber() || constOk.contains(str[i]) || str[i].isNull())
            modified[i-offset]=str[i];
        else
            if(i<slen && str[i+1].isSpace())  // If next char is a space, remove this char
                offset++;
            else
                modified[i-offset]=QChar(' ');  // Else replace with a space
 
    return modified;
}

QString CMisc::shortName(const QString &dir)
{
    QString sn(dir);
 
    if(sn.find(CKfiGlobal::cfg().getFontsDir())==0)
        sn.remove(0, CKfiGlobal::cfg().getFontsDir().length());
 
    return sn;
}

int CMisc::findIndex(const QComboBox *box, const QString &str)
{
    int pos=-1;
 
    if(box!=NULL)
    {
        int index;
 
        for(index=0; index<box->count(); ++index)
            if(box->text(index)==str)
            {
                pos=index;
                break;
            }
    }
 
    return pos;
}

int CMisc::stricmp(const char *s1, const char *s2)
{
    char c1,
         c2;
 
    for(;;)
    {
        c1=*s1++;
        c2=*s2++;
        if(!c1 || !c2)
            break;
        if(isupper(c1))
            c1=tolower (c1);
        if(isupper(c2))
            c2=tolower (c2);
        if(c1!=c2)
            break;
    }
    return (int)c2-(int)c1;
}
