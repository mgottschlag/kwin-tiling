#ifndef __FONT_GROUPS_H__
#define __FONT_GROUPS_H__

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

#include <QString>
#include <QStringList>
#include <QSet>
#include <QList>
#include <time.h>
#include "KfiConstants.h"
#include "Misc.h"
#include "kfontinst_export.h"

class QDomDocument;
class QDomElement;
class QTextStream;

namespace KFI
{

class KFONTINST_EXPORT CFontGroups
{
    public:

    struct TGroup
    {
        TGroup(const QString &n=QString()) : name(n) { }

        bool operator==(const TGroup &o) const { return name==o.name; }
        bool load(QDomElement &elem);

        QString       name;
        QSet<QString> families;
    };

    struct TGroupList : public QList<TGroup>  // Can't use a set, as may want to rename group
    {
        Iterator locate(const TGroup &t)    { int i = indexOf(t); return (-1==i ? end() : (begin()+i)); }
        void     add(const TGroup &t) const { (const_cast<TGroupList *>(this))->append(t); }
    };

    CFontGroups(const QString &path=QString(), bool abs=false);
    CFontGroups(const QString &file, const TGroup &group);

    ~CFontGroups()             { save(); }

    //
    // Refresh checks the timestap of the file to determine if changes have been
    // made elsewhere.
    bool refresh();
    void load();
    bool save();
    bool modifiable() const    { return itsModifiable; }
    bool modified() const      { return itsModified; }
    void merge(const CFontGroups &other);

    TGroupList::Iterator add(const TGroup &group);
    TGroupList::Iterator create(const QString &name) { return add(TGroup(name)); }
    bool addTo(const QString &group, const QString &family)
         { return addTo(itsGroups.locate(TGroup(group)), family); }
    bool removeFrom(const QString &group, const QString &family)
         { return removeFrom(itsGroups.locate(TGroup(group)), family); }
    bool addTo(TGroupList::Iterator it, const QString &family);
    bool removeFrom(TGroupList::Iterator it, const QString &family);
    bool setName(TGroupList::Iterator it, const QString &newName);
    void removeFamily(const QString &family);
    TGroupList::Iterator find(const QString &name);
    void                 remove(TGroupList::Iterator it) { itsGroups.erase(it);
                                                           itsModified=true; }
    TGroupList & items() { return itsGroups; }

    private:

    QString    itsFileName;
    time_t     itsTimeStamp;
    bool       itsModified,
               itsModifiable;
    TGroupList itsGroups;
};

}

KFONTINST_EXPORT QTextStream & operator<<(QTextStream &s, const KFI::CFontGroups::TGroup &g);

#endif
