////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::KFileFont
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 20/03/2003
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "KFileFont.h"
#include "KfiConstants.h"
#include <qfile.h>
#include <qtextstream.h>
#include <kgenericfactory.h>
#include <kio/netaccess.h>

static void addEntry(int face, QString &existing, const QString &add)
{
    if(face>0)
        existing.append(", ");
    existing.append(add);
}

static int strToWeight(const QString &str)
{
#warning QString == NULL -> QString.isNull() ??
    if(str.isNull())
        return FC_WEIGHT_MEDIUM;
    else if(str.contains("Bold", Qt::CaseInsensitive))
        return FC_WEIGHT_BOLD;
    else if(str.contains("Heavy", Qt::CaseInsensitive))
        return FC_WEIGHT_HEAVY;
    else if(str.contains("Black", Qt::CaseInsensitive))
        return FC_WEIGHT_BLACK;
    else if(str.contains("ExtraBold", Qt::CaseInsensitive))
        return FC_WEIGHT_EXTRABOLD;
    else if(str.contains("UltraBold", Qt::CaseInsensitive))
        return FC_WEIGHT_ULTRABOLD;
    else if(str.contains("ExtraLight", Qt::CaseInsensitive))
        return FC_WEIGHT_EXTRALIGHT;
    else if(str.contains("UltraLight", Qt::CaseInsensitive))
        return FC_WEIGHT_ULTRALIGHT;
    else if(str.contains("Light", Qt::CaseInsensitive))
        return FC_WEIGHT_LIGHT;
    else if(str.contains("Medium", Qt::CaseInsensitive) ||
		    str.contains("Normal", Qt::CaseInsensitive) ||
		    str.contains("Roman", Qt::CaseInsensitive))
        return FC_WEIGHT_MEDIUM;
    else if(str.contains("Regular", Qt::CaseInsensitive))
        return FC_WEIGHT_REGULAR;
    else if(str.contains("SemiBold", Qt::CaseInsensitive))
        return FC_WEIGHT_SEMIBOLD;
    else if(str.contains("DemiBold", Qt::CaseInsensitive))
        return FC_WEIGHT_DEMIBOLD;
    else if(str.contains("Thin", Qt::CaseInsensitive))
        return FC_WEIGHT_THIN;
    else if(str.contains("Book", Qt::CaseInsensitive))
        return FC_WEIGHT_NORMAL;
    else if(str.contains("Demi", Qt::CaseInsensitive))
        return FC_WEIGHT_NORMAL;
    else
        return FC_WEIGHT_MEDIUM;
}

#ifndef KFI_FC_NO_WIDTHS
static int strToWidth(const QString &str)
{
    if(str.isEmpty())
        return FC_WIDTH_NORMAL;
    else if(str.contains("UltraCondensed", Qt::CaseInsensitive))
        return FC_WIDTH_ULTRACONDENSED;
    else if(str.contains("ExtraCondensed", Qt::CaseInsensitive))
        return FC_WIDTH_EXTRACONDENSED;
    else if(str.contains("SemiCondensed", Qt::CaseInsensitive))
        return FC_WIDTH_SEMICONDENSED;
    else if(str.contains("Condensed", Qt::CaseInsensitive))
        return FC_WIDTH_CONDENSED;
    else if(str.contains("SemiExpanded", Qt::CaseInsensitive))
        return FC_WIDTH_SEMIEXPANDED;
    else if(str.contains("UltraExpanded", Qt::CaseInsensitive))
        return FC_WIDTH_ULTRAEXPANDED;
    else if(str.contains("ExtraExpanded", Qt::CaseInsensitive))
        return FC_WIDTH_EXTRAEXPANDED;
    else if(str.contains("Expanded", Qt::CaseInsensitive))
        return FC_WIDTH_EXPANDED;
    else
        return FC_WIDTH_NORMAL;
}
#endif

struct FoundryMap
{
    const char     *noticeStr,
                   *foundry;
    unsigned short len;
};

