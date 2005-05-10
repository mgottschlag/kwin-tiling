#ifndef __KFI_CONSTANTS_H__
#define __KFI_CONSTANTS_H__

#include <klocale.h>

#define KFI_CATALOGUE                "kfontinst"

// io-slave
#define KFI_KIO_FONTS_PROTOCOL       "fonts"
#define KFI_KIO_FONTS_USER           I18N_NOOP("Personal")
#define KFI_KIO_FONTS_SYS            I18N_NOOP("System")

// Config
#define KFI_UI_CFG_FILE              KFI_CATALOGUE"uirc"
#define KFI_CFG_FILE                 KFI_CATALOGUE"rc"
#define KFI_ROOT_CFG_DIR             "/etc/fonts/"
#define KFI_ROOT_CFG_FILE            KFI_ROOT_CFG_DIR KFI_CFG_FILE
#define KFI_CFG_X_KEY                "ConfigureX"
#define KFI_CFG_GS_KEY               "ConfigureGS"
#define KFI_DEFAULT_CFG_X            true
#define KFI_DEFAULT_CFG_GS           false

// KIO::special

namespace KFI
{

enum ESpecial
{
    SPECIAL_RECONFIG = 0
};

}

// Font name...
#define KFI_WEIGHT_THIN              I18N_NOOP("Thin")
#define KFI_WEIGHT_EXTRALIGHT        I18N_NOOP("ExtraLight")
#define KFI_WEIGHT_ULTRALIGHT        I18N_NOOP("UltraLight")
#define KFI_WEIGHT_LIGHT             I18N_NOOP("Light")
#define KFI_WEIGHT_REGULAR           I18N_NOOP("Regular")
#define KFI_WEIGHT_NORMAL            I18N_NOOP("Normal")
#define KFI_WEIGHT_MEDIUM            I18N_NOOP("Medium")
#define KFI_WEIGHT_DEMIBOLD          I18N_NOOP("DemiBold")
#define KFI_WEIGHT_SEMIBOLD          I18N_NOOP("SemiBold")
#define KFI_WEIGHT_BOLD              I18N_NOOP("Bold")
#define KFI_WEIGHT_EXTRABOLD         I18N_NOOP("ExtraBold")
#define KFI_WEIGHT_ULTRABOLD         I18N_NOOP("UltraBold")
#define KFI_WEIGHT_BLACK             I18N_NOOP("Black")
#define KFI_WEIGHT_HEAVY             I18N_NOOP("Heavy")

#define KFI_SLANT_ROMAN              I18N_NOOP("Roman")
#define KFI_SLANT_ITALIC             I18N_NOOP("Italic")
#define KFI_SLANT_OBLIQUE            I18N_NOOP("Oblique")

#define KFI_WIDTH_ULTRACONDENSED     I18N_NOOP("UltraCondensed")
#define KFI_WIDTH_EXTRACONDENSED     I18N_NOOP("ExtraCondensed")
#define KFI_WIDTH_CONDENSED          I18N_NOOP("Condensed")
#define KFI_WIDTH_SEMICONDENSED      I18N_NOOP("SemiCondensed")
#define KFI_WIDTH_NORMAL             I18N_NOOP("Normal")
#define KFI_WIDTH_SEMIEXPANDED       I18N_NOOP("SemiExpanded")
#define KFI_WIDTH_EXPANDED           I18N_NOOP("Expanded")
#define KFI_WIDTH_EXTRAEXPANDED      I18N_NOOP("ExtraExpanded")
#define KFI_WIDTH_ULTRAEXPANDED      I18N_NOOP("UltraExpanded")

#define KFI_SPACING_MONO             I18N_NOOP("Monospaced")
#define KFI_SPACING_CHARCELL         I18N_NOOP("Charcell")
#define KFI_SPACING_PROPORTIONAL     I18N_NOOP("Proportional")

#define KFI_UNKNOWN_FOUNDRY          I18N_NOOP("Unknown")

#endif
