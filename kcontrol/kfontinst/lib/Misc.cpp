////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CMisc
// Author        : Craig Drummond
// Project       : K Font Installer
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003
////////////////////////////////////////////////////////////////////////////////

#include "Misc.h"
#include <kprocess.h>
#include <klargefile.h>
#include <qdir.h>
#include <qfile.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>

QString CMisc::linkedTo(const QString &i)
{
    QString d;

    if(isLink(i))
    {
        char buffer[1000];
        int  n=readlink(QFile::encodeName(i), buffer, 1000);

        if(n!=-1)
        {
            buffer[n]='\0';
            d=buffer;
        }
    }

    return d;
}

QString CMisc::dirSyntax(const QString &d)
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

QString CMisc::xDirSyntax(const QString &d)
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

QString CMisc::getDir(const QString &f)
{
    QString d(f);

    int slashPos=d.findRev('/');
 
    if(slashPos!=-1)
        d.remove(slashPos+1, d.length());

    return dirSyntax(d);
}

QString CMisc::getFile(const QString &f)
{
    QString d(f);

    int slashPos=d.findRev('/');
 
    if(slashPos!=-1)
        d.remove(0, slashPos+1);

    return d;
}

bool CMisc::doCmd(const QString &cmd, const QString &p1, const QString &p2, const QString &p3)
{
    KProcess proc;

    proc << cmd;

    if(!p1.isNull())
        proc << p1;
    if(!p2.isNull())
        proc << p2;
    if(!p3.isNull())
        proc << p3;

    proc.start(KProcess::Block);

    return proc.normalExit() && proc.exitStatus()==0;
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
 
    QString constOk(" _");

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

    if(modified[modified.length()-1].isNull())
        modified.truncate(modified.length()-1);
    return modified;
}

void CMisc::createBackup(const QString &f)
{
    const QString constExt(".bak");

    if(!fExists(f+constExt) && fExists(f) && dWritable(getDir(f)))
        doCmd("cp", "-f", f, f+constExt);
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

QString CMisc::getName(const QString &f)
{
    //
    // Ensure there is no trailing /, and no double //'s
    if(!f.isNull())
    {
        QString nf(f);

        nf.replace("//", "/");

        int slashPos=nf.findRev('/');

        if(slashPos==(((int)nf.length())-1))
            nf.remove(slashPos, 1);

        return -1==nf.find('/') ? nf : nf.section('/', -1);
    }

    return f.section('/', -1);
}

void CMisc::removeAssociatedFiles(const QString realPath, bool d)
{
    QDir dir(d ? realPath
               : getDir(realPath),
             d ? QString::null
               : getFile(changeExt(realPath, "*")));

    if(dir.isReadable())
    {
        const QFileInfoList *files=dir.entryInfoList();

        if(files)
        {
            QFileInfoListIterator    it(*files);
            QFileInfo                *fInfo;
            for(; NULL!=(fInfo=it.current()); ++it)
                if(!fInfo->isDir())
                    unlink(QFile::encodeName(fInfo->filePath()));
        }
    }
}

bool CMisc::hidden(const QString &u, bool dir)
{
    QString str;

    if(dir)
    {
        QString str2(dirSyntax(u));

        int slash=str2.findRev('/');

        if(-1!=slash)
            slash=str2.findRev('/', slash-1);
        str= -1==slash ? str2 : str2.mid(slash+1);
    }
    else
        str=getFile(u);

    return QChar('.')==str[0];
}

time_t CMisc::getTimeStamp(const QString &item)
{
    KDE_struct_stat info;

    return !item.isNull() && 0==KDE_lstat(QFile::encodeName(item), &info) ? info.st_mtime : 0;
}

void CMisc::setTimeStamps(const QString &ds)
{
    QCString        dirC(QFile::encodeName(ds));
    KDE_struct_stat dirStat;

    utime(dirC, NULL);   // Set dir time-stamp to now...

    if(0==KDE_lstat(dirC, &dirStat))  // Read timestamp back...
    {
        static const char *files[]=
                          {
                             "fonts.scale",
                              "fonts.dir",
                              "Fontmap",
                              ".fonts-config-timestamp",  // Only on SuSE...
                              NULL
                          };
        int               f;
        struct utimbuf    times;

        times.actime=dirStat.st_atime;
        times.modtime=dirStat.st_mtime;

        for(f=0; NULL!=files[f]; ++f)
            if(CMisc::fExists(ds+files[f]))
                utime(QFile::encodeName(ds+files[f]), &times);
    }
}

bool CMisc::check(const QString &path, unsigned int fmt, bool checkW)
{ 
    KDE_struct_stat info;
    QCString        pathC(QFile::encodeName(path));

    return 0==KDE_lstat(pathC, &info) && (info.st_mode&S_IFMT)==fmt && (!checkW || 0==::access(pathC, W_OK));
}