static const FoundryMap map[]=   // These are (mainly) taken from type1inst
{
    { "Bigelow",                           "B&H",        3},
    { "Adobe",                             "Adobe",      5},
    { "Bitstream",                         "Bitstream",  9},
    { "Monotype",                          "Monotype",   8},
    { "Linotype",                          "Linotype",   8},
    { "LINOTYPE-HELL",                     "Linotype",   0},
    { "IBM",                               "IBM",        3},
    { "URW",                               "URW",        3},
    { "International Typeface Corporation", "ITC",        3},
    { "Tiro Typeworks",                    "Tiro",       4},
    { "XFree86",                           "XFree86",    7},
    { "Microsoft",                         "Microsoft",  9},
    { "Omega",                             "Omega",      5},
    { "Font21",                            "Hwan",       4},
    { "HanYang System",                    "Hanyang",    7},
    { "Richard Mitchell",                  "Mitchell",   8},
    { "Doug Miles",                        "Miles",      5},
    { "Hank Gillette",                     "Gillette",   8},
    { "Three Islands Press",               "3ip",        3},
    { "MacroMind",                         "Macromind",  9},
    { "MWSoft",                            "MWSoft",     6},
    { "Digiteyes Multimedia",              "DigitEyes",  9},
    { "ZSoft",                             "ZSoft",      5},
    { "Title Wave",                        "Titlewave",  9},
    { "Southern Software",                 "Southern",   8},
    { "Reasonable Solutions",              "Reasonable", 10},
    { "David Rakowski",                    "Rakowski",   8},
    { "D. Rakowski",                       "Rakowski",   0},
    { "S. G. Moye",                        "Moye",       4},
    { "S.G. Moye",                         "Moye",       0},
    { "Andrew s. Meit",                    "Meit",       4},
    { "A.S.Meit",                          "Meit",       0},
    { "Hershey",                           "Hershey",    7},
    { "FontBank",                          "FontBank",   8},
    { "A. Carr",                           "Carr",       4},
    { "Brendel Informatik",                "Brendel",    7},
    { "Jonathan Brecher",                  "Brecher",    7},
    { "SoftMaker",                         "Softmaker",  9},
    { "LETRASET",                          "Letraset",   8},
    { "Corel Corp",                        "Corel",      5},
    { "PUBLISHERS PARADISE",               "Paradise",   8},
    { "Publishers Paradise",               "Paradise",   0},
    { "Allied Corporation",                "Allied",     6},
    { NULL,                                NULL,         0}
};

static const char * getFoundry(const char *notice)
{
    const FoundryMap *entry;

    if(notice)
        for(entry=map; NULL!=entry->foundry; entry++)
            if(NULL!=strstr(notice, entry->noticeStr))
                return entry->foundry;

    return NULL;
}

static bool readAfm(const QString &file, QString &full, QString &family, QString &foundry, QString &weight,
#ifndef KFI_FC_NO_WIDTHS
                    QString &width,
#endif
                    QString &spacing, QString &slant)
{
    QFile f(file);
    bool  foundName=false,
          foundFamily=false;
    int   intSpacing=FC_PROPORTIONAL,
#ifndef KFI_FC_NO_WIDTHS
          intWidth=FC_WIDTH_NORMAL,
#endif
          intWeight=FC_WEIGHT_NORMAL,
          intSlant=FC_SLANT_ROMAN;

    if(f.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&f);
        QString     line;
        bool        inMetrics=false;

        while(!stream.atEnd())
        {
            line=stream.readLine();
            line=line.simplified();

            if(inMetrics)
            {
                if(0==line.indexOf("FullName "))
                {
                    full=line.mid(9);
#ifndef KFI_FC_NO_WIDTHS
                    intWidth=strToWidth(full);
#endif
                    foundName=true;
                }
                else if(0==line.indexOf("FamilyName "))
                {
                    family=line.mid(11);
                    foundFamily=true;
                }
                else if(0==line.indexOf("Weight "))
                    intWeight=strToWeight(line.mid(7));
                else if(0==line.indexOf("ItalicAngle "))
                    intSlant=0.0f==line.mid(12).toFloat() ? FC_SLANT_ROMAN : FC_SLANT_ITALIC;
                else if(0==line.indexOf("IsFixedPitch "))
                    intSpacing= ( line.mid(13).contains("false", Qt::CaseInsensitive) ? FC_PROPORTIONAL : FC_MONO );
                else if(0==line.indexOf("Notice "))
                    foundry=getFoundry(line.mid(7).toLatin1());
                else if(0==line.indexOf("StartCharMetrics"))
                    break;
            }
            else
                if(0==line.indexOf("StartFontMetrics"))
                    inMetrics=true;
        };
        f.close();

        if(!foundFamily && foundName)
        {
            family=full;
            foundFamily=true;
        }

        if(foundName && FC_SLANT_ITALIC==intSlant && (-1!=full.indexOf("Oblique") || -1!=full.indexOf("Slanted")))
            intSlant=FC_SLANT_OBLIQUE;
    }

    if(foundName && foundFamily)
    {
        weight=KFI::CFcEngine::weightStr(intWeight, false);
#ifndef KFI_FC_NO_WIDTHS
        width=KFI::CFcEngine::widthStr(intWidth, false);
#endif
        slant=KFI::CFcEngine::slantStr(intSlant, false);
        spacing=KFI::CFcEngine::spacingStr(intSpacing);

        if(foundry.isEmpty())
            foundry=i18n(KFI_UNKNOWN_FOUNDRY);

        return true;
    }

    return false;
}

typedef KGenericFactory<KFI::KFileFontPlugin> KFileFontPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kfile_font, KFileFontPluginFactory("kfontinst"))

