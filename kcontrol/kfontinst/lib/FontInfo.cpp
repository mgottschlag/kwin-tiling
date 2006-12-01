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

#include "FontInfo.h"
#include "FcEngine.h"
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

namespace KFI
{

QString const CFontInfo::theirDocumentTag("fontinfo");
QString const CFontInfo::theirGroupTag("group");
QString const CFontInfo::theirFontTag("font");

#define FILE_TAG      "file"
#define NAME_ATTR     "name"
#define PATH_ATTR     "path"
#define FAMILY_ATTR   "family"
#define WEIGHT_ATTR   "weight"
#define WIDTH_ATTR    "width"
#define SLANT_ATTR    "slant"
#define FACE_ATTR     "face"

#define STALE_LOCK_TIME 5

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

    if(Misc::doCmd("mv", "-f", f, dest))
    {
        KUrl::List urls;

        Misc::getAssociatedUrls(KUrl(f), urls);

        if(urls.count())
        {
            KUrl::List::Iterator uIt,
                                 uEnd=urls.end();

            for(uIt=urls.begin(); uIt!=uEnd; ++uIt)
                Misc::doCmd("mv", "-f", (*uIt).path(), changeName((*uIt).path(), enable));
        }
        return true;
    }
    return false;
}

static bool changeStatus(const CFontInfo::TFileList &files, bool enable)
{
    QStringList                         mods;
    CFontInfo::TFileList::ConstIterator it(files.begin()),
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

static QString expandHome(QString path)
{
    return !path.isEmpty() && '~'==path[0]
        ? 1==path.length() ? QDir::homePath() : path.replace(0, 1, QDir::homePath())
        : path;
}

CFontInfo::CFontInfo(const QString &path, bool abs, bool sys, const QString &name)
         : itsTimeStamp(0),
           itsModified(false)
{
    if(abs)
        itsFileName=path;
    else
    {
        QString p;

        if(path.isEmpty())
        {
            if(Misc::root())
                p=KFI_ROOT_CFG_DIR;
            else
            {
                FcStrList   *list=FcConfigGetFontDirs(FcInitLoadConfigAndFonts());
                QStringList dirs;
                FcChar8     *dir;

                while((dir=FcStrListNext(list)))
                    dirs.append(Misc::dirSyntax((const char *)dir));

                QString home(Misc::dirSyntax(QDir::homePath())),
                        defaultDir(Misc::dirSyntax(QDir::homePath()+"/.fonts/"));

                p=Misc::getFolder(defaultDir, home, dirs);
            }
        }
        else
            p=path;

        itsFileName=p+(Misc::root()||sys ? "/" : "/.")+name+".xml";
    }
    itsModifiable=Misc::fWritable(itsFileName) ||
                  (!Misc::fExists(itsFileName) && Misc::dWritable(Misc::getDir(itsFileName)));
    load();
}

//
// This constrcutor is only used internally, and is called in ::save() when it has been
// detected that the file has been modified by another process...
CFontInfo::CFontInfo(const CFontInfo &o)
         : itsFileName(o.itsFileName),
           itsTimeStamp(o.itsTimeStamp),
           itsModified(false),
           itsModifiable(false)
{
    load(false);
}

CFontInfo::CFontInfo(const QString &file, const TGroup &group)
         : itsFileName(file),
           itsTimeStamp(0),
           itsModified(true),
           itsModifiable(true)
{
    itsGroups.add(group);
}

bool CFontInfo::refresh()
{
    bool update=Misc::getTimeStamp(itsFileName)!=itsTimeStamp;
    save();
    load();
    return update;
}

//
// Dont always lock during a load, as we may be trying to read global file (but not as root),
// or this load might be being called within the save() - so cant lock as is already!
void CFontInfo::load(bool lock)
{
    KLockFile lf(itsFileName+constLockExt);

    lf.setStaleTime(STALE_LOCK_TIME);
    lock=lock && itsModifiable;

    if(!lock || KLockFile::LockOK==lf.lock(KLockFile::ForceFlag))
    {
        time_t ts=Misc::getTimeStamp(itsFileName);

        if(!ts || ts!=itsTimeStamp)
        {
            itsTimeStamp=ts;

            QFile f(itsFileName);

            if(f.open(IO_ReadOnly))
            {
                itsModified=false;
                itsDisabledFonts.clear();
                itsGroups.clear();

                QDomDocument doc;

                if(doc.setContent(&f))
                    for(QDomNode n=doc.documentElement().firstChild(); !n.isNull(); n=n.nextSibling())
                    {
                        QDomElement e=n.toElement();

                        if(theirFontTag==e.tagName())
                        {
                            TFont font;

                            if(font.load(e))
                                itsDisabledFonts.add(font);
                        }
                        else if(theirGroupTag==e.tagName())
                        {
                            TGroup group;

                            if(group.load(e))
                                itsGroups.add(group);
                        }
                    }

                f.close();
            }
        }
        if(lock)
            lf.unlock();
    }
}

bool CFontInfo::save()
{
    bool rv=false;

    if(itsModified)
    {
        KLockFile lf(itsFileName+constLockExt);

        lf.setStaleTime(STALE_LOCK_TIME);
        if(KLockFile::LockOK==lf.lock(KLockFile::ForceFlag))
        {
            time_t ts=Misc::getTimeStamp(itsFileName);

            if(Misc::fExists(itsFileName) && ts!=itsTimeStamp)
            {
                // Timestamps differ, so possibly file was modified by another process...
                CFontInfo inf(*this);

                merge(inf);
            }

            QDomDocument              doc(theirDocumentTag);
            QDomElement               inf(doc.createElement(theirDocumentTag));
            TGroupList::ConstIterator git(itsGroups.begin()),
                                      gend(itsGroups.end());

            for(; git!=gend; ++git)
                (*git).save(doc, inf);

            if(itsDisabledFonts.count())
            {
                TFontList::ConstIterator fit(itsDisabledFonts.begin()),
                                         fend(itsDisabledFonts.end());

                for(; fit!=fend; ++fit)
                    (*fit).save(doc, inf);
            }

            doc.appendChild(inf);

            KSaveFile file(itsFileName);

            if(file.open())
            {
                QTextStream(&file) << "<?xml version=\"1.0\"?>\n" << doc.toString();
                itsModified=false;
                lf.unlock();
                rv=file.finalize();
            }
        }
    }

    if(rv)
        itsTimeStamp=Misc::getTimeStamp(itsFileName);
    return rv;
}


bool CFontInfo::TFile::load(QDomElement &elem)
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

bool CFontInfo::TFile::save(QDomDocument &doc, QDomElement &parent) const
{
    if(Misc::fExists(path))
    {
        QDomElement elem=doc.createElement(FILE_TAG);

        elem.setAttribute(PATH_ATTR, contractHome(path));
        if(face>0)
            elem.setAttribute(FACE_ATTR, face);

        parent.appendChild(elem);
        return true;
    }
    return false;
}

bool CFontInfo::TFont::load(QDomElement &elem, bool ignoreFiles)
{
    if(elem.hasAttribute(FAMILY_ATTR))
    {
        bool ok(false);
        int  weight(KFI_NULL_SETTING), width(KFI_NULL_SETTING), slant(KFI_NULL_SETTING), tmp(KFI_NULL_SETTING);

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

        styleInfo=CFcEngine::createStyleVal(weight, width, slant);

        if(!ignoreFiles)
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

        return ignoreFiles || files.count()>0;
    }

    return false;
}

bool CFontInfo::TFont::save(QDomDocument &doc, QDomElement &parent, bool ignoreFiles) const
{
    QDomElement elem=doc.createElement(theirFontTag);
    bool        use=false;
    int         weight, width, slant;

    CFcEngine::decomposeStyleVal(styleInfo, weight, width, slant);
    elem.setAttribute(FAMILY_ATTR, family);
    if(KFI_NULL_SETTING!=weight)
        elem.setAttribute(WEIGHT_ATTR, weight);
    if(KFI_NULL_SETTING!=width)
        elem.setAttribute(WIDTH_ATTR, width);
    if(KFI_NULL_SETTING!=slant)
        elem.setAttribute(SLANT_ATTR, slant);
    if(!ignoreFiles)
    {
        TFileList::ConstIterator it(files.begin()),
                                 end(files.end());

        for(; it!=end; ++it)
            if((*it).save(doc, elem))
                use=true;
    }

    if(use || ignoreFiles)
        parent.appendChild(elem);

    return use || ignoreFiles;
}

const QString & CFontInfo::TFont::getName() const
{
    if(name.isEmpty())
        name=CFcEngine::createName(family, styleInfo);
    return name;
}

CFontInfo::TFontList::Iterator CFontInfo::TFontList::locate(const TFont &t)
{
    return find(t);
}

CFontInfo::TFontList::Iterator CFontInfo::TFontList::locate(const Misc::TFont &t)
{
    return locate((TFont &)t);
}

void CFontInfo::TFontList::add(const TFont &t) const
{
    (const_cast<TFontList *>(this))->insert(t);
}

bool CFontInfo::TGroup::load(QDomElement &elem)
{
    if(elem.hasAttribute(NAME_ATTR))
    {
        name=elem.attribute(NAME_ATTR);

        for(QDomNode n=elem.firstChild(); !n.isNull(); n=n.nextSibling())
        {
            QDomElement ent=n.toElement();

            if(theirFontTag==ent.tagName())
            {
                TFont font;

                if(font.load(ent, true))
                    fonts.add(font);
            }
        }

        return true; // Can have empty groups!
    }
    return false;
}

bool CFontInfo::TGroup::save(QDomDocument &doc, QDomElement &parent) const
{
    TFontList::ConstIterator it(fonts.begin()),
                             end(fonts.end());
    QDomElement              elem=doc.createElement(theirGroupTag);

    elem.setAttribute(NAME_ATTR, name);

    for(; it!=end; ++it)
        (*it).save(doc, elem, true);

    parent.appendChild(elem);
    return true; // Can have empty groups!
}

bool CFontInfo::merge(const TFontList &from, TFontList &to, bool ignoreFiles)
{
    TFontList::ConstIterator it(from.begin()),
                             end(from.end());
    bool                     modified(false);

    for(; it!=end; ++it)
    {
        TFontList::Iterator existing(to.locate(*it));

        if(existing!=to.end())
        {
            if(!ignoreFiles)
            {
                TFileList::ConstIterator fit((*it).files.begin()),
                                         fend((*it).files.end());

                for(; fit!=fend; ++fit)
                    if(!(*existing).files.contains(*fit))
                    {
                        (*existing).files.add(*fit);
                        modified=true;
                    }
            }
        }
        else
        {
            to.add(*it);
            modified=true;
        }
    }

    return modified;
}

bool CFontInfo::merge(const TGroupList &from, TGroupList &to)
{
    TGroupList::ConstIterator it(from.begin()),
                              end(from.end());
    bool                      modified(false);

    for(; it!=end; ++it)
    {
        TGroupList::Iterator existing(to.locate(*it));

        if(existing!=to.end())
        {
            if(merge((*it).fonts, (*existing).fonts, true))
                modified=true;
        }
        else
        {
            to.add(*it);
            modified=true;
        }
    }

    return modified;
}

bool CFontInfo::merge(const CFontInfo &other)
{
    bool modified(false);

    if(merge(other.itsDisabledFonts, itsDisabledFonts, false))
        modified=true;

    if(merge(other.itsGroups, itsGroups))
        modified=true;

    if(modified)
        itsModified=true;

    return modified;
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

CFontInfo::TFontList::Iterator CDisabledFonts::find(const QString &name, int face)
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

CFontGroups::TGroupList::Iterator CFontGroups::add(const TGroup &group)
{
    TGroupList::Iterator it=itsGroups.locate(group);

    if(it==itsGroups.end())
    {
        it=itsGroups.insert(itsGroups.end(), group);
        itsModified=true;
    }

    return it;
}

bool CFontGroups::refresh()
{
    CFontGroups newGroups(*this);

    if(merge(newGroups.itsGroups, itsGroups))
        itsModified=true;

    return itsModified;
}

bool CFontGroups::addTo(TGroupList::Iterator it, const TFont &font)
{
    if(it!=itsGroups.end() && !(*it).fonts.contains(font))
    {
        (*it).fonts.add(font);
        itsModified=true;
        return true;
    }
    return false;
}

bool CFontGroups::removeFrom(TGroupList::Iterator it, const TFont &font)
{
    TFontList::Iterator fIt;

    if(it!=itsGroups.end() && (*it).fonts.end()!=(fIt=(*it).fonts.locate(font)))
    {
        (*it).fonts.erase(fIt);
        itsModified=true;
        return true;
    }
    return false;
}

bool CFontGroups::setName(TGroupList::Iterator it, const QString &newName)
{
    if(it!=itsGroups.end() && !itsGroups.contains(TGroup(newName)))
    {
        (*it).name=newName;
        itsModified=true;
        return true;
    }
    return false;
}

void CFontGroups::removeFont(const TFont &font)
{
    TGroupList::Iterator it(itsGroups.begin()),
                         end(itsGroups.end());

    for(; it!=end; ++it)
        removeFrom(it, font);
}

CFontInfo::TGroupList::Iterator CFontGroups::find(const QString &name)
{
    TGroupList::Iterator it(itsGroups.begin()),
                         end(itsGroups.end());

    for(; it!=end; ++it)
        if((*it).name==name)
            break;

    return it;
}

}

/*
uint qHash(const KFI::CFontInfo::TFile &key)
{
    return qHash
    const QChar *p = key.path.unicode();
    int         n = key.path.size();
    uint        h = 0,
                g;

    h = (h << 4) + key.face;
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
*/
