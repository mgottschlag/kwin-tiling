////////////////////////////////////////////////////////////////////////////////
//
// Namespace     : KFI::Misc
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
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "Misc.h"
#include <qfile.h>
#include <kprocess.h> 
#include <kstandarddirs.h>
#include <klargefile.h>
#include <kio/netaccess.h>
#include <unistd.h>

namespace KFI
{

namespace Misc
{

QString linkedTo(const QString &i)
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

QString dirSyntax(const QString &d)
{
    if(!d.isEmpty())
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

QString xDirSyntax(const QString &d)
{
    if(!d.isEmpty())
    {
        QString ds(d);

        ds.replace("//", "/");

        int slashPos=ds.findRev('/');
 
        if(slashPos==(((int)ds.length())-1))
            ds.remove(slashPos, 1);
        return ds;
    }

    return d;
}

QString getDir(const QString &f)
{
    QString d(f);

    int slashPos=d.findRev('/');
 
    if(slashPos!=-1)
        d.remove(slashPos+1, d.length());

    return dirSyntax(d);
}

QString getFile(const QString &f)
{
    QString d(f);

    int slashPos=d.findRev('/');
 
    if(slashPos!=-1)
        d.remove(0, slashPos+1);

    return d;
}

bool createDir(const QString &dir)
{
    //
    // Clear any umask before dir is created
    mode_t oldMask=umask(0000);
    bool   status=KStandardDirs::makeDir(dir, DIR_PERMS);
    // Reset umask
    ::umask(oldMask);
    return status;
}

bool doCmd(const QString &cmd, const QString &p1, const QString &p2, const QString &p3)
{
    KProcess proc;

    proc << cmd;

    if(!p1.isEmpty())
        proc << p1;
    if(!p2.isEmpty())
        proc << p2;
    if(!p3.isEmpty())
        proc << p3;

    proc.start(KProcess::Block);

    return proc.normalExit() && proc.exitStatus()==0;
}

QString changeExt(const QString &f, const QString &newExt)
{
    QString newStr(f);
    int     dotPos=newStr.findRev('.');

    if(-1==dotPos)
        newStr+=QChar('.')+newExt;
    else
    {
        newStr.remove(dotPos+1, newStr.length());
        newStr+=newExt;
    }
    return newStr;
}

void createBackup(const QString &f)
{
    const QString constExt(".bak");

    if(!fExists(f+constExt) && fExists(f))
        doCmd("cp", "-f", f, f+constExt);
}

//
// Get a list of files associated with a file, e.g.:
//
//    File: /home/a/courier.pfa
//
//    Associated: /home/a/courier.afm /home/a/courier.pfm
//
void getAssociatedUrls(const KURL &url, KURL::List &list, bool afmAndPfm, QWidget *widget)
{
    const char *afm[]={"afm", "AFM", "Afm", "AFm", "AfM", "aFM", "aFm", "afM", NULL},
               *pfm[]={"pfm", "PFM", "Pfm", "PFm", "PfM", "pFM", "pFm", "pfM", NULL};
    bool       gotAfm=false,
               localFile=url.isLocalFile();
    int        e;

    for(e=0; afm[e]; ++e)
    {
        KURL statUrl(url);
        KIO::UDSEntry uds;

        statUrl.setPath(changeExt(url.path(), afm[e]));

        if(localFile ? fExists(statUrl.path()) : KIO::NetAccess::stat(statUrl, uds, widget))
        {
            list.append(statUrl);
            gotAfm=true;
            break;
        }
    }

    if(afmAndPfm || !gotAfm)
        for(e=0; pfm[e]; ++e)
        {
            KURL          statUrl(url);
            KIO::UDSEntry uds;

            statUrl.setPath(changeExt(url.path(), pfm[e]));
            if(localFile ? fExists(statUrl.path()) : KIO::NetAccess::stat(statUrl, uds, widget))
            {
                list.append(statUrl);
                break;
            }
        }
}

time_t getTimeStamp(const QString &item)
{
    KDE_struct_stat info;

    return !item.isEmpty() && 0==KDE_lstat(QFile::encodeName(item), &info) ? info.st_mtime : 0;
}


bool check(const QString &path, unsigned int fmt, bool checkW)
{ 
    KDE_struct_stat info;
    QCString        pathC(QFile::encodeName(path));

    return 0==KDE_lstat(pathC, &info) && (info.st_mode&S_IFMT)==fmt && (!checkW || 0==::access(pathC, W_OK));
}

}

}
