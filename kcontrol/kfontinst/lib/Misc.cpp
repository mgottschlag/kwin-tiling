/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Misc.h"
#include <QFile>
#include <QByteArray>
#include <kprocess.h> 
#include <kstandarddirs.h>
#include <kde_file.h>
#include <kio/netaccess.h>
#include <unistd.h>
#include <ctype.h>

namespace KFI
{

namespace Misc
{

QString dirSyntax(const QString &d)
{
    if(!d.isEmpty())
    {
        QString ds(d);

        ds.replace("//", "/");

        int slashPos(ds.lastIndexOf('/'));

        if(slashPos!=(((int)ds.length())-1))
            ds.append('/');

        return ds;
    }

    return d;
}

QString fileSyntax(const QString &d)
{
    if(!d.isEmpty())
    {
        QString ds(d);

        ds.replace("//", "/");

        int slashPos(ds.lastIndexOf('/'));

        if(slashPos==(((int)ds.length())-1))
            ds.remove(slashPos, 1);
        return ds;
    }

    return d;
}

QString getDir(const QString &f)
{
    QString d(f);

    int slashPos(d.lastIndexOf('/'));
 
    if(slashPos!=-1)
        d.remove(slashPos+1, d.length());

    return dirSyntax(d);
}

QString getFile(const QString &f)
{
    QString d(f);

    int slashPos=d.lastIndexOf('/');
 
    if(slashPos!=-1)
        d.remove(0, slashPos+1);

    return d;
}

bool createDir(const QString &dir)
{
    //
    // Clear any umask before dir is created
    mode_t oldMask(umask(0000));
    bool   status(KStandardDirs::makeDir(dir, DIR_PERMS));
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
    int     dotPos(newStr.lastIndexOf('.'));

    if(-1==dotPos)
        newStr+=QChar('.')+newExt;
    else
    {
        newStr.remove(dotPos+1, newStr.length());
        newStr+=newExt;
    }
    return newStr;
}

//
// Get a list of files associated with a file, e.g.:
//
//    File: /home/a/courier.pfa
//
//    Associated: /home/a/courier.afm /home/a/courier.pfm
//
void getAssociatedUrls(const KUrl &url, KUrl::List &list, bool afmAndPfm, QWidget *widget)
{
    QString ext(url.path());
    int     dotPos(ext.lastIndexOf('.'));
    bool    check(false);

    if(-1==dotPos) // Hmm, no extension - check anyway...
        check=true;
    else           // Cool, got an extension - see if its a Type1 font...
    {
        ext=ext.mid(dotPos+1);
        check=0==ext.compare("pfa", Qt::CaseInsensitive) ||
              0==ext.compare("pfb", Qt::CaseInsensitive);
    }

    if(check)
    {
        const char *afm[]={"afm", "AFM", "Afm", NULL},
                   *pfm[]={"pfm", "PFM", "Pfm", NULL};
        bool       gotAfm(false),
                   localFile(url.isLocalFile());
        int        e;

        for(e=0; afm[e]; ++e)
        {
            KUrl statUrl(url);
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
                KUrl          statUrl(url);
                KIO::UDSEntry uds;

                statUrl.setPath(changeExt(url.path(), pfm[e]));
                if(localFile ? fExists(statUrl.path()) : KIO::NetAccess::stat(statUrl, uds, widget))
                {
                    list.append(statUrl);
                    break;
                }
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
    QByteArray      pathC(QFile::encodeName(path));

    return 0==KDE_lstat(pathC, &info) && (info.st_mode&S_IFMT)==fmt && (!checkW || 0==::access(pathC, W_OK));
}

QString getFolder(const QString &defaultDir, const QString &root, QStringList &dirs)
{
    if(dirs.contains(defaultDir))
        return defaultDir;
    else
    {
        QStringList::Iterator it,
                              end=dirs.end();
        bool                  found=false;

        for(it=dirs.begin(); it!=end && !found; ++it)
            if(0==(*it).indexOf(root))
                return *it;
    }

    return QString();
}

bool checkExt(const QString &fname, const QString &ext)
{
    QString extension('.'+ext);

    return fname.length()>extension.length()
            ? 0==fname.mid(fname.length()-extension.length()).compare(extension, Qt::CaseInsensitive)
            : false;
}

bool isMetrics(const QString &str)
{
    return checkExt(str, "afm") || checkExt(str, "pfm");
}

int getIntQueryVal(const KUrl &url, const char *key, int defVal)
{
    QString item(url.queryItem(key));
    int     val(defVal);

    if(!item.isNull())
        val=item.toInt();

    return val;
}

bool printable(const QString &mime)
{
    return "application/x-font-type1"==mime || "application/x-font-ttf"==mime ||
           "application/x-font-otf"==mime || "application/x-font-ttc"==mime ||
           "application/x-font-ghostscript"==mime;
}

uint qHash(const KFI::Misc::TFont &key)
{
    //return qHash(QString(key.family+'%'+QString().setNum(key.styleInfo, 16)));
    const QChar *p = key.family.unicode();
    int         n = key.family.size();
    uint        h = 0,
                g;

    h = (h << 4) + key.styleInfo;
    if ((g = (h & 0xf0000000)) != 0)
        h ^= g >> 23;
    h &= ~g;

    while (n--)
    {
        h = (h << 4) + (*p++).unicode();
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
    }
    return h;
}

} // Misc::

} // KFI::

