#include "kfile_font.h"

// CPD: How to sym-link files in CVS?
#include "FontEngine.cpp"
#include "Misc.cpp"
#include "CompressedFile.cpp"
#include <kgenericfactory.h>
#include <klibloader.h>

typedef KGenericFactory<KFileFontPlugin> KFileFontPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kfile_font, KFileFontPluginFactory("kfile_font"));

KFileFontPlugin::KFileFontPlugin(QObject *parent, const char *name, const QStringList& args)
               : KFilePlugin(parent, name, args)
{
    static const int constNumTypes=2;

    KFileMimeTypeInfo *info[constNumTypes];

    info[0] = addMimeTypeInfo("application/x-font-ttf"),
    info[1] = addMimeTypeInfo("application/x-font-type1");

    for(int t=0; t<constNumTypes; ++t)
    {
        KFileMimeTypeInfo::GroupInfo *group;
        group=addGroupInfo(info[t], "General", i18n("General"));
        addItemInfo(group, "Full", i18n("Full Name"), QVariant::String);
        //addItemInfo(group, "KFile::MetaSummary", QString::null, QVariant::String);
        addItemInfo(group, "Family", i18n("Family"), QVariant::String);
        addItemInfo(group, "PostScript", i18n("PostScript Name"), QVariant::String);
        addItemInfo(group, "KFile::Summary", i18n("Summary"), QVariant::String);
        addItemInfo(group, "Foundry", i18n("Foundry"), QVariant::String);
        addItemInfo(group, "Weight", i18n("Weight"),  QVariant::String);
        addItemInfo(group, "Width", i18n("Width"), QVariant::String);
        addItemInfo(group, "Spacing", i18n("Spacing"),  QVariant::String);
        addItemInfo(group, "Slant", i18n("Slant"), QVariant::String);
    }
}

KFileFontPlugin::~KFileFontPlugin()
{
}

static QString toStr(CFontEngine::ESpacing v)
{
    switch(v)
    {
        case CFontEngine::SPACING_MONOSPACED:
            return i18n("Monospaced");
        case CFontEngine::SPACING_PROPORTIONAL:
            return i18n("Proportional");
    }
}

static QString toStr(CFontEngine::EItalic v)
{
    switch(v)
    {
        case CFontEngine::ITALIC_NONE:
            return i18n("Roman");
        case CFontEngine::ITALIC_ITALIC:
            return i18n("Italic");
        case CFontEngine::ITALIC_OBLIQUE:
            return i18n("Oblique");
    }
}

QString toStr(enum CFontEngine::EWeight v)
{
    switch(v)
    {
        case CFontEngine::WEIGHT_THIN:
            return i18n("Thin");
        case CFontEngine::WEIGHT_ULTRA_LIGHT:
            return i18n("Ultra Light");
        case CFontEngine::WEIGHT_EXTRA_LIGHT:
            return i18n("Extra Light");
        case CFontEngine::WEIGHT_DEMI:
            return i18n("Demi");
        case CFontEngine::WEIGHT_LIGHT:
            return i18n("Light");
        case CFontEngine::WEIGHT_BOOK:
            return i18n("Book");
        case CFontEngine::WEIGHT_MEDIUM:
            return i18n("Medium");
        case CFontEngine::WEIGHT_REGULAR:
            return i18n("Regular");
        case CFontEngine::WEIGHT_SEMI_BOLD:
            return i18n("Semi Bold");
        case CFontEngine::WEIGHT_DEMI_BOLD:
            return i18n("Demi Bold");
        case CFontEngine::WEIGHT_BOLD:
            return i18n("Bold");
        case CFontEngine::WEIGHT_EXTRA_BOLD:
            return i18n("Extra Bold");
        case CFontEngine::WEIGHT_ULTRA_BOLD:
            return i18n("Ultra Bold");
        case CFontEngine::WEIGHT_HEAVY:
            return i18n("Heavy");
        case CFontEngine::WEIGHT_BLACK:
            return i18n("Black");
        case CFontEngine::WEIGHT_UNKNOWN:
        default:
            return i18n("Medium");
    }
}

QString toStr(CFontEngine::EWidth v)
{   
    switch(v)
    {
        case CFontEngine::WIDTH_ULTRA_CONDENSED:
            return i18n("Ultra Condensed");
        case CFontEngine::WIDTH_EXTRA_CONDENSED:
            return i18n("Extra Condensed");
        case CFontEngine::WIDTH_CONDENSED:
            return i18n("Condensed");
        case CFontEngine::WIDTH_SEMI_CONDENSED:
            return i18n("Semi Condensed");
        case CFontEngine::WIDTH_SEMI_EXPANDED:
            return i18n("Semi Expanded");
        case CFontEngine::WIDTH_EXPANDED:
            return i18n("Expanded");
        case CFontEngine::WIDTH_EXTRA_EXPANDED:
            return i18n("Extra Expanded");
        case CFontEngine::WIDTH_ULTRA_EXPANDED:
            return i18n("Ultra Expanded");
        case CFontEngine::WIDTH_NORMAL:
        case CFontEngine::WIDTH_UNKNOWN:
        default:
            return i18n("Normal");
    }
}

bool KFileFontPlugin::readInfo(KFileMetaInfo& info, uint what = KFileMetaInfo::Fastest)
{
    what=0;

    if(itsFontEngine.openFont(info.path(), KFileMetaInfo::Fastest==what
                                               ? CFontEngine::NAME
                                               : CFontEngine::NAME|CFontEngine::PROPERTIES|CFontEngine::XLFD))
    {
        KFileMetaInfoGroup group;

        group=appendGroup(info, "General");
        appendItem(group, "Full", itsFontEngine.getFullName());
        //appendItem(group, "KFile::MetaSummary", itsFontEngine.getFullName());

        if(KFileMetaInfo::Fastest!=what)
        {
            appendItem(group, "Family", itsFontEngine.getFamilyName());
            appendItem(group, "PostScript", itsFontEngine.getPsName());
            appendItem(group, "Foundry", itsFontEngine.getFoundry());
            appendItem(group, "Weight", toStr(itsFontEngine.getWeight()));
            appendItem(group, "Width", toStr(itsFontEngine.getWidth()));
            appendItem(group, "Spacing", toStr(itsFontEngine.getSpacing()));
            appendItem(group, "Slant", toStr(itsFontEngine.getItalic()));
        }

        itsFontEngine.closeFont();
        return true;
    }
    return false;
}

//#include "kfontmetainfo.moc"
