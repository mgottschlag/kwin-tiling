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
#include <QTextStream>
#include <kprocess.h> 
#include <kstandarddirs.h>
#include <kde_file.h>
#include <ksavefile.h>
#include <unistd.h>
#include <ctype.h>

namespace KFI
{

namespace Misc
{

QString prettyUrl(const KUrl &url)
{
    QString u(url.prettyUrl());

    u.replace("%20", " ");
    return u;
}

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
void getAssociatedFiles(const QString &path, QStringList &files, bool afmAndPfm)
{
    QString ext(path);
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
        bool       gotAfm(false);
        int        e;

        for(e=0; afm[e]; ++e)
        {
            QString statFile(changeExt(path, afm[e]));

            if(fExists(statFile))
            {
                files.append(statFile);
                gotAfm=true;
                break;
            }
        }

        if(afmAndPfm || !gotAfm)
            for(e=0; pfm[e]; ++e)
            {
                QString statFile(changeExt(path, pfm[e]));

                if(fExists(statFile))
                {
                    files.append(statFile);
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

//
// mkfontscale doesnt ingore hidden files :-(
static void removeHiddenEntries(const QString &file)
{
    QStringList lines;
    QFile       f(file);

    if(f.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&f);
        QString     line;

        int lineCount=stream.readLine().toInt(); // Ignore line count...

        while(!stream.atEnd())
        {
            line=stream.readLine();
            if(line.length() && '.'!=line[0])
                lines.append(line);
        }
        f.close();

        if(lineCount!=lines.count())
        {
            KSaveFile out(file);

            if(out.open())
            {
                QTextStream                stream(&out);
                QStringList::ConstIterator it(lines.begin()),
                                           end(lines.end());

                stream << lines.count() << endl;
                for(; it!=end; ++it)
                    stream << (*it).toLocal8Bit() << endl;
                out.finalize();
            }
        }
    }
}

bool configureForX11(const QString &dir)
{
    //
    // On systems without mkfontscale, the following will fail, so cant base
    // return value upon that - hence only check return value of mkfontdir
    doCmd("mkfontscale", QFile::encodeName(dir));
    removeHiddenEntries(dir+"fonts.scale");
    bool rv=doCmd("mkfontdir", QFile::encodeName(dir));
    removeHiddenEntries(dir+"fonts.dir");
    return rv;
}

} // Misc::

} // KFI::