namespace KFI
{

KFileFontPlugin::KFileFontPlugin(QObject *parent, const char *name, const QStringList& args)
               : KFilePlugin(parent, args)
{
    KGlobal::locale()->insertCatalog(KFI_CATALOGUE);

    addMimeType("application/x-font-ttf"),
    addMimeType("application/x-font-type1");
    //addMimeType("application/x-font-speedo");
    addMimeType("application/x-font-bdf");
    addMimeType("application/x-font-pcf");
    //addMimeType("application/x-font-snf");
    addMimeType("application/x-font-otf");
    addMimeType("application/x-font-ttc");
    addMimeType("application/x-afm");
}

void KFileFontPlugin::addMimeType(const char *mime)
{
    KFileMimeTypeInfo            *info=addMimeTypeInfo(mime);
    KFileMimeTypeInfo::GroupInfo *group=addGroupInfo(info, "General", i18n("General"));

    addItemInfo(group, "Full", i18n("Full Name"), QVariant::String);
    addItemInfo(group, "Family", i18n("Family"), QVariant::String);
    addItemInfo(group, "Foundry", i18n("Foundry"), QVariant::String);
    addItemInfo(group, "Weight", i18n("Weight"), QVariant::String);
#ifndef KFI_FC_NO_WIDTHS
    addItemInfo(group, "Width", i18n("Width"), QVariant::String);
#endif
    addItemInfo(group, "Spacing", i18n("Spacing"), QVariant::String);
    addItemInfo(group, "Slant", i18n("Slant"), QVariant::String);
}

bool KFileFontPlugin::readInfo(KFileMetaInfo& info, uint what)
{
    QString full,
            lastFull,
            family,
            foundry,
            weight,
#ifndef KFI_FC_NO_WIDTHS
            width,
#endif
            spacing,
            slant,
            fullAll,
            familyAll,
            foundryAll,
            weightAll,
#ifndef KFI_FC_NO_WIDTHS
            widthAll,
#endif
            spacingAll,
            slantAll;
    KUrl    url(info.url());
    QString fName;
    bool    fontsProt  = KFI_KIO_FONTS_PROTOCOL == url.protocol(),
            fileProt   = "file"             == url.protocol(),
            downloaded = false,
            status     = false;

    what=0;

    if(!fontsProt && !fileProt && KIO::NetAccess::download(url, fName, NULL))
    {
        downloaded=true;
        url=KUrl(fName);
    }

    if(downloaded || fontsProt || fileProt)
    {
        if("application/x-afm"==info.mimeType())  // Then fontconfig can't give us the data :-(
            status=readAfm(url.path(), fullAll, familyAll, foundryAll, weightAll,
#ifndef KFI_FC_NO_WIDTHS
                           widthAll,
#endif
                           spacingAll, slantAll);
        else
            for(int face=0; face<10; ++face)  // How to get num faces from fontconfig? don't know - so just try 1st 10...
            {
                if(itsEngine.getInfo(url, face, full, family, foundry, weight,
#ifndef KFI_FC_NO_WIDTHS
                                     width,
#endif
                                     spacing, slant) &&
                   !full.isEmpty() && full!=lastFull)
                {
                    addEntry(face, fullAll, full);
                    lastFull=full;

                    if(KFileMetaInfo::Fastest!=what)
                    {
                        addEntry(face, familyAll, family);
                        if(0==face)
                        {
                            foundryAll=foundry;

                            if(foundryAll.isEmpty())
                                foundryAll=i18n(KFI_UNKNOWN_FOUNDRY);
                            else
                            {
                                // Try to make sure foundry is capitalised, and looks the same as that of
                                // any AFM.
                                foundryAll[0]=foundryAll[0].toUpper();

                                const FoundryMap *entry;

                                for(entry=map; NULL!=entry->foundry; entry++)
                                    if(foundryAll.length()==entry->len && foundryAll.contains(entry->foundry, Qt::CaseInsensitive))
                                    {
                                        foundryAll=entry->foundry;
                                        break;
                                    }
                            }
                        }
                        addEntry(face, weightAll, weight);
#ifndef KFI_FC_NO_WIDTHS
                        addEntry(face, widthAll, width);
#endif
                        addEntry(face, spacingAll, spacing);
                        addEntry(face, slantAll, slant);
                    }
                    status=true;
                }
                else
                    break;
            }

        if(status)
        {
            KFileMetaInfoGroup group;

            group=appendGroup(info, "General");
            appendItem(group, "Full", fullAll);

            if(KFileMetaInfo::Fastest!=what)
            {
                appendItem(group, "Family", familyAll);
                appendItem(group, "Foundry", foundryAll);
                appendItem(group, "Weight", weightAll);
#ifndef KFI_FC_NO_WIDTHS
                appendItem(group, "Width", widthAll);
#endif
                appendItem(group, "Spacing", spacingAll);
                appendItem(group, "Slant", slantAll);
            }
        }

        if(downloaded)
            KIO::NetAccess::removeTempFile(fName);
    }

    return status;
}

}
