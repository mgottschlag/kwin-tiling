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

#include "KFileFont.h"
#include "KfiConstants.h"
#include "FcEngine.h"
#include "KfiConstants.h"
#include "Misc.h"
#include <QFile>
#include <QTextStream>
#include <kgenericfactory.h>
#include <kio/netaccess.h>
#include <kzip.h>
#include <ktempdir.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#define KFI_DBUG kDebug(7034)

static void addEntry(QString &existing, const QString &add)
{
    if(existing.length())
        existing.append("; ");
    existing.append(add);
}

//
// Check if all items in the list are the same...
static bool same(const QStringList &list)
{
    if(list.count()>1)
    {
        QStringList::ConstIterator it(list.begin()),
                                   end(list.end()),
                                   first(it);
        for(++it; it!=end; ++it)
            if(*first!=*it)
                return false;
    }

    return true;
}

static void combine(const QStringList &list, QString &str)
{
    QStringList::ConstIterator it(list.begin()),
                               end(list.end());

    for(; it!=end; ++it)
        addEntry(str, *it);
}

static int strToWeight(const QString &str)
{
    if(str.isEmpty())
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

static int strToWidth(const QString &str)
{
    if(str.isEmpty())
        return KFI_FC_WIDTH_NORMAL;
    else if(str.contains("UltraCondensed", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_ULTRACONDENSED;
    else if(str.contains("ExtraCondensed", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_EXTRACONDENSED;
    else if(str.contains("SemiCondensed", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_SEMICONDENSED;
    else if(str.contains("Condensed", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_CONDENSED;
    else if(str.contains("SemiExpanded", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_SEMIEXPANDED;
    else if(str.contains("UltraExpanded", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_ULTRAEXPANDED;
    else if(str.contains("ExtraExpanded", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_EXTRAEXPANDED;
    else if(str.contains("Expanded", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_EXPANDED;
    else
        return KFI_FC_WIDTH_NORMAL;
}

struct FoundryMap
{
    const char     *noticeStr,
                   *foundry;
    unsigned short len;
};

static const FoundryMap map[]=   // These are (mainly) taken from type1inst
{
    { "Bigelow",                            "B&H",         3},
    { "Adobe",                              "Adobe",       5},
    { "Bitstream",                          "Bitstream",   9},
    { "Monotype",                           "Monotype",    8},
    { "Linotype",                           "Linotype",    8},
    { "LINOTYPE-HELL",                      "Linotype",    0},
    { "IBM",                                "IBM",         3},
    { "URW",                                "URW",         3},
    { "International Typeface Corporation", "ITC",         3},
    { "Tiro Typeworks",                     "Tiro",        4},
    { "XFree86",                            "XFree86",     7},
    { "Microsoft",                          "Microsoft",   9},
    { "Omega",                              "Omega",       5},
    { "Font21",                             "Hwan",        4},
    { "HanYang System",                     "Hanyang",     7},
    { "Richard Mitchell",                   "Mitchell",    8},
    { "Doug Miles",                         "Miles",       5},
    { "Hank Gillette",                      "Gillette",    8},
    { "Three Islands Press",                "3ip",         3},
    { "MacroMind",                          "Macromind",   9},
    { "MWSoft",                             "MWSoft",      6},
    { "Digiteyes Multimedia",               "DigitEyes",   9},
    { "ZSoft",                              "ZSoft",       5},
    { "Title Wave",                         "Titlewave",   9},
    { "Southern Software",                  "Southern",    8},
    { "Reasonable Solutions",               "Reasonable", 10},
    { "David Rakowski",                     "Rakowski",    8},
    { "D. Rakowski",                        "Rakowski",    0},
    { "S. G. Moye",                         "Moye",        4},
    { "S.G. Moye",                          "Moye",        0},
    { "Andrew s. Meit",                     "Meit",        4},
    { "A.S.Meit",                           "Meit",        0},
    { "Hershey",                            "Hershey",     7},
    { "FontBank",                           "FontBank",    8},
    { "A. Carr",                            "Carr",        4},
    { "Brendel Informatik",                 "Brendel",     7},
    { "Jonathan Brecher",                   "Brecher",     7},
    { "SoftMaker",                          "Softmaker",   9},
    { "LETRASET",                           "Letraset",    8},
    { "Corel Corp",                         "Corel",       5},
    { "PUBLISHERS PARADISE",                "Paradise",    8},
    { "Publishers Paradise",                "Paradise",    0},
    { "Allied Corporation",                 "Allied",      6},
    { NULL,                                 NULL,          0}
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

static bool readAfm(const QString &file, QString &full, QString &family, QString &foundry,
                    QString &weight, QString &width, QString &spacing, QString &slant, QString &version)
{
    QFile f(file);
    bool  foundName=false,
          foundFamily=false;
    int   intSpacing=FC_PROPORTIONAL,
          intWidth=KFI_FC_WIDTH_NORMAL,
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
                    intWidth=strToWidth(full);
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
                    intSpacing= ( line.mid(13).contains("false", Qt::CaseInsensitive)
                                ? FC_PROPORTIONAL : FC_MONO );
                else if(0==line.indexOf("Notice "))
                    foundry=getFoundry(line.mid(7).toLatin1());
                else if(0==line.indexOf("Version "))
                    version=getFoundry(line.mid(8).toLatin1());
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

        if(foundName && FC_SLANT_ITALIC==intSlant &&
           (-1!=full.indexOf("Oblique") || -1!=full.indexOf("Slanted")))
            intSlant=FC_SLANT_OBLIQUE;
    }

    if(foundName && foundFamily)
    {
        weight=KFI::FC::weightStr(intWeight, false);
        width=KFI::FC::widthStr(intWidth, false);
        slant=KFI::FC::slantStr(intSlant, false);
        spacing=KFI::FC::spacingStr(intSpacing);

        if(foundry.isEmpty())
            foundry=i18n(KFI_UNKNOWN_FOUNDRY);

        return true;
    }

    return false;
}

typedef KGenericFactory<KFI::KFileFontPlugin> KFileFontPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kfile_font, KFileFontPluginFactory(KFI_NAME))

namespace KFI
{

KFileFontPlugin::KFileFontPlugin(QObject *parent, const QStringList& args)
               : KFilePlugin(parent, args)
{
    KGlobal::locale()->insertCatalog(KFI_CATALOGUE);

    addMimeType("application/x-font-ttf"),
    addMimeType("application/x-font-type1");
    addMimeType("application/x-font-bdf");
    addMimeType("application/x-font-pcf");
    addMimeType("application/x-font-otf");
    addMimeType("application/x-font-ttc");
    addMimeType("application/x-afm");
    addMimeType("fonts/package");
}

void KFileFontPlugin::addMimeType(const char *mime)
{
    KFileMimeTypeInfo            *info=addMimeTypeInfo(mime);
    KFileMimeTypeInfo::GroupInfo *group=addGroupInfo(info, "General", i18n("General"));

    addItemInfo(group, "Full", i18n("Full Name"), QVariant::String);
    addItemInfo(group, "Foundry", i18n("Foundry"), QVariant::String);
    addItemInfo(group, "Family", i18n("Family"), QVariant::String);
    addItemInfo(group, "Weight", i18n("Weight"), QVariant::String);
    addItemInfo(group, "Width", i18n("Width"), QVariant::String);
    addItemInfo(group, "Spacing", i18n("Spacing"), QVariant::String);
    addItemInfo(group, "Slant", i18n("Slant"), QVariant::String);
    addItemInfo(group, "Version", i18n("Version"), QVariant::String);
}

bool KFileFontPlugin::readInfo(KFileMetaInfo& info, uint what)
{
    QString     full,
                lastFull,
                family,
                foundry,
                weight,
                width,
                spacing,
                slant,
                version,
                fullAll,
                familyAll,
                foundryAll,
                weightAll,
                widthAll,
                spacingAll,
                slantAll,
                versionAll;
    QStringList families,
                weights,
                widths,
                spacings,
                slants;
    KUrl        url(info.url());
    KTempDir    *tempDir(NULL);
    QString     fName;
    bool        fontsProt  = KFI_KIO_FONTS_PROTOCOL == url.protocol(),
                fileProt   = "file"                 == url.protocol(),
                downloaded = false,
                status     = false;
    int         faceFrom(KFI::Misc::getIntQueryVal(url, KFI_KIO_FACE, 0)),
                faceTo=faceFrom ? faceFrom+1 : (fontsProt ? 1 : 10); // How to get num faces from fontconfig? don't know
                                                                     // Don't know - so just try 1st 10...

    what=0;

    KFI_DBUG << "Get font meta info for:" << url.prettyUrl() << " face from:" << faceFrom << " to " << faceTo << endl;

    if(!fontsProt && !fileProt && KIO::NetAccess::download(url, fName, NULL))
    {
        downloaded=true;
        url=KUrl(fName);
    }

    if(downloaded || fontsProt || fileProt)
    {
        if("application/x-afm"==info.mimeType())  // Then fontconfig can't give us the data :-(
            status=readAfm(url.path(), fullAll, familyAll, foundryAll, weightAll, widthAll,
                           spacingAll, slantAll, versionAll);
        else
        {
            if(!fontsProt && "fonts/package"==info.mimeType())
            {
                KZip zip(url.path());

                if(zip.open(QIODevice::ReadOnly))
                {
                    const KArchiveDirectory *zipDir=zip.directory();

                    if(zipDir)
                    {
                        QStringList fonts(zipDir->entries());

                        if(fonts.count())
                        {
                            QStringList::ConstIterator it(fonts.begin()),
                                                       end(fonts.end());

                            for(; it!=end; ++it)
                            {
                                const KArchiveEntry *entry=zipDir->entry(*it);

                                if(entry && entry->isFile())
                                {
                                    tempDir=new KTempDir(KStandardDirs::locateLocal("tmp", KFI_TMP_DIR_PREFIX));
                                    tempDir->setAutoRemove(true);

                                    ((KArchiveFile *)entry)->copyTo(tempDir->name());

                                    QString mime(KMimeType::findByPath(tempDir->name()+entry->name())->name());

                                    if(mime=="application/x-font-ttf" || mime=="application/x-font-otf" ||
                                       mime=="application/x-font-ttc" || mime=="application/x-font-type1")
                                    {
                                        url=KUrl::fromPath(tempDir->name()+entry->name());
                                        break;
                                    }
                                    else
                                        ::unlink(QFile::encodeName(tempDir->name()+entry->name()).data());
                                }
                            }
                        }
                    }
                }
            }

            for(int face=faceFrom; face<faceTo; ++face)
            {
                if(CFcEngine::instance()->getInfo(url, face, full, family, foundry, weight, width,
                                                  spacing, slant, version) && !full.isEmpty() && full!=lastFull)
                {
                    addEntry(fullAll, full);
                    lastFull=full;

                    KFI_DBUG << "Read meta data for face " << face << " fullname:" << full << endl;
                    if(faceFrom==face)
                    {
                        if(foundryAll.isEmpty())
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
                                    if(foundryAll.length()==entry->len &&
                                       -1!=foundryAll.indexOf(entry->foundry, false))
                                    {
                                        foundryAll=entry->foundry;
                                        break;
                                    }
                            }
                        }

                        if(versionAll.isEmpty())
                            versionAll=version;
                    }
                    families.append(family);
                    weights.append(weight);
                    widths.append(width);
                    spacings.append(spacing);
                    slants.append(slant);
                    status=true;
                }
                else
                    break;
            }
        }

        if(status)
        {
            KFileMetaInfoGroup group;

            group=appendGroup(info, "General");
            appendItem(group, "Full", fullAll);

            if(same(families) && same(weights) && same(widths) && same(spacings) && same(slants))
            {
                familyAll=family;
                weightAll=weight;
                widthAll=width;
                spacingAll=spacing;
                slantAll=slant;
                versionAll=version;
            }
            else
            {
                combine(families, familyAll);
                combine(weights, weightAll);
                combine(widths, widthAll);
                combine(spacings, spacingAll);
                combine(slants, slantAll);
            }
            appendItem(group, "Foundry", foundryAll);
            appendItem(group, "Family", familyAll);
            appendItem(group, "Weight", weightAll);
            appendItem(group, "Width", widthAll);
            appendItem(group, "Spacing", spacingAll);
            appendItem(group, "Slant", slantAll);
            appendItem(group, "Version", versionAll);
        }

        if(downloaded)
            KIO::NetAccess::removeTempFile(fName);
    }

    delete tempDir;

    return status;
}

}
