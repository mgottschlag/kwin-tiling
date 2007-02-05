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

#include "DisabledFonts.h"
#include "Fc.h"
#include "Misc.h"
#include "KfiConstants.h"
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QTextStream>
#include <klockfile.h>
#include <ksavefile.h>
#include <klocale.h>
#include <fontconfig/fontconfig.h>
#include <stdio.h>

namespace KFI
{

#define FILE_NAME     "disabledfonts"
#define DISABLED_DOC  "disabledfonts"
#define FONT_TAG      "font"
#define FILE_TAG      "file"
#define PATH_ATTR     "path"
#define FAMILY_ATTR   "family"
#define WEIGHT_ATTR   "weight"
#define WIDTH_ATTR    "width"
#define SLANT_ATTR    "slant"
#define FACE_ATTR     "face"

static const int     constStaleLockTime(5);
static const QString constLockExt(".lock");

static QString changeName(const QString &f, bool enable)
{
    QString file(Misc::getFile(f)),
            dest;

    if(enable && Misc::isHidden(file))
        dest=Misc::getDir(f)+file.mid(1);
    else if (!enable && !Misc::isHidden(file))
        dest=Misc::getDir(f)+QChar('.')+file;
    else
        dest=f;
    return dest;
}

static bool changeFileStatus(const QString &f, bool enable)
{
    QString dest(changeName(f, enable));

    if(dest==f)  // File is already enabled/disabled
        return true;

    if(Misc::fExists(dest) && !Misc::fExists(f)) // File is already enabled/disabled
        return true;

    if(0==::rename(QFile::encodeName(f).data(), QFile::encodeName(dest).data()))
    {
        QStringList files;

        Misc::getAssociatedFiles(f, files);

        if(files.count())
        {
            QStringList::Iterator fIt,
                                  fEnd=files.end();

            for(fIt=files.begin(); fIt!=fEnd; ++fIt)
                ::rename(QFile::encodeName(*fIt).data(),
                         QFile::encodeName(changeName(*fIt, enable)).data());
        }
        return true;
    }
    return false;
}

static bool changeStatus(const CDisabledFonts::TFileList &files, bool enable)
{
    QStringList                         mods;
    CDisabledFonts::TFileList::ConstIterator it(files.begin()),
                                        end(files.end());

    for(; it!=end; ++it)
        if(changeFileStatus((*it).path, enable))
            mods.append((*it).path);
        else
            break;

    if(mods.count()!=files.count())
    {
        //
        // Failed to enable/disable a file - so need to revert any
        // previous changes...
        QStringList::ConstIterator sit(mods.begin()),
                                   send(mods.end());
        for(; sit!=send; ++sit)
            changeFileStatus(*sit, !enable);
        return false;
    }
    return true;
}

CDisabledFonts::CDisabledFonts(const QString &path, bool sys)
              : itsTimeStamp(0),
                itsModified(false)
{
    QString p;

    if(path.isEmpty())
    {
        if(Misc::root() || sys)
            p=KFI_ROOT_CFG_DIR;
        else
        {
            FcStrList   *list=FcConfigGetFontDirs(FcInitLoadConfig());
            FcChar8     *dir;
            QString     home(QDir::homePath()),
                        defaultDir(home+"/.fonts");

            while((dir=FcStrListNext(list)))
            {
                QString d((const char *)dir);

                if(0==d.indexOf(home))
                    if(d==defaultDir)
                    {
                        p=defaultDir;
                        break;
                    }
                    else if(p.isEmpty())
                        p=d;
            }

            if(p.isEmpty())
                p=defaultDir;
        }
    }
    else
        p=path;

    itsFileName=p+"/"FILE_NAME".xml";

    itsModifiable=Misc::fWritable(itsFileName) ||
                  (!Misc::fExists(itsFileName) && Misc::dWritable(Misc::getDir(itsFileName)));
    load();
}

bool CDisabledFonts::refresh()
{
    bool update=Misc::getTimeStamp(itsFileName)!=itsTimeStamp;
    save();
    load();
    return update;
}

//
// Dont always lock during a load, as we may be trying to read global file (but not as root),
// or this load might be being called within the save() - so cant lock as is already!
void CDisabledFonts::load(bool lock)
{
    KLockFile lf(itsFileName+constLockExt);

    lf.setStaleTime(constStaleLockTime);
    lock=lock && itsModifiable;

    if(!lock || KLockFile::LockOK==lf.lock(KLockFile::ForceFlag))
    {
        time_t ts=Misc::getTimeStamp(itsFileName);

        if(!ts || ts!=itsTimeStamp)
        {
            itsTimeStamp=ts;

            QFile f(itsFileName);

            if(f.open(QIODevice::ReadOnly))
            {
                itsModified=false;

                QDomDocument doc;

                if(doc.setContent(&f))
                    for(QDomNode n=doc.documentElement().firstChild(); !n.isNull(); n=n.nextSibling())
                    {
                        QDomElement e=n.toElement();

                        if(FONT_TAG==e.tagName())
                        {
                            TFont font;

                            if(font.load(e))
                                itsDisabledFonts.add(font);
                        }
                    }

                f.close();
            }
        }
    }
}

bool CDisabledFonts::save()
{
    bool rv(true);

    if(itsModified)
    {
        KLockFile lf(itsFileName+constLockExt);

        lf.setStaleTime(constStaleLockTime);
        if(KLockFile::LockOK==lf.lock(KLockFile::ForceFlag))
        {
            time_t ts=Misc::getTimeStamp(itsFileName);

            if(Misc::fExists(itsFileName) && ts!=itsTimeStamp)
            {
                // Timestamps differ, so possibly file was modified by another process...
                merge(CDisabledFonts(*this));
            }

            KSaveFile file(itsFileName);

            if(file.open())
            {
                QTextStream str(&file);

                str << "<"DISABLED_DOC">" << endl;

                TFontList::Iterator it(itsDisabledFonts.begin()),
                                    end(itsDisabledFonts.end());

                for(; it!=end; ++it)
                    str << (*it);
                str << "</"DISABLED_DOC">" << endl;
                itsModified=false;
                file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|
                                    QFile::ReadGroup|QFile::ReadOther);
                rv=file.finalize();
                if(rv)
                    itsTimeStamp=Misc::getTimeStamp(itsFileName);
            }
        }
    }

