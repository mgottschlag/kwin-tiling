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

#include "FontmapCreator.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "Misc.h"
#include "BufferedFile.h"
#include <qdir.h>
#include <qfileinfo.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <qcstring.h>
#include <qregexp.h>
#include <klocale.h>

static const QString  constUnknown    ("_____");
static const QCString constGSGuardStr ("% kfontinst ");

QString CFontmapCreator::getQtName(const QString &font)
{
    QString       newName(font);
    unsigned int  ch;
    bool          newWord=true;

    newName.replace(QRegExp("\\-"), "_");

    for(ch=0; ch<newName.length(); ++ch)
    {
        if(newName[ch].isSpace())
            newWord=true;
        else
        {
            if(newName[ch]==newName[ch].upper())
            {
                if(!newWord)
                    newName[ch]=newName[ch].lower();
            }
            else
                if(newName[ch]==newName[ch].lower())
                {
                    if(newWord)
                        newName[ch]=newName[ch].upper();
                }
            newWord=false;
        }
    }

    newName.replace(QRegExp(" "), "");
    return newName;
} 

CFontmapCreator::TListEntry * CFontmapCreator::locateTail(TListEntry *entry)
{
    if(NULL==entry || NULL==entry->next)
        return entry;
    else
        return locateTail(entry->next);
}

CFontmapCreator::TListEntry * CFontmapCreator::newListEntry(TListEntry **list, const QString &familyname, CFontEngine::EWidth width)
{
    TListEntry *entry=new TListEntry;

    if(entry)
    {
        entry->family.name=familyname;
        entry->family.width=width;

        if(NULL==*list)
            *list=entry;
        else
            locateTail(*list)->next=entry;
    }

    return entry;
}

CFontmapCreator::TListEntry * CFontmapCreator::locateFamily(TListEntry *entry, const QString &familyname, CFontEngine::EWidth width)
{
    if(NULL==entry)
        return NULL;
    else
        if(strcmp(entry->family.name.latin1(), familyname.latin1())==0 && entry->family.width==width)
            return entry;
        else
            return locateFamily(entry->next, familyname, width);
}

bool CFontmapCreator::insertNames(TFontEntry **entry, const QString &filename)
{
    bool status=false;

    if(NULL!=entry)
    {
        if(NULL==*entry)
            *entry=new TFontEntry;

        if(NULL!=*entry)
        {
            TSlant &slant=CFontEngine::ITALIC_NONE!=CKfiGlobal::fe().getItalic() ? (*entry)->italic : (*entry)->roman;
            
            if(QString::null==slant.filename)
            {
                slant.filename=filename;
                slant.psname=CKfiGlobal::fe().getPsName();
                status=true;
            }
        }
    }

    return status;
}

