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

#include "FontGroups.h"
#include "FcEngine.h"
#include "Misc.h"
#include "KfiConstants.h"
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QTextStream>
#include <ksavefile.h>
#include <klocale.h>
#include <fontconfig/fontconfig.h>

namespace KFI
{

#define GROUPS_DOC    "groups"
#define GROUP_TAG     "group"
#define NAME_ATTR     "name"
#define FAMILY_TAG    "family"

CFontGroups::CFontGroups(const QString &path, bool abs)
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
            FcStrList   *list=FcConfigGetFontDirs(FcInitLoadConfig());
            QStringList dirs;
            FcChar8     *dir;

            while((dir=FcStrListNext(list)))
                dirs.append(Misc::dirSyntax((const char *)dir));

            QString home(Misc::dirSyntax(QDir::homePath())),
                    defaultDir(Misc::dirSyntax(QDir::homePath()+"/.fonts/"));

            p=Misc::getFolder(defaultDir, home, dirs);
        }
        else
            p=path;

        itsFileName=p+"/"KFI_GROUPS_FILE;
    }
    itsModifiable=Misc::fWritable(itsFileName) ||
                  (!Misc::fExists(itsFileName) && Misc::dWritable(Misc::getDir(itsFileName)));
    load();
}

CFontGroups::CFontGroups(const QString &file, const TGroup &group)
           : itsFileName(file),
             itsTimeStamp(0),
             itsModified(true),
             itsModifiable(true)
{
    itsGroups.add(group);
}

bool CFontGroups::refresh()
{
    bool update=Misc::getTimeStamp(itsFileName)!=itsTimeStamp;
    save();
    load();
    return update;
}

void CFontGroups::load()
{
    time_t ts=Misc::getTimeStamp(itsFileName);

    if(!ts || ts!=itsTimeStamp)
    {
        itsTimeStamp=ts;

        QFile f(itsFileName);

        if(f.open(IO_ReadOnly))
        {
            itsModified=false;

            QDomDocument doc;

            if(doc.setContent(&f))
                for(QDomNode n=doc.documentElement().firstChild(); !n.isNull(); n=n.nextSibling())
                {
                    QDomElement e=n.toElement();

                    if(GROUP_TAG==e.tagName())
                    {
                        TGroup group;

                        if(group.load(e))
                            itsGroups.add(group);
                    }
                }
            f.close();
        }
    }
}

bool CFontGroups::save()
{
    bool rv=false;

    if(itsModified)
    {
        KSaveFile file(itsFileName);

        if(file.open())
        {
            QTextStream str(&file);

            str << "<"GROUPS_DOC">" << endl;

            TGroupList::Iterator it(itsGroups.begin()),
                                end(itsGroups.end());

            for(; it!=end; ++it)
                str << (*it);
            str << "</"GROUPS_DOC">" << endl;
            itsModified=false;
            rv=file.finalize();
        }
    }

    if(rv)
        itsTimeStamp=Misc::getTimeStamp(itsFileName);
    return rv;
}

void CFontGroups::merge(const CFontGroups &other)
{
    TGroupList::ConstIterator it(other.itsGroups.begin()),
                              end(other.itsGroups.end());
    bool                      modified(false);

    for(; it!=end; ++it)
    {
        TGroupList::Iterator existing(itsGroups.locate(*it));

        if(existing!=itsGroups.end())
        {
            int b4((*existing).families.count());

            (*existing).families+=(*it).families;
            if(b4!=(*existing).families.count())
                modified=true;
        }
        else
        {
            itsGroups.add(*it);
            modified=true;
        }
    }
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

bool CFontGroups::addTo(TGroupList::Iterator it, const QString &family)
{
    if(it!=itsGroups.end() && !(*it).families.contains(family))
    {
        (*it).families.insert(family);
        itsModified=true;
        return true;
    }
    return false;
}

bool CFontGroups::removeFrom(TGroupList::Iterator it, const QString &family)
{
    QSet<QString>::Iterator fIt;

    if(it!=itsGroups.end() && (*it).families.end()!=(fIt=(*it).families.find(family)))
    {
        (*it).families.erase(fIt);
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

void CFontGroups::removeFamily(const QString &family)
{
    TGroupList::Iterator it(itsGroups.begin()),
                         end(itsGroups.end());

    for(; it!=end; ++it)
        removeFrom(it, family);
}

CFontGroups::TGroupList::Iterator CFontGroups::find(const QString &name)
{
    TGroupList::Iterator it(itsGroups.begin()),
                         end(itsGroups.end());

    for(; it!=end; ++it)
        if((*it).name==name)
            break;

    return it;
}

bool CFontGroups::TGroup::load(QDomElement &elem)
{
    if(elem.hasAttribute(NAME_ATTR))
    {
        name=elem.attribute(NAME_ATTR);

        for(QDomNode n=elem.firstChild(); !n.isNull(); n=n.nextSibling())
        {
            QDomElement ent=n.toElement();

            if(FAMILY_TAG==ent.tagName())
                families.insert(ent.text());
            }

        return true; // Can have empty groups!
    }
    return false;
}

}

QTextStream & operator<<(QTextStream &s, const KFI::CFontGroups::TGroup &g)
{
    s << "  <"GROUP_TAG" "NAME_ATTR"=\"" << g.name << "\">" << endl;
    if(g.families.count())
    {
        QSet<QString>::ConstIterator it(g.families.begin()),
                                     end(g.families.end());

        for(; it!=end; ++it)
            s << "    <"FAMILY_TAG">" << (*it) << "</"FAMILY_TAG">" << endl;
    }
    s << "  </"GROUP_TAG">" << endl;
    return s;
}
