#ifndef __FONTMAP_CREATOR_H__
#define __FONTMAP_CREATOR_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontmapCreator
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 03/05/2001
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
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include <qstring.h>
#include <qobject.h>
#include "FontEngine.h"

class CBufferedFile;

class CFontmapCreator : public QObject
{
    Q_OBJECT

    private:

    struct TSlant
    {
        QString psname,
                filename;
    };

    struct TFontEntry
    {
        TSlant roman,
               italic;
    };

    struct TFontFamily
    {
        TFontFamily();
        ~TFontFamily();

        QString             name;
        CFontEngine::EWidth width;
        TFontEntry          *thin,
                            *ultralight,
                            *extralight,
                            *demi,
                            *light,
                            *book,
                            *regular,
                            *medium,
                            *semibold,
                            *demibold,
                            *bold,
                            *extrabold,
                            *ultrabold,
                            *heavy,
                            *black;
    };

    struct TListEntry
    {
        TListEntry() : next(NULL) {}

        TFontFamily family;
        TListEntry  *next;
    };

    public:

    enum EStatus
    {
        SUCCESS,
        COULD_NOT_CREATE_TEMP_FONTMAP,
        COULD_NOT_MERGE_FILES
    };

    public:

    CFontmapCreator() : QObject(NULL, NULL) {}

    bool         go(const QString &dir);

    signals:

    void         step(const QString &);

    private:

    QString      getQtName(const QString &font);
    TListEntry * locateTail(TListEntry *entry);
    TListEntry * newListEntry(TListEntry **list, const QString &familyname, CFontEngine::EWidth width);
    TListEntry * locateFamily(TListEntry *entry, const QString &familyname, CFontEngine::EWidth width);
    bool         insertNames(TFontEntry **entry, const QString &filename);
    void         scanFiles(TListEntry **list, const QString &path);
    void         outputReal(CBufferedFile &file, const QString &psname, const QString &filename);
    void         outputAlias(CBufferedFile &file, const QString &family, const QString &style, const QString &alias);
    void         outputPsEntry(CBufferedFile &file, const TSlant &slant);
    void         outputPsEntry(CBufferedFile &file, const TFontEntry *entry);
    void         outputPsEntry(CBufferedFile &file, const TListEntry &entry);
    const        TSlant * findNormal(const TFontFamily &family);
    const        TSlant * findBold(const TFontFamily &family);
    const        TSlant * findBoldItalic(const TFontFamily &family);
    const        TSlant * findItalic(const TFontFamily &family);
    void         outputAliasEntry(CBufferedFile &file, const TSlant *slant, const QString &familyname, const QString &style);
    void         outputAliasEntry(CBufferedFile &file, const TFontEntry *entry, const QString &familyname, const QString &style);
    void         outputAliasEntry(CBufferedFile &file, const TListEntry &entry, const QString &familyname);
    void         outputResults(CBufferedFile &file, const TListEntry *entry);
    void         emptyList(TListEntry **entry);
};

#endif