void CFontmapCreator::scanFiles(TListEntry **list, const QString &path)
{
    QDir dir(path);

    if(dir.isReadable())
    {
        const QFileInfoList *files=dir.entryInfoList();
 
        if(files)
        {
            QFileInfoListIterator it(*files);
            QFileInfo             *fInfo;
 
            for(; NULL!=(fInfo=it.current()); ++it)
                if("."!=fInfo->fileName() && ".."!=fInfo->fileName() && !fInfo->isDir() &&
                  (CFontEngine::isAType1(fInfo->fileName()) || CFontEngine::isATtf(fInfo->fileName())))
                {
                    emit step(i18n("Adding %1 to Fontmap").arg(fInfo->filePath()));

                    if(CKfiGlobal::fe().openFont(fInfo->filePath(), CFontEngine::NAME|CFontEngine::PROPERTIES))
                    {
                        TListEntry          *entry;
                        const QString       &familyname=CKfiGlobal::fe().getFamilyName();
                        CFontEngine::EWidth width=CKfiGlobal::fe().getWidth();
                        bool                newEntry=false,
                                            inserted=false;

                        if(NULL==(entry=locateFamily(*list, familyname, width)))
                        {
                            entry=newListEntry(list, familyname, width);
                            newEntry=true;
                        }

                        if(entry)
                        {
                            switch(CKfiGlobal::fe().getWeight())
                            {
                                default:
                                    inserted=false;
                                    break;
                                case CFontEngine::WEIGHT_THIN:
                                    inserted=insertNames(&(entry->family.thin), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_ULTRA_LIGHT:
                                    inserted=insertNames(&(entry->family.ultralight), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_EXTRA_LIGHT:
                                    inserted=insertNames(&(entry->family.extralight), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_DEMI:
                                    inserted=insertNames(&(entry->family.demi), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_LIGHT:
                                    inserted=insertNames(&(entry->family.light), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_BOOK:
                                    inserted=insertNames(&(entry->family.book), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_UNKNOWN:
                                case CFontEngine::WEIGHT_MEDIUM:
                                    inserted=insertNames(&(entry->family.medium), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_REGULAR:
                                    inserted=insertNames(&(entry->family.regular), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_SEMI_BOLD:
                                    inserted=insertNames(&(entry->family.semibold), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_DEMI_BOLD:
                                    inserted=insertNames(&(entry->family.demibold), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_BOLD:
                                    inserted=insertNames(&(entry->family.bold), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_EXTRA_BOLD:
                                    inserted=insertNames(&(entry->family.extrabold), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_ULTRA_BOLD:
                                    inserted=insertNames(&(entry->family.ultrabold), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_HEAVY:
                                    inserted=insertNames(&(entry->family.heavy), fInfo->filePath());
                                    break;
                                case CFontEngine::WEIGHT_BLACK:
                                    inserted=insertNames(&(entry->family.black), fInfo->filePath());
                                    break;
                            }

                            if(!inserted)
                            {
                                if(newEntry)
                                    entry->family.name=constUnknown;
                                else
                                    entry=newListEntry(list, constUnknown, width);
                                if(entry)
                                    insertNames(&(entry->family.medium), fInfo->filePath());
                            }
                        }
                        CKfiGlobal::fe().closeFont();
                    }
                }
        }
   }
}

void CFontmapCreator::outputReal(CBufferedFile &file, const QString &psname, const QString &filename)
{
    QCString str("/");

    str+=psname.latin1();
    str+=" (";
    str+=filename.latin1();
    str+=") ;";

    file.write(str);
}

void CFontmapCreator::outputAlias(CBufferedFile &file, const QString &family, const QString &style, const QString &alias)
{
    QCString name;

    name+=family;
    name+=style;

    if(strcmp(name, alias.latin1()))
    {
        QCString str("/");

        str+=name;
        str+=" /"; 
        str+=alias.latin1();;
        str+=" ;";
        file.write(str);
    }
}

void CFontmapCreator::outputPsEntry(CBufferedFile &file, const TSlant &slant)
{
    if(QString::null!=slant.psname)
    {
        outputReal(file, slant.psname, slant.filename);
 
        QString starOfficeName(slant.psname);
 
        starOfficeName.replace(QRegExp(" "), QChar('_'));
 
        if(strcmp(slant.psname.latin1(), starOfficeName.latin1()))
            outputAlias(file, starOfficeName, "", slant.psname);
    }
}

void CFontmapCreator::outputPsEntry(CBufferedFile &file, const TFontEntry *entry)
{
    if(NULL!=entry)
    {
        if(QString::null!=entry->roman.psname)
            outputPsEntry(file, entry->roman);
        if(QString::null!=entry->italic.psname)
            outputPsEntry(file, entry->italic);
    }
}

void CFontmapCreator::outputPsEntry(CBufferedFile &file, const TListEntry &entry)
{
    outputPsEntry(file, entry.family.thin);
    outputPsEntry(file, entry.family.ultralight);
    outputPsEntry(file, entry.family.extralight);
    outputPsEntry(file, entry.family.demi);
    outputPsEntry(file, entry.family.light);
    outputPsEntry(file, entry.family.book);
    outputPsEntry(file, entry.family.medium);
    outputPsEntry(file, entry.family.regular);
    outputPsEntry(file, entry.family.semibold);
    outputPsEntry(file, entry.family.demibold);
    outputPsEntry(file, entry.family.bold);
    outputPsEntry(file, entry.family.extrabold);
    outputPsEntry(file, entry.family.ultrabold);
    outputPsEntry(file, entry.family.heavy);
    outputPsEntry(file, entry.family.black);
}

// Hmm... horrible macro time
#define FIND_FONT_ENTRY(WEIGHT, SLANT) \
    if(NULL!=family.WEIGHT && QString::null!=family.WEIGHT->SLANT.psname) \
        return &(family.WEIGHT->SLANT);

const CFontmapCreator::TSlant * CFontmapCreator::findNormal(const TFontFamily &family)
{
    FIND_FONT_ENTRY(medium, roman)
    FIND_FONT_ENTRY(regular, roman)
    FIND_FONT_ENTRY(book, roman)
    FIND_FONT_ENTRY(light, roman)
    FIND_FONT_ENTRY(demi, roman)
    FIND_FONT_ENTRY(extralight, roman)
    FIND_FONT_ENTRY(ultralight, roman)
    FIND_FONT_ENTRY(thin, roman)
    FIND_FONT_ENTRY(semibold, roman)
    FIND_FONT_ENTRY(demibold, roman)
    FIND_FONT_ENTRY(bold, roman)
    FIND_FONT_ENTRY(extrabold, roman)
    FIND_FONT_ENTRY(ultrabold, roman)
    FIND_FONT_ENTRY(heavy, roman)
    FIND_FONT_ENTRY(black, roman)
    FIND_FONT_ENTRY(medium, italic)
    FIND_FONT_ENTRY(regular, italic)
    FIND_FONT_ENTRY(book, italic)
    FIND_FONT_ENTRY(light, italic)
    FIND_FONT_ENTRY(demi, italic)
    FIND_FONT_ENTRY(extralight, italic)
    FIND_FONT_ENTRY(ultralight, italic)
    FIND_FONT_ENTRY(thin, italic)
    FIND_FONT_ENTRY(semibold, italic)
    FIND_FONT_ENTRY(demibold, italic)
    FIND_FONT_ENTRY(bold, italic)
    FIND_FONT_ENTRY(extrabold, italic)
    FIND_FONT_ENTRY(ultrabold, italic)
    FIND_FONT_ENTRY(heavy, italic)
    FIND_FONT_ENTRY(black, italic)
    return NULL;
}

const CFontmapCreator::TSlant * CFontmapCreator::findBold(const TFontFamily &family)
{
    FIND_FONT_ENTRY(bold, roman)
    FIND_FONT_ENTRY(extrabold, roman)
    FIND_FONT_ENTRY(ultrabold, roman)
    FIND_FONT_ENTRY(heavy, roman)
    FIND_FONT_ENTRY(black, roman)
    FIND_FONT_ENTRY(demibold, roman)
    FIND_FONT_ENTRY(semibold, roman)
    FIND_FONT_ENTRY(medium, roman)
    FIND_FONT_ENTRY(regular, roman)
    FIND_FONT_ENTRY(book, roman)
    FIND_FONT_ENTRY(light, roman)
    FIND_FONT_ENTRY(demi, roman)
    FIND_FONT_ENTRY(extralight, roman)
    FIND_FONT_ENTRY(ultralight, roman)
    FIND_FONT_ENTRY(thin, roman)
    FIND_FONT_ENTRY(bold, italic)
    FIND_FONT_ENTRY(extrabold, italic)
    FIND_FONT_ENTRY(ultrabold, italic)
    FIND_FONT_ENTRY(heavy, italic)
    FIND_FONT_ENTRY(black, italic)
    FIND_FONT_ENTRY(demibold, italic)
    FIND_FONT_ENTRY(semibold, italic)
    FIND_FONT_ENTRY(medium, italic)
    FIND_FONT_ENTRY(regular, italic)
    FIND_FONT_ENTRY(book, italic)
    FIND_FONT_ENTRY(light, italic)
    FIND_FONT_ENTRY(demi, italic)
    FIND_FONT_ENTRY(extralight, italic)
    FIND_FONT_ENTRY(ultralight, italic)
    FIND_FONT_ENTRY(thin, italic)
    return NULL;
}

const CFontmapCreator::TSlant * CFontmapCreator::findBoldItalic(const TFontFamily &family)
{
    FIND_FONT_ENTRY(bold, italic)
    FIND_FONT_ENTRY(extrabold, italic)
    FIND_FONT_ENTRY(ultrabold, italic)
    FIND_FONT_ENTRY(heavy, italic)
    FIND_FONT_ENTRY(black, italic)
    FIND_FONT_ENTRY(demibold, italic)
    FIND_FONT_ENTRY(semibold, italic)
    FIND_FONT_ENTRY(medium, italic)
    FIND_FONT_ENTRY(regular, italic)
    FIND_FONT_ENTRY(book, italic)
    FIND_FONT_ENTRY(light, italic)
    FIND_FONT_ENTRY(demi, italic)
    FIND_FONT_ENTRY(extralight, italic)
    FIND_FONT_ENTRY(ultralight, italic)
    FIND_FONT_ENTRY(thin, italic)
    FIND_FONT_ENTRY(bold, roman)
    FIND_FONT_ENTRY(extrabold, roman)
    FIND_FONT_ENTRY(ultrabold, roman)
    FIND_FONT_ENTRY(heavy, roman)
    FIND_FONT_ENTRY(black, roman)
    FIND_FONT_ENTRY(demibold, roman)
    FIND_FONT_ENTRY(semibold, roman)
    FIND_FONT_ENTRY(medium, roman)
    FIND_FONT_ENTRY(regular, roman)
    FIND_FONT_ENTRY(book, roman)
    FIND_FONT_ENTRY(light, roman)
    FIND_FONT_ENTRY(demi, roman)
    FIND_FONT_ENTRY(extralight, roman)
    FIND_FONT_ENTRY(ultralight, roman)
    FIND_FONT_ENTRY(thin, roman)
    return NULL;
}

const CFontmapCreator::TSlant * CFontmapCreator::findItalic(const TFontFamily &family)
{
    FIND_FONT_ENTRY(medium, italic)
    FIND_FONT_ENTRY(regular, italic)
    FIND_FONT_ENTRY(book, italic)
    FIND_FONT_ENTRY(light, italic)
    FIND_FONT_ENTRY(demi, italic)
    FIND_FONT_ENTRY(extralight, italic)
    FIND_FONT_ENTRY(ultralight, italic)
    FIND_FONT_ENTRY(thin, italic)
    FIND_FONT_ENTRY(semibold, italic)
    FIND_FONT_ENTRY(demibold, italic)
    FIND_FONT_ENTRY(bold, italic)
    FIND_FONT_ENTRY(extrabold, italic)
    FIND_FONT_ENTRY(ultrabold, italic)
    FIND_FONT_ENTRY(heavy, italic)
    FIND_FONT_ENTRY(black, italic)
    FIND_FONT_ENTRY(medium, roman)
    FIND_FONT_ENTRY(regular, italic)
    FIND_FONT_ENTRY(book, roman)
    FIND_FONT_ENTRY(light, roman)
    FIND_FONT_ENTRY(demi, roman)
    FIND_FONT_ENTRY(extralight, roman)
    FIND_FONT_ENTRY(ultralight, roman)
    FIND_FONT_ENTRY(thin, roman)
    FIND_FONT_ENTRY(semibold, roman)
    FIND_FONT_ENTRY(demibold, roman)
    FIND_FONT_ENTRY(bold, roman)
    FIND_FONT_ENTRY(extrabold, roman)
    FIND_FONT_ENTRY(ultrabold, roman)
    FIND_FONT_ENTRY(heavy, roman)
    FIND_FONT_ENTRY(black, roman)
    return NULL;
}

#undef FIND_FONT_ENTRY

void CFontmapCreator::outputAliasEntry(CBufferedFile &file, const TSlant *slant, const QString &familyname, const QString &style)
{
    if(NULL!=slant)
        outputAlias(file, familyname, style, slant->psname);
}

void CFontmapCreator::outputAliasEntry(CBufferedFile &file, const TFontEntry *entry, const QString &familyname, const QString &style)
{
    if(NULL!=entry)
    {
        if(QString::null!=entry->roman.psname)
            outputAlias(file, familyname, style, entry->roman.psname);
        if(QString::null!=entry->italic.psname)
            outputAlias(file, familyname, style+"Italic", entry->italic.psname);
    }
}

void CFontmapCreator::outputAliasEntry(CBufferedFile &file, const TListEntry &entry, const QString &familyname)
{
    /* By default only generate alias for

        <Blank> i.e. no style
        Roman   This will be the same as the above
        Bold
        BoldItalic
        Italic

        The other styles will be output only if a corresponding file exists
        - generating aliases for these would create too many entries.
    */


    outputAliasEntry(file, findNormal(entry.family), familyname, "");
    outputAliasEntry(file, findNormal(entry.family), familyname, "-Roman");
    outputAliasEntry(file, findBold(entry.family), familyname, "-Bold");
    outputAliasEntry(file, findBoldItalic(entry.family), familyname, "-BoldItalic");
    outputAliasEntry(file, findItalic(entry.family), familyname, "-Italic");

    outputAliasEntry(file, entry.family.thin, familyname, "-Thin");
    outputAliasEntry(file, entry.family.ultralight, familyname, "-UltraLight");
    outputAliasEntry(file, entry.family.extralight, familyname, "-ExtraLight");
    outputAliasEntry(file, entry.family.demi, familyname, "-Demi");
    outputAliasEntry(file, entry.family.light, familyname, "-Light");
    outputAliasEntry(file, entry.family.book, familyname, "-Book");
    outputAliasEntry(file, entry.family.medium, familyname, "-Medium");
    outputAliasEntry(file, entry.family.regular, familyname, "-Regular");
    outputAliasEntry(file, entry.family.semibold, familyname, "-SemiBold");
    outputAliasEntry(file, entry.family.demibold, familyname, "-DemiBold");
    outputAliasEntry(file, entry.family.extrabold, familyname, "-ExtraBold");
    outputAliasEntry(file, entry.family.ultrabold, familyname, "-UltraBold");
    outputAliasEntry(file, entry.family.heavy, familyname, "-Heavy");
    outputAliasEntry(file, entry.family.black, familyname, "-Black");
}

void CFontmapCreator::outputResults(CBufferedFile &file, const TListEntry *entry)
{
    if(entry!=NULL)
    {
        QString familyname=getQtName(entry->family.name);

        outputPsEntry(file, *entry);
        if(strcmp(familyname.latin1(), constUnknown.latin1()))
            outputAliasEntry(file, *entry, familyname);

        outputResults(file, entry->next);
    }
}

void CFontmapCreator::emptyList(TListEntry **entry)
{
    if(*entry!=NULL)
    {
        emptyList(&((*entry)->next));
        delete *entry;
        *entry=NULL;
    }
}

bool CFontmapCreator::go(const QString &dir)
{
    bool status=false;
 
    CBufferedFile file(CKfiGlobal::cfg().getGhostscriptFile().local8Bit(), CBufferedFile::createGuard(constGSGuardStr, dir.local8Bit(), false), NULL, true, true);

    if(file)
    {
        TListEntry *list=NULL;

        scanFiles(&list, dir);

        emit step(i18n("Writing to Fontmap file"));

        outputResults(file, list);
        emptyList(&list);
        file.close();
        status=true;
    }
 
    return status; 
}

CFontmapCreator::TFontFamily::TFontFamily()
{
    thin=ultralight=extralight=demi=light=book=medium=regular=semibold=demibold=bold=extrabold=ultrabold=heavy=black=NULL;
}

CFontmapCreator::TFontFamily::~TFontFamily()
{
    if(thin)
        delete thin;
    if(ultralight)
        delete ultralight;
    if(extralight)
        delete extralight;
    if(demi)
        delete demi;
    if(light)
        delete light;
    if(book)
        delete book;
    if(medium)
        delete medium;
    if(regular)
        delete regular;
    if(semibold)
        delete semibold;
    if(demibold)
        delete demibold;
    if(bold)
        delete bold;
    if(extrabold)
        delete extrabold;
    if(ultrabold)
        delete ultrabold;
    if(heavy)
        delete heavy;
    if(black)
        delete black;
}
#include "FontmapCreator.moc"