    return rv;
}

static QString expandHome(QString path)
{
    return !path.isEmpty() && '~'==path[0]
        ? 1==path.length() ? QDir::homePath() : path.replace(0, 1, QDir::homePath())
        : path;
}

bool CDisabledFonts::TFile::load(QDomElement &elem)
{
    if(elem.hasAttribute(PATH_ATTR))
    {
        bool ok(false);

        path=expandHome(elem.attribute(PATH_ATTR));

        if(elem.hasAttribute(FACE_ATTR))
            face=elem.attribute(FACE_ATTR).toInt(&ok);
        if(!ok || face<0)
            face=0;
        return Misc::fExists(path);
    }

    return false;
}

bool CDisabledFonts::TFont::load(QDomElement &elem)
{
    if(elem.hasAttribute(FAMILY_ATTR))
    {
        bool ok(false);
        int  weight(KFI_NULL_SETTING), width(KFI_NULL_SETTING), slant(KFI_NULL_SETTING),
             tmp(KFI_NULL_SETTING);

        family=elem.attribute(FAMILY_ATTR);

        if(elem.hasAttribute(WEIGHT_ATTR))
        {
            tmp=elem.attribute(WEIGHT_ATTR).toInt(&ok);
            if(ok)
                weight=tmp;
        }
        if(elem.hasAttribute(WIDTH_ATTR))
        {
            tmp=elem.attribute(WIDTH_ATTR).toInt(&ok);
            if(ok)
                width=tmp;
        }

        if(elem.hasAttribute(SLANT_ATTR))
        {
            tmp=elem.attribute(SLANT_ATTR).toInt(&ok);
            if(ok)
                slant=tmp;
        }

        styleInfo=FC::createStyleVal(weight, width, slant);

        if(elem.hasAttribute(PATH_ATTR))
        {
            TFile file;

            if(file.load(elem))
                files.add(file);
        }
        else
            for(QDomNode n=elem.firstChild(); !n.isNull(); n=n.nextSibling())
            {
                QDomElement ent=n.toElement();

                if(FILE_TAG==ent.tagName())
                {
                    TFile file;

                    if(file.load(ent))
                        files.add(file);
                }
            }

        return files.count()>0;
    }

    return false;
}

const QString & CDisabledFonts::TFont::getName() const
{
    if(name.isEmpty())
        name=FC::createName(family, styleInfo);
    return name;
}

CDisabledFonts::TFontList::Iterator CDisabledFonts::TFontList::locate(const TFont &t)
{
    return find(t);
}

CDisabledFonts::TFontList::Iterator CDisabledFonts::TFontList::locate(const Misc::TFont &t)
{
    return locate((TFont &)t);
}

void CDisabledFonts::TFontList::add(const TFont &t) const
{
    (const_cast<TFontList *>(this))->insert(t);
}

bool CDisabledFonts::disable(const TFont &font)
{
    TFontList::Iterator it=itsDisabledFonts.locate(font);

    if(it==itsDisabledFonts.end())
    {
        TFont newFont(font.family, font.styleInfo);

        if(changeStatus(font.files, false))
        {
            TFileList::ConstIterator it(font.files.begin()),
                                     end(font.files.end());

            for(; it!=end; ++it)
                newFont.files.add(TFile(changeName((*it).path, false), (*it).face));

            itsDisabledFonts.add(newFont);
            itsModified=true;
            return true;
        }
        else
        {
            TFileList::ConstIterator it(font.files.begin()),
                                     end(font.files.end());

            for(; it!=end; ++it)
            {
                QString modName(changeName((*it).path, false));
                if(Misc::fExists(modName))
                    newFont.files.add(TFile(modName, (*it).face));
                else
                    break;
            }

            if(newFont.files.count()==font.files.count())
            {
                itsDisabledFonts.add(newFont);
                itsModified=true;
                return true;
            }
        }
    }
    else
        return true; // Already disabled...

    return false;
}

