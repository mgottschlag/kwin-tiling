#ifndef __DISABLED_FONTS_H__
#define __DISABLED_FONTS_H__

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

//
// Class used to store list of disabled fonts, and font groups, within an XML file.
namespace KFI
{

class KFONTINST_EXPORT CDisabledFonts
{
    public:

    struct TFile
    {
        TFile(const QString &p=QString(), int f=0) : path(p), face(f) { }

        bool operator==(const TFile &o) const { return face==o.face && path==o.path; } 
        bool load(QDomElement &elem);

        operator QString() const { return path; }

        QString path;
        int     face;  // This is only really required for TTC fonts -> where a file will belong
    };                 // to more than one font.

    struct TFileList : public QList<TFile>
    {
        Iterator locate(const TFile &t) { int i = indexOf(t); return (-1==i ? end() : (begin()+i)); }
        void     add(const TFile &t) const { (const_cast<TFileList *>(this))->append(t); }
    };

    struct KFONTINST_EXPORT TFont : public Misc::TFont
    {
        TFont(const QString &f=QString(), unsigned long s=KFI_NO_STYLE_INFO) : Misc::TFont(f, s) { }
        TFont(const Misc::TFont &f) : Misc::TFont(f) { }

        bool operator==(const TFont &o) const { return styleInfo==o.styleInfo && family==o.family; }
        bool load(QDomElement &elem, bool ignoreFiles=false);

        const QString & getName() const;

        mutable QString name;
        TFileList       files;
    };

    struct KFONTINST_EXPORT TFontList : public QSet<TFont>
    {
        Iterator locate(const TFont &t);
        Iterator locate(const Misc::TFont &t);
        void     add(const TFont &t) const;
    };

    CDisabledFonts(const QString &path=QString(), bool sys=false);
    ~CDisabledFonts()          { save(); }

    //
    // Refresh checks the timestap of the file to determine if changes have been
    // made elsewhere.
    bool refresh();
    void load();
    bool save();
    bool modifiable() const    { return itsModifiable; }
    bool modified() const      { return itsModified; }

    bool disable(const QString &family, unsigned long styleInfo)
             { return disable(TFont(family, styleInfo)); }
    bool disable(const TFont &font);
    bool enable(const QString &family, unsigned long styleInfo)
             { return enable(TFont(family, styleInfo)); }
    bool enable(const TFont &font)
             { return enable(itsDisabledFonts.locate(font)); }
    bool enable(TFontList::Iterator font);

    TFontList::Iterator find(const QString &name, int face);
    void                remove(TFontList::Iterator it)  { itsDisabledFonts.erase(it);
                                                          itsModified=true; }
    TFontList & items() { return itsDisabledFonts; }

    private:

    CDisabledFonts(const CDisabledFonts &o);
    void merge(const CDisabledFonts &other);

    private:

    QString   itsFileName;
    time_t    itsTimeStamp;
    bool      itsModified,
              itsModifiable;
    TFontList itsDisabledFonts;
};

inline KDE_EXPORT uint qHash(const CDisabledFonts::TFont &key)
{
    return qHash((Misc::TFont&)key);
}

}

KFONTINST_EXPORT QTextStream & operator<<(QTextStream &s, const KFI::CDisabledFonts::TFile &f);
KFONTINST_EXPORT QTextStream & operator<<(QTextStream &s, const KFI::CDisabledFonts::TFont &f);

#endif
