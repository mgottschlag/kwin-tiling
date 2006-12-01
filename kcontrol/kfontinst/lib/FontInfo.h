#ifndef __FONT_INFO_H__
#define __FONT_INFO_H__

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

class QDomDocument;
class QDomElement;

//
// Class used to store list of disabled fonts, and font groups, within an XML file.
namespace KFI
{

class CFontInfo
{
    public:

    struct TFile
    {
        TFile(const QString &p=QString(), int f=0) : path(p), face(f) { }

        bool operator==(const TFile &o) const { return face==o.face && path==o.path; } 
        bool load(QDomElement &elem);
        bool save(QDomDocument &doc, QDomElement &parent) const;

        operator QString() const { return path; }

        QString path;
        int     face;  // This is only really required for TTC fonts -> where a file will belong
    };                 // to more than one font.

    struct TFileList : public QList<TFile> // QSet<TFile>
    {
        Iterator locate(const TFile &t) { int i = indexOf(t); return (-1==i ? end() : (begin()+i)); }
        //Iterator locate(const TFile &t) { return find(t); }
        void     add(const TFile &t) const { (const_cast<TFileList *>(this))->append(t); }
    };

    struct TFont : public Misc::TFont
    {
        TFont(const QString &f=QString(), unsigned long s=KFI_NO_STYLE_INFO) : Misc::TFont(f, s) { }
        TFont(const Misc::TFont &f) : Misc::TFont(f) { }

        bool operator==(const TFont &o) const { return styleInfo==o.styleInfo && family==o.family; }
        bool load(QDomElement &elem, bool ignoreFiles=false);
        bool save(QDomDocument &doc, QDomElement &parent, bool ignoreFiles=false) const;

        const QString & getName() const;

        mutable QString name;
        TFileList       files;
    };

    struct TFontList : public QSet<TFont>
    {
        //Iterator locate(const TFont &t) { int i = indexOf(t); return (-1==i ? end() : (begin()+i)); }
        Iterator locate(const TFont &t);//       { return find(t); }
        Iterator locate(const Misc::TFont &t);// { return locate((TFont &)t); }
        void     add(const TFont &t) const;//    { (const_cast<TFontList *>(this))->insert(t); }
    };

    struct TGroup
    {
        TGroup(const QString &n=QString()) : name(n) { }

        bool operator==(const TGroup &o) const { return name==o.name; }
        bool load(QDomElement &elem);
        bool save(QDomDocument &doc, QDomElement &parent) const;

        QString   name;
        TFontList fonts;
    };

    struct TGroupList : public QList<TGroup>  // Can't use a set, as may want to rename group
    {
        Iterator locate(const TGroup &t) { int i = indexOf(t); return (-1==i ? end() : (begin()+i)); }
        //Iterator locate(const TGroup &t) { return find(t); }
        void     add(const TGroup &t) const { (const_cast<TGroupList *>(this))->append(t); }
    };

    static const QString theirDocumentTag;
    static const QString theirGroupTag;
    static const QString theirFontTag;

    CFontInfo(const QString &path, bool abs, bool sys, const QString &type);
    virtual ~CFontInfo()                     { save(); }

    //
    // Refresh checks the timestap of the file to determine if changes have been
    // made elsewhere.
    virtual bool       refresh();
    void               load(bool lock=true);
    bool               save();
    bool               modifiable() const    { return itsModifiable; }
    bool               modified() const      { return itsModified; }
    const TGroupList & groups() const        { return itsGroups; }
    const TFontList  & disabledFonts() const { return itsDisabledFonts; }
    void               remove(TGroupList::Iterator it) { itsGroups.erase(it);
                                                              itsModified=true; }
    void               remove(TFontList::Iterator it)  { itsDisabledFonts.erase(it);
                                                              itsModified=true; }

    bool                        merge(const CFontInfo &other);
    static bool                 merge(const TFontList &from, TFontList &to, bool ignoreFiles);
    static bool                 merge(const TGroupList &from, TGroupList &to);

    protected:

    CFontInfo(const CFontInfo &o);
    CFontInfo(const QString &file, const TGroup &group);

    protected:

    QString    itsFileName;
    time_t     itsTimeStamp;
    bool       itsModified,
               itsModifiable,
               itsConfig;
    TGroupList itsGroups;
    TFontList  itsDisabledFonts;
};

class CDisabledFonts : public CFontInfo
{
    public:

    CDisabledFonts(const QString &path=QString(), bool sys=false) : CFontInfo(path, false, sys, "disabledfonts") { }
    ~CDisabledFonts() { }

    bool disable(const QString &family, unsigned long styleInfo)
             { return disable(TFont(family, styleInfo)); }
    bool disable(const TFont &font);
    bool enable(const QString &family, unsigned long styleInfo)
             { return enable(TFont(family, styleInfo)); }
    bool enable(const TFont &font)
             { return enable(itsDisabledFonts.locate(font)); }
    bool enable(TFontList::Iterator font);

    TFontList::Iterator find(const QString &name, int face);

    TFontList & items() { return itsDisabledFonts; }
};

class CFontGroups : public CFontInfo
{
    public:

    CFontGroups(const QString &path=QString(), bool abs=false, bool sys=false,
                const QString &name="groups") : CFontInfo(path, abs, sys, name) { }
    CFontGroups(const QString &file, const TGroup &group) : CFontInfo(file, group) { }
    CFontGroups(const CFontGroups &o) : CFontInfo(o) { }
    ~CFontGroups() { }

    TGroupList::Iterator add(const TGroup &group);
    TGroupList::Iterator create(const QString &name) { return add(TGroup(name)); }
    bool refresh();
    bool addTo(const QString &group, const QString &family, unsigned long styleInfo)
         { return addTo(itsGroups.locate(TGroup(group)), TFont(family, styleInfo)); }
    bool removeFrom(const QString &group, const QString &family, unsigned long styleInfo)
         { return removeFrom(itsGroups.locate(TGroup(group)), TFont(family, styleInfo)); }
    bool addTo(TGroupList::Iterator it, const QString &family, unsigned long styleInfo)
         { return addTo(it, TFont(family, styleInfo)); }
    bool removeFrom(TGroupList::Iterator it, const QString &family, unsigned long styleInfo)
         { return removeFrom(it, TFont(family, styleInfo)); }
    bool addTo(TGroupList::Iterator it, const TFont &font);
    bool removeFrom(TGroupList::Iterator it, const TFont &font);
    bool setName(TGroupList::Iterator it, const QString &newName);
    void removeFont(const TFont &font);
    TGroupList::Iterator find(const QString &name);

    TGroupList & items() { return itsGroups; }
};

inline KDE_EXPORT uint qHash(const CFontInfo::TFont &key)
{
    return qHash((Misc::TFont&)key);
}

}

#endif