bool CDisabledFonts::enable(TFontList::Iterator font)
{
    if(font!=itsDisabledFonts.end())
    {
        if(changeStatus((*font).files, true))
        {
            itsDisabledFonts.erase(font);
            itsModified=true;
            return true;
        }
        else
        {
            QStringList              mod;
            TFileList::ConstIterator fit((*font).files.begin()),
                                     fend((*font).files.end());

            for(; fit!=fend; ++fit)
            {
                QString modName(changeName((*fit).path, true));
                if(Misc::fExists(modName))
                    mod.append((*fit).path);
                else
                    break;
            }

            if(mod.count()==(*font).files.count())
            {
                itsDisabledFonts.erase(font);
                itsModified=true;
                return true;
            }
        }
    }
    else
        return true; // Already enabled...

    return false;
}

CDisabledFonts::TFontList::Iterator CDisabledFonts::find(const QString &name, int face)
{
    TFontList::Iterator it(itsDisabledFonts.begin()),
                        end(itsDisabledFonts.end());
    QString             fontName(name);

    if('.'==fontName[0])
        fontName=fontName.mid(1);

    for(; it!=end; ++it)
        if((*it).getName()==fontName)
            break;

    if(it==end && '.'==name[0])
        for(it=itsDisabledFonts.begin(); it!=end ; ++it)
        {
            TFileList::ConstIterator fit((*it).files.begin()),
                                     fend((*it).files.end());

            for(; fit!=fend; ++fit)
                if(Misc::getFile((*fit).path)==name && (*fit).face==face)
                    return it;
        }
    return it;
}

//
// This constrcutor is only used internally, and is called in ::save() when it has been
// detected that the file has been modified by another process...
CDisabledFonts::CDisabledFonts(const CDisabledFonts &o)
              : itsFileName(o.itsFileName),
                itsTimeStamp(o.itsTimeStamp),
                itsModified(false),
                itsModifiable(false)
{
    load(false);
}

void CDisabledFonts::merge(const CDisabledFonts &other)
{
    TFontList::ConstIterator it(other.itsDisabledFonts.begin()),
                             end(other.itsDisabledFonts.end());

    for(; it!=end; ++it)
    {
        TFontList::Iterator existing(itsDisabledFonts.locate(*it));

        if(existing!=itsDisabledFonts.end())
        {
            TFileList::ConstIterator fit((*it).files.begin()),
                                     fend((*it).files.end());

            for(; fit!=fend; ++fit)
                if(!(*existing).files.contains(*fit))
                {
                    (*existing).files.add(*fit);
                    itsModified=true;
                }
        }
        else
        {
            itsDisabledFonts.add(*it);
            itsModified=true;
        }
    }
}

}

static QString contractHome(QString path)
{
    if (!path.isEmpty() && '/'==path[0])
    {
        QString home(QDir::homePath());

        if(path.startsWith(home))
        {
            int len = home.length();

            if(len>=0 && (path.length() == len || path[len] == '/'))
                return path.replace(0, len, QString::fromLatin1("~"));
        }
    }

    return path;
}

QTextStream & operator<<(QTextStream &s, const KFI::CDisabledFonts::TFile &f)
{
    s << PATH_ATTR"=\"" << contractHome(f.path) << "\" ";

    if(f.face>0)
        s << FACE_ATTR"=\"" << f.face << "\" ";

    return s;
}

QTextStream & operator<<(QTextStream &s, const KFI::CDisabledFonts::TFont &f)
{
    int                                                  weight, width, slant;
    QList<KFI::CDisabledFonts::TFileList::ConstIterator> saveFiles;
    KFI::CDisabledFonts::TFileList::ConstIterator        it(f.files.begin()),
                                                         end(f.files.end());

    KFI::FC::decomposeStyleVal(f.styleInfo, weight, width, slant);

    for(; it!=end; ++it)
        if(KFI::Misc::fExists((*it).path))
            saveFiles.append(it);

    if(saveFiles.count())
    {
        QList<KFI::CDisabledFonts::TFileList::ConstIterator>::ConstIterator
                fIt(saveFiles.begin()),
                fEnd(saveFiles.end());

        s << " <"FONT_TAG" "FAMILY_ATTR"=\"" << f.family << "\" ";

        if(KFI_NULL_SETTING!=weight)
            s << WEIGHT_ATTR"=\"" << weight << "\" ";
        if(KFI_NULL_SETTING!=width)
            s << WIDTH_ATTR"=\"" << width << "\" ";
        if(KFI_NULL_SETTING!=slant)
            s << SLANT_ATTR"=\"" << slant << "\" ";

        if(1==saveFiles.count())
            s << (*(*fIt)) << "/>" << endl;
        else
        {
            s << '>' << endl;
            for(; fIt!=fEnd; ++fIt)
                s << "  <"FILE_TAG" " << (*(*fIt)) << "/>" << endl;
            s << " </"FONT_TAG">" << endl;
        }
    }

    return s;
}
